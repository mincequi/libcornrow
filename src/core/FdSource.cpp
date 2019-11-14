#include "core/FdSource.h"

#include <loguru/loguru.hpp>
#include <unistd.h>

#define cr_fd_source_parent_class parent_class
G_DEFINE_TYPE (CrFdSource, cr_fd_source, GST_TYPE_PUSH_SRC);

static GstFlowReturn cr_fd_source_create (GstPushSrc * bsrc, GstBuffer ** outbuf);
static gboolean cr_fd_source_start (GstBaseSrc * bsrc);
static gboolean cr_fd_source_stop (GstBaseSrc * bsrc);
static gboolean cr_fd_source_unlock (GstBaseSrc * bsrc);
static gboolean cr_fd_source_unlock_stop (GstBaseSrc * bsrc);

static GstStaticPadTemplate s_srcTemplate =
    GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\","
        "payload = (int) [96, 127], "
        "clock-rate = (int) { 44100, 48000 }, "
        "encoding-name = (string) \"SBC\"; "));

static void cr_fd_source_class_init (CrFdSourceClass * klass)
{
    //GObjectClass *gObjectClass = G_OBJECT_CLASS (klass);
    GstElementClass *elementClass = GST_ELEMENT_CLASS (klass);
    GstBaseSrcClass *baseSrcClass = GST_BASE_SRC_CLASS (klass);
    GstPushSrcClass *pushSrcClass = GST_PUSH_SRC_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    baseSrcClass->start = GST_DEBUG_FUNCPTR (cr_fd_source_start);
    baseSrcClass->stop = GST_DEBUG_FUNCPTR (cr_fd_source_stop);
    baseSrcClass->unlock = GST_DEBUG_FUNCPTR (cr_fd_source_unlock);
    baseSrcClass->unlock_stop = GST_DEBUG_FUNCPTR (cr_fd_source_unlock_stop);
    pushSrcClass->create = GST_DEBUG_FUNCPTR (cr_fd_source_create);

    gst_element_class_set_static_metadata (elementClass,
                                           "Conrow File Descriptor Source",
                                           "Source/File",
                                           "Read from a file descriptor",
                                           "Manuel Weichselbaumer <mincequi@web.de>");

    gst_element_class_add_static_pad_template(elementClass, &s_srcTemplate);
}

static void cr_fd_source_init (CrFdSource * self)
{
    self->m_fd = -1;
    self->m_poll = nullptr;
    self->m_blockSize = 4096;
    self->m_allocFactor = 1;
    self->m_currentPacketSize = -1;

    gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);
    gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
    gst_base_src_set_do_timestamp (GST_BASE_SRC (self), TRUE);
}

void CrFdSource::init(uint32_t sampleRate, int fd, uint32_t blockSize, uint8_t allocFactor)
{
    m_fd = fd;
    m_blockSize = blockSize;
    m_allocFactor = allocFactor;

    pushConf(sampleRate);
}

GstBuffer* CrFdSource::readFd()
{
    // The allocFactor multiplies the space that is actually needed for a single block (since
    // there might be multiple blocks waiting on the file descriptor).
    //   * 4 seems to be enough for most cases.
    //   * 7 might be reasonable, so we can decode SBC frames in-place (but we need to allocate it for each slice).
    //   * 10 seems to be the max (for maxSize 672 and real size 608).
    auto data = new char[m_blockSize*m_allocFactor];
    auto size = read(m_pfd.fd, data, m_blockSize*m_allocFactor);
    if (size < 1) {
        delete [] data;
        return nullptr;
    }
    auto buffer = gst_buffer_new_wrapped(data, m_blockSize*m_allocFactor);
    int slices = 1+(size/m_blockSize); // 608 -> 1, 1216 -> 2, 1824 -> 3, 4864 -> 8
    if (slices == 1) {
        gst_buffer_resize(buffer, 0, size);
    } else if (size%slices != 0) {
        LOG_F(WARNING, "Cannot estimate number of slices. Pushing as a whole.");
        gst_buffer_resize(buffer, 0, size);
    } else {
        // Create pending buffers
        for (int i = 1; i < slices; ++i) {
            m_pendingBuffers.push(gst_buffer_copy_region(buffer, GST_BUFFER_COPY_ALL, i*(size/slices), size/slices));
        }
        // Resize original buffer
        gst_buffer_resize(buffer, 0, size/slices);
    }

    // Some logging
    if (m_currentPacketSize != size) {
        LOG_F(INFO, "Current packet size: {0}", size);
        m_currentPacketSize = size;
    }

    return buffer;
}

void CrFdSource::pushConf(uint32_t sampleRate)
{
    auto caps = gst_caps_new_simple("application/x-rtp",
                                    "media", G_TYPE_STRING, "audio",
                                    "payload", GST_TYPE_INT_RANGE, 96, 127,
                                    "encoding-name", G_TYPE_STRING, "SBC",
                                    NULL);
    gst_caps_set_simple(caps, "clock-rate", G_TYPE_INT, sampleRate, NULL);
    gst_pad_push_event (GST_BASE_SRC_PAD(this), gst_event_new_caps (caps));
    gst_caps_unref(caps);
}

static GstFlowReturn cr_fd_source_create(GstPushSrc* bsrc, GstBuffer** outBuffer)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    if (g_atomic_int_get(&self->m_isFlushing)) {
        return GST_FLOW_FLUSHING;
    }

    // Deliver pending buffer
    if (!self->m_pendingBuffers.empty()) {
        *outBuffer = self->m_pendingBuffers.front();
        self->m_pendingBuffers.pop();
        return GST_FLOW_OK;
    }

    // Try if we can read
    auto buffer = self->readFd();
    if (buffer) {
        *outBuffer = buffer;
        return GST_FLOW_OK;
    }

    // Now wait
    gint ret;
    while ((ret = gst_poll_wait(self->m_poll, GST_CLOCK_TIME_NONE))) {
        if (g_atomic_int_get(&self->m_isFlushing)) {
            return GST_FLOW_FLUSHING;
        }

        if (ret < 0) {
            return GST_FLOW_ERROR;
        }

        /* Got some data */
        if (ret > 0) {
            break;
        }
    }

    *outBuffer = self->readFd();

    return GST_FLOW_OK;
}

static gboolean cr_fd_source_unlock(GstBaseSrc * bsrc)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    g_atomic_int_set(&self->m_isFlushing, TRUE);
    gst_poll_set_flushing(self->m_poll, TRUE);

    return TRUE;
}

static gboolean cr_fd_source_unlock_stop(GstBaseSrc * bsrc)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    g_atomic_int_set(&self->m_isFlushing, FALSE);
    gst_poll_set_flushing(self->m_poll, FALSE);

    return TRUE;
}

static gboolean cr_fd_source_start (GstBaseSrc * bsrc)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    // If FD is set, we start polling it
    if (self->m_fd > 0) {
        self->m_poll = gst_poll_new(TRUE);
        gst_poll_fd_init(&self->m_pfd);
        self->m_pfd.fd = self->m_fd;
        gst_poll_add_fd(self->m_poll, &self->m_pfd);
        gst_poll_fd_ctl_read(self->m_poll, &self->m_pfd, TRUE);
        gst_poll_set_flushing(self->m_poll, FALSE);
    }

    return TRUE;
}

static gboolean cr_fd_source_stop (GstBaseSrc * bsrc)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    if (self->m_poll) {
        gst_poll_remove_fd(self->m_poll, &self->m_pfd);
        gst_poll_set_flushing(self->m_poll, TRUE);
        close(self->m_pfd.fd);
        gst_poll_free(self->m_poll);
        self->m_poll = nullptr;
        self->m_fd = -1;
    }

    // Clear pending buffers
    while (!self->m_pendingBuffers.empty()) {
        gst_buffer_unref(self->m_pendingBuffers.front());
        self->m_pendingBuffers.pop();
    }

    return TRUE;
}
