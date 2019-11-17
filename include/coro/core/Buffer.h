#pragma once

#include <cstdint>
#include <list>
#include <vector>

namespace coro {
namespace audio {
class AudioBuffer;
}
namespace core {

class Buffer
{
public:
    Buffer(size_t reservedSize = 0);
    Buffer(const uint8_t* data, size_t size, size_t reservedSize = 0);
    virtual ~Buffer();

    uint8_t* data();
    size_t size() const;

    uint8_t* acquire(size_t size);
    void commit(size_t newSize);

    template <class T>
    std::list<T> split(size_t size) const;

protected:
    std::vector<uint8_t> m_buffer;
    size_t   m_size = 0;
    size_t   m_offset = 0;
    size_t   m_acquiredOffset = 0;
};

} // namespace core
} // namespace coro
