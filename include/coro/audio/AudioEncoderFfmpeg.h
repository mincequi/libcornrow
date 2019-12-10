#pragma once

#include <coro/audio/Node.h>

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;

namespace coro {
namespace audio {

class AudioEncoderFfmpeg : public Node
{
public:
    AudioEncoderFfmpeg();
    ~AudioEncoderFfmpeg();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { Codec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { Codec::Ac3,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    Channels::Mono | Channels::Stereo } }};
    }

protected:
    void start(const AudioConf& conf) override;
    void stop() override;
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    static void freeBuffer(void *opaque, uint8_t *data);
    AVFrame* createFrame() const;
    AVFrame* fillFrame(AVFrame* frame, AudioBuffer& buffer);
    void     pushFrame(AVFrame* frame);

    AudioConf m_conf;

    AVCodecContext* m_context;
    AVFrame* m_partialFrame = nullptr;
    AVPacket* m_packet;
};

} // namespace audio
} // namespace coro
