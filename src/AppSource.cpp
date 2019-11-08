#include "AppSource.h"

#include <iostream>

#define parent_class cr_app_source_parent_class
G_DEFINE_TYPE (CrAppSource, cr_app_source, GST_TYPE_PUSH_SRC);

static void cr_app_source_finalize (GObject * gObj);
static GstFlowReturn cr_app_source_create (GstPushSrc * bsrc, GstBuffer ** outbuf);
static gboolean cr_app_source_start (GstBaseSrc * bsrc);
static gboolean cr_app_source_stop (GstBaseSrc * bsrc);
static gboolean cr_app_source_unlock (GstBaseSrc * bsrc);
static gboolean cr_app_source_unlock_stop (GstBaseSrc * bsrc);

static void cr_app_source_class_init (CrAppSourceClass * klass)
{
    GObjectClass *gObjectClass = G_OBJECT_CLASS (klass);
    GstElementClass *elementClass = GST_ELEMENT_CLASS (klass);
    GstBaseSrcClass *baseSrcClass = GST_BASE_SRC_CLASS (klass);
    GstPushSrcClass *pushSrcClass = GST_PUSH_SRC_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gObjectClass->finalize = GST_DEBUG_FUNCPTR (cr_app_source_finalize);
    baseSrcClass->start = GST_DEBUG_FUNCPTR (cr_app_source_start);
    baseSrcClass->stop = GST_DEBUG_FUNCPTR (cr_app_source_stop);
    baseSrcClass->unlock = GST_DEBUG_FUNCPTR (cr_app_source_unlock);
    baseSrcClass->unlock_stop = GST_DEBUG_FUNCPTR (cr_app_source_unlock_stop);
    pushSrcClass->create = GST_DEBUG_FUNCPTR (cr_app_source_create);

    gst_element_class_set_static_metadata (elementClass,
                                           "Conrow Application Source",
                                           "Generic/Source",
                                           "Allows an application to feed buffers to a pipeline",
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

static void cr_app_source_init (CrAppSource * buffersrc)
{
    buffersrc->queue = g_queue_new ();
    g_mutex_init(&buffersrc->mutex);

    gst_base_src_set_format (GST_BASE_SRC (buffersrc), GST_FORMAT_TIME);
    gst_base_src_set_live (GST_BASE_SRC (buffersrc), TRUE);
    gst_base_src_set_do_timestamp (GST_BASE_SRC (buffersrc), TRUE);
}

static void cr_app_source_finalize (GObject * gObj)
{
    CrAppSource *buffersrc = (CrAppSource*)gObj;

    g_queue_free(buffersrc->queue);
}

static GstFlowReturn cr_app_source_create(GstPushSrc * bsrc, GstBuffer ** buf)
{
    CrAppSource *self = (CrAppSource*)bsrc;

    g_mutex_lock (&self->mutex);

    while (TRUE) {
        if (!g_queue_is_empty (self->queue)) {
            *buf = (GstBuffer*)g_queue_pop_head (self->queue);
            gst_buffer_unref(*buf);
            break;
        }

        if (self->isFlushing) {
            break;
        }

        g_cond_wait (&self->cond, &self->mutex);
    }

    g_mutex_unlock (&self->mutex);
    return self->isFlushing ? GST_FLOW_FLUSHING : GST_FLOW_OK;
}

void cr_app_source_push_buffer (CrAppSource * buffersrc, GstBuffer * buffer)
{
    g_mutex_lock (&buffersrc->mutex);

    std::cout << __func__ << gst_buffer_get_size(buffer) << std::endl;
    gst_buffer_ref(buffer);
    g_queue_push_tail (buffersrc->queue, buffer);
    g_cond_broadcast (&buffersrc->cond);

    g_mutex_unlock (&buffersrc->mutex);
}

static gboolean cr_app_source_unlock (GstBaseSrc * bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;

    g_mutex_lock (&self->mutex);
    self->isFlushing = TRUE;
    g_cond_broadcast (&self->cond);
    g_mutex_unlock (&self->mutex);

    return TRUE;
}

static gboolean cr_app_source_unlock_stop (GstBaseSrc * bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;

    g_mutex_lock (&self->mutex);
    self->isFlushing = FALSE;
    g_mutex_unlock (&self->mutex);

    return TRUE;
}

static void cr_app_source_flush_queued (CrAppSource * self)
{
    GstBuffer *buf;

    while ((buf = (GstBuffer*)g_queue_pop_head (self->queue)))
        gst_buffer_unref (buf);
}

static gboolean cr_app_source_start (GstBaseSrc * bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;

    g_mutex_lock (&self->mutex);
    self->isFlushing = FALSE;
    g_mutex_unlock (&self->mutex);

  return TRUE;
}

static gboolean cr_app_source_stop (GstBaseSrc * bsrc)
{
    CrAppSource *self = (CrAppSource*)bsrc;

    g_mutex_lock (&self->mutex);
    self->isFlushing = TRUE;
    cr_app_source_flush_queued (self);
    g_cond_broadcast (&self->cond);
    g_mutex_unlock (&self->mutex);

    return TRUE;
}
