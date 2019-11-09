#pragma once

#include <gst/rtp/gstrtpbasedepayload.h>

G_BEGIN_DECLS
#define CR_TYPE_RTP_SBC_DEPAY \
    (cr_rtp_sbc_depay_get_type())
#define CR_RTP_SBC_DEPAY(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),CR_TYPE_RTP_SBC_DEPAY,\
        CrRtpSbcDepay))
#define CR_RTP_SBC_DEPAY_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),CR_TYPE_RTP_SBC_DEPAY,\
        CrRtpSbcDepayClass))
#define CR_IS_RTP_SBC_DEPAY(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),CR_TYPE_RTP_SBC_DEPAY))
#define CR_IS_RTP_SBC_DEPAY_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass),CR_TYPE_RTP_SBC_DEPAY))
typedef struct _CrRtpSbcDepay CrRtpSbcDepay;
typedef struct _CrRtpSbcDepayClass CrRtpSbcDepayClass;

struct _CrRtpSbcDepay
{
    GstRTPBaseDepayload base;

    int m_sampleRate;
};

struct _CrRtpSbcDepayClass
{
    GstRTPBaseDepayloadClass parent_class;
};

GType cr_rtp_sbc_depay_get_type (void);

G_END_DECLS
