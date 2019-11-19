#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/core/Types.h>

#include <array>
#include <type_traits>

namespace coro {
namespace core {

class Caps {
private:
    template<class In, class Out>
    static constexpr bool canIntersect(const In& in, const Out& out);

    friend class Node;
};

template<class In, class Out>
constexpr bool Caps::canIntersect(const In& in, const Out& out)
{
    if (std::is_base_of<coro::audio::AudioCaps, In>::value &&
            std::is_base_of<coro::audio::AudioCaps, Out>::value) {
        return audio::AudioCaps::canIntersect(in, out);
    }
    return false;
}

} // namespace core
} // namespace coro
