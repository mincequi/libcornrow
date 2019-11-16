#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/Buffer.h>
#include <coro/audio/Conf.h>
#include <coro/audio/Source.h>

namespace coro {
namespace audio {

class AppSource : public Source
{
public:
    static constexpr std::array<audio::Caps,1> outCaps() {
        return {{ { } }};
    }

    void pushBuffer(const Conf& conf, Buffer& buffer);

protected:
    virtual void start() override;
    virtual void stop() override;

    virtual Conf process(const Conf& conf, Buffer& buffer) override;
};

} // namespace audio
} // namespace coro
