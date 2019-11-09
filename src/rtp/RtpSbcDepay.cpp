#include "RtpSbcDepay.h"

#include <iostream>
#include <stdint.h>

#include <core/Buffer.h>

GST_DEBUG_CATEGORY_STATIC (rtpsbcdepay_debug);
#define GST_CAT_DEFAULT (rtpsbcdepay_debug)

static GstStaticPadTemplate cr_rtp_sbc_depay_src_template =
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-sbc, "
        "rate = (int) { 44100, 48000 }, "
        "channels = (int) [ 1, 2 ], "                       // usually 2
        "mode = (string) { mono, dual, stereo, joint }, "   // usually joint
        "blocks = (int) { 4, 8, 12, 16 }, "                 // usually 16
        "subbands = (int) { 4, 8 }, "                       // usually 8
        "allocation-method = (string) { snr, loudness }, "  // usually loudness
        "bitpool = (int) [ 2, 64 ]")
    );

static GstStaticPadTemplate cr_rtp_sbc_depay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) audio,"
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) { 44100, 48000 },"
        "encoding-name = (string) SBC")
    );

#define cr_rtp_sbc_depay_parent_class parent_class
G_DEFINE_TYPE (CrRtpSbcDepay, cr_rtp_sbc_depay, GST_TYPE_RTP_BASE_DEPAYLOAD);

static gboolean cr_rtp_sbc_depay_setcaps (GstRTPBaseDepayload * base, GstCaps * caps);
static GstBuffer *cr_rtp_sbc_depay_process (GstRTPBaseDepayload * base, GstRTPBuffer * rtp);
static GstBuffer * cr_rtp_sbc_depay_get_payload_subbuffer (GstRTPBuffer * buffer, guint offset);

static void cr_rtp_sbc_depay_class_init (CrRtpSbcDepayClass * klass)
{
    GstRTPBaseDepayloadClass *gstbasertpdepayload_class = GST_RTP_BASE_DEPAYLOAD_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gstbasertpdepayload_class->set_caps = cr_rtp_sbc_depay_setcaps;
    gstbasertpdepayload_class->process_rtp_packet = cr_rtp_sbc_depay_process;

    gst_element_class_add_static_pad_template (element_class, &cr_rtp_sbc_depay_src_template);
    gst_element_class_add_static_pad_template (element_class, &cr_rtp_sbc_depay_sink_template);

    GST_DEBUG_CATEGORY_INIT (rtpsbcdepay_debug, "rtpsbcdepay", 0,
                             "SBC Audio RTP Depayloader");

    gst_element_class_set_static_metadata (element_class,
                                           "RTP SBC audio depayloader",
                                           "Codec/Depayloader/Network/RTP",
                                           "Extracts SBC audio from RTP packets",
                                           "Manuel Weichselbaumer <mincequi@web.de>");
}

static void cr_rtp_sbc_depay_init(CrRtpSbcDepay * rtpsbcdepay)
{
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

static gboolean cr_rtp_sbc_depay_setcaps (GstRTPBaseDepayload * base, GstCaps * caps)
{
    CrRtpSbcDepay *depay = CR_RTP_SBC_DEPAY (base);
    GstStructure *structure;
    GstCaps *outcaps;

    structure = gst_caps_get_structure (caps, 0);

    if (!gst_structure_get_int (structure, "clock-rate", &depay->m_sampleRate))
        goto bad_caps;

    outcaps = gst_caps_new_simple ("audio/x-sbc", "rate", G_TYPE_INT, depay->m_sampleRate, NULL);
    gst_pad_set_caps (GST_RTP_BASE_DEPAYLOAD_SRCPAD (base), outcaps);
    gst_caps_unref (outcaps);

    return TRUE;

bad_caps:
    GST_WARNING_OBJECT (depay, "Can't support the caps we got: %" GST_PTR_FORMAT, caps);
    return FALSE;
}

static GstBuffer* cr_rtp_sbc_depay_process (GstRTPBaseDepayload * base, GstRTPBuffer * rtp)
{
    CrRtpSbcDepay *depay = CR_RTP_SBC_DEPAY (base);
    GstBuffer *data = NULL;

    gboolean isFragmented;
    guint8 framesCount;
    guint8 *payload;
    guint payloadSize;
    gint samples = 0;
    gint frameSize = 0;

    //std::cout << __func__ << "> hash: " << coro::core::Buffer::hash(rtp->buffer) << std::endl;

    // Marker shall be zero
    if (gst_rtp_buffer_get_marker (rtp)) {
        GST_WARNING_OBJECT (depay, "Marker bit was set");
        goto bad_packet;
    }

    payload = (uint8_t*)gst_rtp_buffer_get_payload (rtp);
    payloadSize = gst_rtp_buffer_get_payload_len (rtp);

    isFragmented = payload[0] & 0x80;
    framesCount = payload[0] & 0x0f;

    payload += 1;
    payloadSize -= 1;

    // Do sanity checks
    if (isFragmented) {
        GST_WARNING_OBJECT (depay, "Fragmented packet(s) not supported");
        goto bad_packet;
    }
    if (payloadSize < 3) {
        GST_WARNING_OBJECT (depay, "Payload too small");
        goto bad_packet;
    }
    if (payload[0] != 0x9c) {
        GST_WARNING_OBJECT (depay, "Syncword invalid");
        goto bad_packet;
    }

    frameSize = cr_rtp_sbc_depay_get_params(payload, &samples);
    samples *= framesCount;
    if (framesCount * frameSize > (gint) payloadSize) {
        GST_WARNING_OBJECT (depay, "Short packet");
        goto bad_packet;
    } else if (framesCount * frameSize < (gint) payloadSize) {
        GST_WARNING_OBJECT (depay, "Junk at end of packet");
    }

    GST_LOG_OBJECT (depay, "Got %d frames with a total payload size of %d", framesCount, payloadSize);
    return cr_rtp_sbc_depay_get_payload_subbuffer(rtp, 1);

bad_packet:
    GST_ELEMENT_WARNING (depay, STREAM, DECODE, ("Received invalid RTP payload, dropping"), (NULL));
    gst_buffer_unref (data);
    data = NULL;
    return NULL;
}

static GstBuffer * cr_rtp_sbc_depay_get_payload_subbuffer(GstRTPBuffer * buffer, guint offset)
{
    guint poffset, payloadSize;

    payloadSize = gst_buffer_get_size(buffer->buffer) - gst_rtp_buffer_get_header_len (buffer) - buffer->size[3];
    /* we can't go past the length */
    if (G_UNLIKELY (offset >= payloadSize))
        goto wrong_offset;

    /* apply offset */
    poffset = gst_rtp_buffer_get_header_len (buffer) + offset;
    payloadSize -= offset;

    return gst_buffer_copy_region (buffer->buffer, GST_BUFFER_COPY_ALL, poffset, payloadSize);

wrong_offset:
    {
        g_warning ("offset: %u should be less then payload size: %u", offset, payloadSize);
        return NULL;
    }
}
