#pragma once

#include <coro/core/Buffer.h>

namespace coro {
namespace audio {

class AudioBuffer : public core::Buffer
{
public:
    AudioBuffer();
    ~AudioBuffer();
};

} // namespace audio
} // namespace coro
