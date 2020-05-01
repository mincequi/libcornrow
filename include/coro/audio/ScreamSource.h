#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Source.h>
#include <coro/core/UdpSource.h>

namespace coro {
namespace audio {

class ScreamSource : public core::UdpSource
{
public:
    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    ScreamSource();
    virtual ~ScreamSource();

private:
    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    const char* name() const override;
};

} // namespace audio
} // namespace coro
