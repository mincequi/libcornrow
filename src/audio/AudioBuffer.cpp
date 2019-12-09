#include "audio/AudioBuffer.h"

#include <cstring>
#include <iostream>

namespace coro {
namespace audio {

std::list<AudioBuffer> AudioBuffer::split(size_t size, size_t reservedSize) const
{
    std::list<AudioBuffer> buffers;
    for (size_t i = 0; i < m_size; i += size) {
        buffers.emplace_back( (char*)(m_buffer.data())+m_offset+i, size, reservedSize );
    }
    return buffers;
}

} // namespace audio
} // namespace coro
