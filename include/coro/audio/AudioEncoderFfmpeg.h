#pragma once

#include <coro/audio/Node.h>

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;
typedef struct AVCodec AVCodec;

namespace coro {
namespace audio {

class AudioEncoderFfmpeg : public Node
{
public:
    AudioEncoderFfmpeg(AudioCodec codec);
    ~AudioEncoderFfmpeg();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { AudioCodec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { AudioCodec::Ac3,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    Channels::Mono | Channels::Stereo } }};
    }

    /**
     * @brief setBitrate
     * @param kbps
     */
    void setBitrate(uint16_t kbps);

protected:
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    void updateConf();

    static void freeBuffer(void *opaque, uint8_t *data);
    AVFrame* createFrame() const;
    AVFrame* fillFrame(AVFrame* frame, AudioBuffer& buffer);
    void     pushFrame(AVFrame* frame);

    AudioConf m_conf;

    AVCodec*        m_encoder = nullptr;
    AVCodecContext* m_context = nullptr;
    AVFrame* m_partialFrame = nullptr;
    AVPacket* m_packet;
};

} // namespace audio
} // namespace coro
