#pragma once

#include <stdint.h>

#include <coro/core/Flags.h>

namespace coro {
namespace core {

enum /*class*/ MediaType : uint8_t
{
    Audio = 0x01,
    Video = 0x02,
    Container = 0x04,
};
typedef Flags<MediaType> MediaTypes;

} // namespace core
} // namespace coro
