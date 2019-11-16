#pragma once

#include <gst/rtp/gstrtpbasedepayload.h>

#include "RtpTypes.h"

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

class _CrRtpSbcDepay
{
public:
    GstRTPBaseDepayload base;

    /// Process buffer
   // void process(coro::audio::AudioBuffer& buffer);

// protected:
    /// Push this node's configuration downstream.
    void pushConf(); // override

// private:
    /// Handle the upstream node's configuration.
    void onConfPushed(); // override

    int m_sampleRate;
    uint m_currenBufferSize = 0;

    coro::rtp::SbcFrameHeader m_sbcHeader;
};

struct _CrRtpSbcDepayClass
{
    GstRTPBaseDepayloadClass parent_class;
};

GType cr_rtp_sbc_depay_get_type (void);

G_END_DECLS
