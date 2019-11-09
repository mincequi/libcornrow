#pragma once

#include <stdint.h>

#include <gst/gstbuffer.h>

namespace coro {
namespace core {

class Buffer {
public:
    static uint16_t hash(GstBuffer*);
};

} // namespace core
} // namespace coro
