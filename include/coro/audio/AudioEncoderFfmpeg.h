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
    AudioEncoderFfmpeg(AudioCodec codec);
    ~AudioEncoderFfmpeg();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { AudioCodec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCaps,2> outCaps() {
        return {{ { AudioCodec::Ac3 | AudioCodec::Eac3,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    Channels::Mono | Channels::Stereo },
                  // We can be bypassed, so also accept inCaps as OutCaps
                  { AudioCodec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    /**
     * @brief setBitrate
     * @param kbps
     */
    void setBitrate(uint16_t kbps);

private:
    void stop() override;

    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    void updateConf();

    static void freeBuffer(void *opaque, uint8_t *data);

    AVFrame* createFrame() const;
    AVFrame* fillFrame(AVFrame* frame, AudioBuffer& buffer);
    void     pushFrame(AVFrame* frame);

    AudioCodec m_codec = AudioCodec::Invalid;
    AudioConf m_conf;
    uint16_t m_bitrateKbps = 320;

    AVCodecContext* m_context = nullptr;
    AVFrame* m_partialFrame = nullptr;
    AVPacket* m_packet      = nullptr;
};

} // namespace audio
} // namespace coro
