#pragma once

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

G_BEGIN_DECLS
#define GST_TYPE_AVDTP_SRC2 \
    (gst_avdtp_src_get_type())
#define GST_AVDTP_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AVDTP_SRC2, GstAvdtpSrc2))
#define GST_AVDTP_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AVDTP_SRC2, GstAvdtpSrc2))
#define GST_IS_AVDTP_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AVDTP_SRC2))
#define GST_IS_AVDTP_SRC_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AVDTP_SRC2))
typedef struct _GstAvdtpSrc GstAvdtpSrc2;
typedef struct _GstAvdtpSrcClass GstAvdtpSrc2Class;

struct _GstAvdtpSrcClass
{
    GstBaseSrcClass parentclass;
};

struct _GstAvdtpSrc
{
    GstBaseSrc basesrc;

    GstPoll* poll = nullptr;
    GstPollFD pfd;
    guint blockSize = 0;

    volatile gint unlocked;
};

void gst_avdtp_src_set_fd(int fd, guint blockSize);

GType gst_avdtp_src_get_type (void);

gboolean gst_avdtp_src_plugin_init (GstPlugin * plugin);

G_END_DECLS
