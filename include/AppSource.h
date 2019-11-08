#pragma once

#include <gst/base/gstpushsrc.h>

#include <condition_variable>
#include <mutex>

G_BEGIN_DECLS
#define GST_TYPE_CR_APP_SOURCE \
    (cr_app_source_get_type())
#define GST_BUFFER_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CR_APP_SOURCE, CrAppSource))
#define GST_BUFFER_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CR_APP_SOURCE, CrAppSource))

typedef struct _CrAppSource CrAppSource;
typedef struct _CrAppSourceClass CrAppSourceClass;

struct _CrAppSourceClass
{
    GstPushSrcClass parentclass;
};

struct _CrAppSource
{
    GstPushSrc basesrc;

    std::mutex              m_mutex;
    std::condition_variable m_condVar;
    GQueue* m_queue;

    gboolean m_isFlushing;
};

GType cr_app_source_get_type (void);

void cr_app_source_push_buffer (CrAppSource * buffersrc, GstBuffer * buffer);

gboolean cr_app_source_plugin_init (GstPlugin * plugin);

G_END_DECLS
