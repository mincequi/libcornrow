#include "audio/AudioDecoderFfmpeg.h"

#include <assert.h>
#include <cstring>
#include <loguru/loguru.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

namespace coro {
namespace audio {

static AVCodecID toFfmpegCodecId(AudioCodec codec)
{
    switch(codec) {
    case AudioCodec::Ac3: return AV_CODEC_ID_AC3;
    default: return AV_CODEC_ID_NONE;
    }
}

static SampleRate toCoro(int sampleRate)
{
    switch(sampleRate) {
    case 44100: return SampleRate::Rate44100;
    case 48000: return SampleRate::Rate48000;
    default: return SampleRate::Invalid;
    }
}

AudioDecoderFfmpeg::AudioDecoderFfmpeg()
{
}

AudioDecoderFfmpeg::~AudioDecoderFfmpeg()
{
    if (m_context) {
        // This will also free the encoder.
        avcodec_free_context(&m_context);
    }
}

AudioConf AudioDecoderFfmpeg::doProcess(const AudioConf& conf, AudioBuffer& _buffer)
{
    if (m_conf != conf) {
        m_conf = conf;
        updateConf();
    }

    //_buffer.grow(AV_INPUT_BUFFER_PADDING_SIZE);
    auto packet = av_packet_alloc();
    packet->data = reinterpret_cast<uint8_t*>(_buffer.data());
    packet->size = _buffer.size();

    auto ret = avcodec_send_packet(m_context, packet);
    LOG_IF_F(WARNING, ret == AVERROR(EAGAIN), "No input accepted in current state");
    LOG_IF_F(WARNING, ret == AVERROR_EOF, "Encoder flushed, no new frames can be sent to it");
    LOG_IF_F(WARNING, ret == AVERROR(EINVAL), "Codec not opened, refcounted_frames not set");
    LOG_IF_F(WARNING, ret == AVERROR(ENOMEM), "Failed to add packet to internal queue");
    LOG_IF_F(WARNING, ret < 0, "Decoder error");

    auto frame = av_frame_alloc();
    //while (ret >= 0) {
        ret = avcodec_receive_frame(m_context, frame);
        LOG_IF_F(WARNING, ret == AVERROR(EAGAIN), "No output available in current state");
        LOG_IF_F(WARNING, ret == AVERROR_EOF, "Decoder flushed, no new frames can be sent to it");
        LOG_IF_F(WARNING, ret == AVERROR(EINVAL), "Codec not opened");
        LOG_IF_F(WARNING, ret < 0, "Decoder error");
        if (frame->format != AV_SAMPLE_FMT_FLTP) {
            LOG_F(ERROR, "Only float planar formats supported");
            return conf;
        }
        auto data = reinterpret_cast<float*>(_buffer.acquire(frame->linesize[0] * frame->channels));
        for (int s = 0; s < frame->nb_samples; ++s) {
            for (int c = 0; c < frame->channels; ++c) {
                *data = *(float*)(frame->data[c]+(s*4));
                if (*data <= -1.3f || *data >= 1.3f) {
                    //LOG_F(ERROR, "Eror at %d %d", s, c);
                }
                //assert(*data <= 1.0f && *data >= -1.0f);
                ++data;
            }
        }
        _buffer.commit(frame->linesize[0] * frame->channels);
        //next()->process( { AudioCodec::RawFloat32, toCoro(frame->sample_rate), Channels::Stereo }, _buffer);
    //}

    AudioConf _conf = { AudioCodec::RawFloat32, toCoro(frame->sample_rate), Channels::Stereo };

    av_packet_free(&packet);
    av_frame_free(&frame);

    return _conf;
}

void AudioDecoderFfmpeg::updateConf()
{
    if (m_context) {
        // This will also free the decoder.
        avcodec_free_context(&m_context);
    }

    auto decoder = avcodec_find_decoder(toFfmpegCodecId(m_conf.codec));
    m_context = avcodec_alloc_context3(decoder);
    int ret = avcodec_open2(m_context, decoder, NULL);
    LOG_IF_F(ERROR, ret < 0, "Error opening codec: %d", ret);
}

} // namespace audio
} // namespace coro
