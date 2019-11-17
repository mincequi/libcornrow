#pragma once

#include <coro/core/Buffer.h>

namespace coro {
namespace audio {

class AudioBuffer : public core::Buffer
{
public:
    using core::Buffer::Buffer;

    std::list<AudioBuffer> split(size_t size, size_t reservedSize = 0) const;
};

} // namespace audio
} // namespace coro

