#pragma once

#include <coro/core/Buffer.h>

namespace coro {
namespace audio {

class AudioBuffer : public core::Buffer
{
public:
    using core::Buffer::Buffer;

    std::list<AudioBuffer> split(size_t size) const;
};

} // namespace audio
} // namespace coro

