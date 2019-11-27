#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/core/Types.h>

#include <array>
#include <type_traits>

namespace coro {
namespace core {

class Caps {
private:
    template<class T>
    static constexpr T intersect(const T& in, const T& out);

    constexpr bool isValid() { return false; }

    friend class Node;
};

template<class T>
constexpr T Caps::intersect(const T& in, const T& out)
{
    if (std::is_base_of<coro::audio::AudioCaps, T>::value) {
        return audio::AudioCaps::intersect(in, out);
    }
    return T();
}

} // namespace core
} // namespace coro
