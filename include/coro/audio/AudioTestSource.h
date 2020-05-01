#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Source.h>

namespace coro {
namespace audio {

class AudioTestSource : public Source
{
public:
    static constexpr std::array<AudioCaps,1> outCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::RawFloat32 } }};
    }

    AudioTestSource(const AudioConf& audioConf = { AudioCodec::RawInt16, SampleRate::Rate44100, Channels::Stereo },
                    uint32_t numFramesPerBuffer = 44100,
                    uint32_t numBuffers = 100);
    virtual ~AudioTestSource();

    const char* name() const override;

private:
    void doStart() override;

    AudioConf   m_conf;
    uint32_t    m_numFramesPerBuffer;
    uint32_t    m_numBuffers;
    AudioBuffer m_buffer;
};

} // namespace audio
} // namespace coro
