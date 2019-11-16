#pragma once

//#include <coro/audio/AudioCaps.h>
#include <coro/core/Types.h>

#include <array>
#include <type_traits>

namespace coro {
namespace core {

class Caps {
//public:
    //int      maxValue = 0;

private:
    template<class In, class Out>
    static constexpr bool canIntersect(const In& in, const Out& out);

    friend class Node;
};

template<class In, class Out>
constexpr bool Caps::canIntersect(const In& in, const Out& out)
{
    return true;
}

} // namespace core
} // namespace coro
