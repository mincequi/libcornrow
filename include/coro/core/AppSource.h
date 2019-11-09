#pragma once

#include <gst/base/gstpushsrc.h>

G_BEGIN_DECLS
#define CR_TYPE_APP_SOURCE      (cr_app_source_get_type())
#define GST_BUFFER_SRC(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj),CR_TYPE_APP_SOURCE, CrAppSource))
#define GST_BUFFER_SRC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CR_TYPE_APP_SOURCE, CrAppSource))

typedef struct _CrAppSource CrAppSource;
typedef struct _CrAppSourceClass CrAppSourceClass;

struct _CrAppSourceClass
{
    GstPushSrcClass parentclass;
};

struct _CrAppSource
{
    GstPushSrc basesrc;

    void pushBuffer(GstBuffer* buffer);

    GCond cond;
    GMutex mutex;
    GQueue *queue;

    gboolean isFlushing;

    // FD related members
    GstPoll* poll = nullptr;
    GstPollFD pfd;
    guint blockSize = 0;
};

void cr_app_source_push_buffer (CrAppSource * buffersrc, GstBuffer * buffer);

GType cr_app_source_get_type (void);

G_END_DECLS
