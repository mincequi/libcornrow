#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/core/Node.h>

namespace coro {
namespace core {

class AppSink : public Node
{
public:
    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { } }};
    }

    AppSink();
    ~AppSink();

};

} // namespace core
} // namespace coro
