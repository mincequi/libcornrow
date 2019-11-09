#include "core/FdSource.h"

#include <iostream>
#include <unistd.h>

#define cr_fd_source_parent_class parent_class
G_DEFINE_TYPE (CrFdSource, cr_fd_source, GST_TYPE_PUSH_SRC);

static GstFlowReturn cr_fd_source_create (GstPushSrc * bsrc, GstBuffer ** outbuf);
static gboolean cr_fd_source_start (GstBaseSrc * bsrc);
static gboolean cr_fd_source_stop (GstBaseSrc * bsrc);
static gboolean cr_fd_source_unlock (GstBaseSrc * bsrc);
static gboolean cr_fd_source_unlock_stop (GstBaseSrc * bsrc);

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

    GstCaps *caps = gst_caps_new_simple ("application/x-rtp",
                                         "media", G_TYPE_STRING, "audio",
                                         "payload", GST_TYPE_INT_RANGE, 96, 127,
                                         "encoding-name", G_TYPE_STRING, "SBC",
                                         "clock-rate", G_TYPE_INT, 44100,
                                         NULL);
    GstPadTemplate* templ = gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, caps);
    gst_element_class_add_pad_template (elementClass, templ);
}

static void cr_fd_source_init (CrFdSource * self)
{
    gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);
    gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
    gst_base_src_set_do_timestamp (GST_BASE_SRC (self), TRUE);
}

void CrFdSource::init(int fd, uint32_t blockSize)
{
    m_fd = fd;
    m_blockSize = blockSize;
}

static GstFlowReturn cr_fd_source_create(GstPushSrc* bsrc, GstBuffer** outBuffer)
{
    CrFdSource *self = (CrFdSource*)bsrc;

    if (g_atomic_int_get(&self->m_isFlushing)) {
        return GST_FLOW_FLUSHING;
    }

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

    auto data = new char[self->m_blockSize];
    auto size = read(self->m_pfd.fd, data, self->m_blockSize);
    auto buffer = gst_buffer_new_wrapped(data, self->m_blockSize);
    gst_buffer_resize(buffer, 0, size);
    *outBuffer = buffer;
    //*outBuffer = gst_buffer_new_and_alloc(1);

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

    return TRUE;
}
