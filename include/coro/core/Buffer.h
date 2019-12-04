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
    Buffer(const char* data, size_t size, size_t reservedSize = 0);
    virtual ~Buffer();

    char* data();
    uint32_t size() const;

    char* acquire(size_t size);
    void commit(size_t newSize);

    void clear();

    template <class T>
    std::list<T> split(size_t size) const;

protected:
    std::vector<int32_t> m_buffer; // use int32_t internally to align to 4 byte borders
    size_t   m_size = 0;
    size_t   m_offset = 0;
    size_t   m_acquiredOffset = 0;
};

} // namespace core
} // namespace coro
