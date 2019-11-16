#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/Buffer.h>
#include <coro/audio/Conf.h>
#include <coro/audio/Node.h>

namespace coro {
namespace audio {

class Source : public Node
{
public:
    static constexpr std::array<Caps,0> outCaps() { return {}; }

    virtual void start() {}
    virtual void stop() {}

protected:
    virtual Conf process(const Conf& conf, Buffer& buffer) = 0;
};

} // namespace audio
} // namespace coro
