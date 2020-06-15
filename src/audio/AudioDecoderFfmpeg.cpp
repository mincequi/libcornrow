#include "audio/AudioDecoderFfmpeg.h"

#include <assert.h>
#include <cstdint>
#include <cstring>
#include <regex>
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
    //case AudioCodec::Eac3: return AV_CODEC_ID_EAC3; // untested
    case AudioCodec::Alac: return AV_CODEC_ID_ALAC;
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

template class AudioDecoderFfmpeg<audio::AudioCodec::Ac3>;
template class AudioDecoderFfmpeg<audio::AudioCodec::Alac>;

template<audio::AudioCodec codec>
AudioDecoderFfmpeg<codec>::AudioDecoderFfmpeg()
{
}

template<audio::AudioCodec codec>
AudioDecoderFfmpeg<codec>::~AudioDecoderFfmpeg()
{
    if (m_context) {
        // This will also free the encoder.
        avcodec_free_context(&m_context);
    }
}

template<>
void AudioDecoderFfmpeg<audio::AudioCodec::Alac>::init(const std::string& data)
{
    LOG_F(INFO, "ALAC fmtp: %s", data.c_str());

    /**
     * @file
     * ALAC (Apple Lossless Audio Codec) decoder
     * @author 2005 David Hammerton
     * @see http://crazney.net/programs/itunes/alac.html
     *
     * Note: This decoder expects a 36-byte QuickTime atom to be
     * passed through the extradata[_size] fields. This atom is tacked onto
     * the end of an 'alac' stsd atom and has the following format:
     *
     * 32 bits  atom size
     * 32 bits  tag                  ("alac")
     * 32 bits  tag version          (0)
     * 32 bits  samples per frame    (used when not set explicitly in the frames)
     *  8 bits  compatible version   (0)
     *  8 bits  sample size
     *  8 bits  history mult         (40)
     *  8 bits  initial history      (14)   @TODO(mawe): 14 and 10 are changed here.
     *  8 bits  rice param limit     (10)   should be addressed to FFMPEG guys
     *  8 bits  channels
     * 16 bits  maxRun               (255)
     * 32 bits  max coded frame size (0 means unknown)
     * 32 bits  average bitrate      (0 means unknown)
     * 32 bits  samplerate
     */

    struct QuicktimeAtom {
        uint32_t size = 36;             // unhandled
        uint32_t tag = 0; //"alac";     // unhandled
        uint32_t tagVersion = 0;        // unhandled
        uint32_t samplesPerFrame;
        uint8_t compatibleVersion = 0;  // unhandled
        uint8_t sampleSize;
        uint8_t riceHistoryMult = 40;
        uint8_t riceInitialHistory = 10;
        uint8_t riceLimit = 14;
        uint8_t channels;
        uint16_t maxRun = 255;
        uint32_t maxCodedFrameSize = 0;
        uint32_t averageBitrate = 0;
        uint32_t samplerate;
    };

    QuicktimeAtom atom;
    std::regex rx("(\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)");
    std::smatch match;
    if (std::regex_search(data, match, rx) && match.size() == 13) {
        atom.samplesPerFrame = htobe32(stoi(match[2].str()));
        atom.sampleSize = stoi(match[4].str());
        atom.channels = stoi(match[8].str());
        atom.samplerate = htobe32(stoi(match[12].str()));
    }

    m_codecData.resize(36);
    std::memcpy(m_codecData.data(), &atom, m_codecData.size());
}

template<audio::AudioCodec codec>
void AudioDecoderFfmpeg<codec>::init(const std::string& data)
{
    LOG_F(INFO, "codec data size: %zu", data.size());
    m_codecData = data;
}

template<audio::AudioCodec codec>
const char* AudioDecoderFfmpeg<codec>::name() const
{
    return "AudioDecoderFfmpeg";
}

template<audio::AudioCodec codec>
AudioConf AudioDecoderFfmpeg<codec>::onProcess(const AudioConf& conf, AudioBuffer& _buffer)
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
        AudioConf _conf;
        _conf.channels = Channels::Stereo;
        _conf.rate = toCoro(frame->sample_rate);

        if (frame->format == AV_SAMPLE_FMT_FLTP) {
            interleave<float>(frame, _buffer);
            _conf.codec = AudioCodec::RawFloat32;
        } else if (frame->format == AV_SAMPLE_FMT_S16P) {
            interleave<int16_t>(frame, _buffer);
            _conf.codec = AudioCodec::RawInt16;
        } else {
            LOG_F(ERROR, "format not supported: %d", frame->format);
            _conf = {};
            _buffer.clear();
        }

        //next()->process( { AudioCodec::RawFloat32, toCoro(frame->sample_rate), Channels::Stereo }, _buffer);
    //}

    av_packet_free(&packet);
    av_frame_free(&frame);

    LOG_F(INFO, "onProcessCodec> %zu bytes", _buffer.size());

    return _conf;
}

template<audio::AudioCodec codec>
AudioConf AudioDecoderFfmpeg<codec>::onProcessCodec(AudioBuffer &buffer)
{
    return {};
}

template<audio::AudioCodec codec>
void AudioDecoderFfmpeg<codec>::updateConf()
{
    if (m_context) {
        // This will also free the decoder.
        avcodec_free_context(&m_context);
    }

    auto decoder = avcodec_find_decoder(toFfmpegCodecId(m_conf.codec));
    m_context = avcodec_alloc_context3(decoder);
    m_context->extradata = (uint8_t*)m_codecData.data();
    m_context->extradata_size = m_codecData.size();
    int ret = avcodec_open2(m_context, decoder, NULL);
    LOG_IF_F(ERROR, ret < 0, "Error opening codec: %d", ret);
}

template<audio::AudioCodec codec>
template<typename T>
void AudioDecoderFfmpeg<codec>::interleave(const AVFrame* in, AudioBuffer& out)
{
    auto data = reinterpret_cast<T*>(out.acquire(in->linesize[0] * in->channels, this));
    for (int s = 0; s < in->nb_samples; ++s) {
        for (int c = 0; c < in->channels; ++c) {
            *data = *(T*)(in->data[c] + (s * sizeof(T)));
            ++data;
        }
    }
    out.commit(in->linesize[0] * in->channels);
}

} // namespace audio
} // namespace coro
