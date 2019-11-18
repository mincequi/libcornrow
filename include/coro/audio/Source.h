#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Node.h>

namespace coro {
namespace audio {

class Source : public Node
{
public:
    static constexpr std::array<AudioCaps,0> outCaps() { return {}; }

    Source();
    virtual ~Source();
};

} // namespace audio
} // namespace coro
