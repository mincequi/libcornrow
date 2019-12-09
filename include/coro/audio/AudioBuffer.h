#pragma once

#include <coro/core/Buffer.h>

namespace coro {
namespace audio {

class AudioBuffer : public core::Buffer
{
public:
    using core::Buffer::Buffer;
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer(AudioBuffer&&) = default;


    std::list<AudioBuffer> split(size_t size, size_t reservedSize = 0) const;
};

} // namespace audio
} // namespace coro

