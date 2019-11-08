#include "AppSource.h"

#define parent_class cr_app_source_parent_class
G_DEFINE_TYPE(CrAppSource, cr_app_source, GST_TYPE_PUSH_SRC);

static void cr_app_source_finalize(GObject* gObj);
static GstFlowReturn cr_app_source_create(GstPushSrc* bsrc, GstBuffer ** outbuf);
static gboolean cr_app_source_start(GstBaseSrc* bsrc);
static gboolean cr_app_source_stop(GstBaseSrc* bsrc);
static gboolean cr_app_source_unlock(GstBaseSrc* bsrc);
static gboolean cr_app_source_unlock_stop(GstBaseSrc* bsrc);

static void cr_app_source_class_init(CrAppSourceClass* klass)
{
    GObjectClass *gObjectClass = G_OBJECT_CLASS(klass);
    GstElementClass *elementClass = GST_ELEMENT_CLASS(klass);
    GstBaseSrcClass *baseSrcClass = GST_BASE_SRC_CLASS(klass);
    GstPushSrcClass *pushSrcClass = GST_PUSH_SRC_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    gObjectClass->finalize = GST_DEBUG_FUNCPTR(cr_app_source_finalize);
    baseSrcClass->start = GST_DEBUG_FUNCPTR(cr_app_source_start);
    baseSrcClass->stop = GST_DEBUG_FUNCPTR(cr_app_source_stop);
    baseSrcClass->unlock = GST_DEBUG_FUNCPTR(cr_app_source_unlock);
    baseSrcClass->unlock_stop = GST_DEBUG_FUNCPTR(cr_app_source_unlock_stop);
    pushSrcClass->create = GST_DEBUG_FUNCPTR(cr_app_source_create);

    gst_element_class_set_static_metadata(elementClass,
                                           "Conrow Application Source",
                                           "Generic/Source",
                                           "Allows an application to feed buffers to a pipeline",
                                           "Manuel Weichselbaumer <mincequi@web.de>");

    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
                                         "media", G_TYPE_STRING, "audio",
                                         "payload", GST_TYPE_INT_RANGE, 96, 127,
                                         "encoding-name", G_TYPE_STRING, "SBC",
                                         "clock-rate", G_TYPE_INT, 44100,
                                         NULL);
    GstPadTemplate* templ = gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, caps);
    gst_element_class_add_pad_template(elementClass, templ);
}

static void cr_app_source_init(CrAppSource* self)
{
    self->m_queue = g_queue_new();

    gst_base_src_set_format(GST_BASE_SRC(self), GST_FORMAT_TIME);
    gst_base_src_set_live(GST_BASE_SRC(self), TRUE);
    gst_base_src_set_do_timestamp(GST_BASE_SRC(self), TRUE);
}

static void cr_app_source_finalize(GObject* gObj)
{
    CrAppSource *self = (CrAppSource*)gObj;

    g_queue_free(self->m_queue);
}

static GstFlowReturn cr_app_source_create(GstPushSrc* bsrc, GstBuffer ** buf)
{
    CrAppSource *self = (CrAppSource*)bsrc;
    self->m_mutex.lock();

    while(TRUE) {
        if (!g_queue_is_empty(self->m_queue)) {
            *buf = (GstBuffer*)g_queue_pop_head(self->m_queue);
            break;
        }

        if (self->m_isFlushing) {
            break;
        }

        std::unique_lock<std::mutex> lock(self->m_mutex);
        self->m_condVar.wait(lock);
    }

    self->m_mutex.unlock();
    return self->m_isFlushing ? GST_FLOW_FLUSHING : GST_FLOW_OK;
}

void cr_app_source_push_buffer(CrAppSource* self, GstBuffer* buffer)
{
    self->m_mutex.lock();

    g_queue_push_tail(self->m_queue, buffer);
    self->m_condVar.notify_all();

    self->m_mutex.unlock();
}

static gboolean cr_app_source_unlock(GstBaseSrc* bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;
    self->m_mutex.lock();

    self->m_isFlushing = TRUE;
    self->m_condVar.notify_all();

    self->m_mutex.unlock();
    return TRUE;
}

static gboolean cr_app_source_unlock_stop(GstBaseSrc* bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;
    self->m_mutex.lock();

    self->m_isFlushing = FALSE;

    self->m_mutex.unlock();
    return TRUE;
}

static void cr_app_source_flush_queued(CrAppSource* self)
{
    GstBuffer *buf;

    while((buf = (GstBuffer*)g_queue_pop_head(self->m_queue))) {
        gst_buffer_unref(buf);
    }
}

static gboolean cr_app_source_start(GstBaseSrc* bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;
    self->m_mutex.lock();

    self->m_isFlushing = FALSE;

    self->m_mutex.unlock();
    return TRUE;
}

static gboolean cr_app_source_stop(GstBaseSrc* bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;
    self->m_mutex.lock();

    self->m_isFlushing = TRUE;
    cr_app_source_flush_queued(self);
    self->m_condVar.notify_all();

    self->m_mutex.unlock();
    return TRUE;
}
