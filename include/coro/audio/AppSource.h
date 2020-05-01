#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Source.h>

namespace coro {
namespace audio {

class AppSource : public Source
{
public:
    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { } }};
    }

    AppSource();
    virtual ~AppSource();

    const char* name() const override;

    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;
};

} // namespace audio
} // namespace coro
