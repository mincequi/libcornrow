#include "core/Buffer.h"

#include <cstring>

#include <audio/AudioBuffer.h>

template std::list<coro::audio::AudioBuffer> coro::core::Buffer::split(size_t) const;

namespace coro {
namespace core {

Buffer::Buffer(size_t size)
{
    m_buffer.resize(size);
    m_data = m_buffer.data();
    m_size = 0;
}

Buffer::Buffer(uint8_t* data, size_t size, size_t reservedSize)
{
    m_buffer.resize(std::max(size, reservedSize));
    m_data = m_buffer.data();
    m_size = size;

    std::memcpy(m_data, data, size);
}

Buffer::~Buffer()
{
}

uint8_t* Buffer::data()
{
    return m_data;
}

size_t Buffer::size() const
{
    return m_size;
}

uint8_t* Buffer::acquire(size_t size)
{
    // Current data pointer minus begin of buffer is the offset
    size_t offset = m_data-m_buffer.data();

    // If we have space in front
    if (offset >= size) {
        m_acquiredData = m_buffer.data();
        return m_acquiredData;
    }
    // If we have space at back
    if (m_buffer.size()-offset-m_size >= size) {
        m_acquiredData = m_data+m_size;
        return m_acquiredData;
    }
    // Make space
    m_buffer.resize(m_buffer.size()-offset-m_size+size);
    m_data = m_buffer.data()+offset;
    m_acquiredData = m_data+m_size;
    return m_acquiredData;
}

void Buffer::commit(size_t size)
{
    m_data = m_acquiredData;
    m_size = size;
}

template <class T>
std::list<T> Buffer::split(size_t size) const
{
    std::list<T> buffers;
    for (size_t i = 0; i < m_size; i += size) {
        buffers.push_back(T(m_data+i, size));
    }
    return buffers;
}

} // namespace core
} // namespace coro
