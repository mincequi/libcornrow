#include "RtpSbcDepay.h"

#include "core/Buffer.h"
#include "rtp/RtpTypes.h"

#include <loguru/loguru.hpp>

#include <string>
#include <vector>

GST_DEBUG_CATEGORY_STATIC (rtpsbcdepay_debug);
#define GST_CAT_DEFAULT (rtpsbcdepay_debug)

static GstStaticPadTemplate s_srcTemplate =
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-sbc, "
        "rate = (int) { 44100, 48000 }, "
        "channels = (int) [ 1, 2 ], "                       // usually 2
        "mode = (string) { mono, dual, stereo, joint }, "   // usually joint
        "blocks = (int) { 4, 8, 12, 16 }, "                 // usually 16
        "subbands = (int) { 4, 8 }, "                       // usually 8
        "allocation-method = (string) { snr, loudness }, "  // usually loudness
        "bitpool = (int) [ 2, 64 ], "
        "parsed = (boolean) true")
    );

static GstStaticPadTemplate s_sinkTemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) audio,"
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) { 44100, 48000 },"
        "encoding-name = (string) SBC")
    );

#define cr_rtp_sbc_depay_parent_class parent_class
G_DEFINE_TYPE (CrRtpSbcDepay, cr_rtp_sbc_depay, GST_TYPE_RTP_BASE_DEPAYLOAD);

static GstBuffer *cr_rtp_sbc_depay_process (GstRTPBaseDepayload * base, GstRTPBuffer * rtp);
static GstBuffer * cr_rtp_sbc_depay_get_payload_subbuffer (GstRTPBuffer * buffer, guint offset);

static void cr_rtp_sbc_depay_class_init (CrRtpSbcDepayClass * klass)
{
    GstRTPBaseDepayloadClass *rtpBaseDepayloadClass = GST_RTP_BASE_DEPAYLOAD_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    rtpBaseDepayloadClass->process_rtp_packet = cr_rtp_sbc_depay_process;

    gst_element_class_add_static_pad_template (element_class, &s_srcTemplate);
    gst_element_class_add_static_pad_template (element_class, &s_sinkTemplate);

    GST_DEBUG_CATEGORY_INIT (rtpsbcdepay_debug, "rtpsbcdepay", 0,
                             "SBC Audio RTP Depayloader");

    gst_element_class_set_static_metadata (element_class,
                                           "RTP SBC audio depayloader",
                                           "Codec/Depayloader/Network/RTP",
                                           "Extracts SBC audio from RTP packets",
                                           "Manuel Weichselbaumer <mincequi@web.de>");
}

static void cr_rtp_sbc_depay_init(CrRtpSbcDepay* self)
{
}

void CrRtpSbcDepay::pushConf()
{
    // Rates
    std::vector<int> rates = { 16000, 32000, 44100, 48000 };
    // Modes
    std::vector<std::string> chModes = { "mono", "dual", "stereo", "joint" };

    auto caps = gst_caps_new_simple(
                "audio/x-sbc",
                "rate", G_TYPE_INT, rates[m_sbcHeader.sampleRate],
                "channels", G_TYPE_INT, (m_sbcHeader.channelMode > 0) ? 2 : 1,
                "channel-mode", G_TYPE_STRING, chModes[m_sbcHeader.channelMode].c_str(),
            "blocks", G_TYPE_INT, (1+m_sbcHeader.blockSize)*4,
            "subbands", G_TYPE_INT, m_sbcHeader.subBandCount ? 8 : 4,
            "allocation-method", G_TYPE_STRING, m_sbcHeader.allocMethod ? "snr" : "loudness",
            "bitpool", G_TYPE_INT, m_sbcHeader.bitPool,
            "parsed", G_TYPE_BOOLEAN, TRUE,
            NULL);
    gst_pad_push_event(GST_RTP_BASE_DEPAYLOAD_SRCPAD(this), gst_event_new_caps(caps));
    gst_caps_unref(caps);
}

// Returns size of single frame
static int cr_rtp_sbc_depay_get_params(const guint8* data, int* samples)
{
    int blocks, channel_mode, channels, subbands, bitpool;
    int length;

    blocks = (data[1] >> 4) & 0x3;
    blocks = (blocks + 1) * 4;
    channel_mode = (data[1] >> 2) & 0x3;
    channels = channel_mode ? 2 : 1;
    subbands = (data[1] & 0x1);
    subbands = (subbands + 1) * 4;
    bitpool = data[2];

    length = 4 + ((4 * subbands * channels) / 8);

    if (channel_mode == 0 || channel_mode == 1) {
        /* Mono || Dual channel */
        length += ((blocks * channels * bitpool) + 4 /* round up */ ) / 8;
    } else {
        /* Stereo || Joint stereo */
        gboolean joint = (channel_mode == 3);
        length += ((joint * subbands) + (blocks * bitpool) + 4 /* round up */ ) / 8;
    }

    *samples = blocks * subbands;

    return length;
}

static GstBuffer* cr_rtp_sbc_depay_process (GstRTPBaseDepayload * base, GstRTPBuffer * rtp)
{
    CrRtpSbcDepay *self = CR_RTP_SBC_DEPAY (base);

    GstBuffer *data = NULL;
    uint8_t* payload = (uint8_t*)gst_rtp_buffer_get_payload(rtp);
    guint8 framesCount;
    guint payloadSize = gst_rtp_buffer_get_payload_len(rtp);
    gint samples = 0;
    gint frameSize = 0;

    coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(rtp->data[0]);
    coro::rtp::RtpSbcHeader* rtpSbcHeader = (coro::rtp::RtpSbcHeader*)(payload);
    self->m_sbcHeader = *((coro::rtp::SbcFrameHeader*)(payload+1));
    if (payloadSize < 4) {
        GST_WARNING_OBJECT (self, "Payload too small");
        goto bad_packet;
    }
    if (!rtpHeader->isValidSbc()) {
        LOG_F(WARNING, "Invalid RTP header");
        goto bad_packet;
    }
    if (!rtpSbcHeader->isValid()) {
        LOG_F(WARNING, "Invalid RTP SBC header");
        goto bad_packet;
    }
    if (rtpSbcHeader->isFragmented) {
        LOG_F(WARNING, "Fragmented packet(s) not supported");
        goto bad_packet;
    }

    framesCount = rtpSbcHeader->frameCount;
    payload += 1;
    payloadSize -= 1;

    // Do sanity checks
    if (!self->m_sbcHeader.isValid()) {
        LOG_F(WARNING, "Invalid RTP SBC header");
        goto bad_packet;
    }

    frameSize = cr_rtp_sbc_depay_get_params(payload, &samples);
    samples *= framesCount;
    if (framesCount * frameSize > (gint) payloadSize) {
        LOG_F(WARNING, "Packet truncated. frameSize: %i, framesCount: %i, payloadSize: %i", frameSize, framesCount, payloadSize);
        goto bad_packet;
    } else if (framesCount * frameSize < (gint) payloadSize) {
        LOG_F(WARNING, "Junk after packet. frameSize: %i, framesCount: %i, payloadSize: %i", frameSize, framesCount, payloadSize);
    }

    self->pushConf();
    return cr_rtp_sbc_depay_get_payload_subbuffer(rtp, 1);

bad_packet:
    gst_buffer_unref(data);
    data = NULL;
    return NULL;
}

static GstBuffer * cr_rtp_sbc_depay_get_payload_subbuffer(GstRTPBuffer * buffer, guint offset)
{
    guint poffset, payloadSize;
    GstBuffer* out;

    payloadSize = gst_buffer_get_size(buffer->buffer) - gst_rtp_buffer_get_header_len (buffer) - buffer->size[3];
    /* we can't go past the length */
    if (G_UNLIKELY (offset >= payloadSize))
        goto wrong_offset;

    /* apply offset */
    poffset = gst_rtp_buffer_get_header_len (buffer) + offset;
    payloadSize -= offset;

    out =  gst_buffer_copy_region (buffer->buffer, GST_BUFFER_COPY_ALL, poffset, payloadSize);

    // Some logging
    {
        static gsize currenBufferSize = -1;
        auto bufferSize = gst_buffer_get_size(out);
        if (currenBufferSize != bufferSize) {
            LOG_F(INFO, "RtpSbcDepay> out buffer size: {0}", bufferSize);
            currenBufferSize = bufferSize;
        }
    }

    return out;

wrong_offset:
    {
        LOG_F(WARNING, "offset should be less then payload size");
        return NULL;
    }
}
