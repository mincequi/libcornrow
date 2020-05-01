#pragma once

#include <coro/audio/Node.h>

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;

namespace coro {
namespace audio {

class AudioDecoderFfmpeg : public Node
{
public:
    AudioDecoderFfmpeg();
    ~AudioDecoderFfmpeg();

    static constexpr std::array<audio::AudioCaps, 1> inCaps() {
        return {{ { AudioCodec::Ac3 | AudioCodec::Eac3,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCaps, 1> outCaps() {
        return {{ { AudioCodec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

private:
    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    void updateConf();

    AudioConf m_conf;

    AVCodecContext* m_context = nullptr;
};

} // namespace audio
} // namespace coro
