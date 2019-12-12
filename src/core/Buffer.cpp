#include "core/Buffer.h"

#include <cstring>
#include <iostream>

#include <audio/AudioBuffer.h>
#include <loguru/loguru.hpp>

template std::list<coro::audio::AudioBuffer> coro::core::Buffer::split(size_t) const;

namespace coro {
namespace core {

Buffer::Buffer(size_t size)
{
    m_buffer.resize(size/4+1);
    m_size = 0;
}

Buffer::Buffer(const char* data, uint32_t size, uint32_t reservedSize, uint32_t offset)
{
    m_buffer.resize(std::max(size+offset, reservedSize)/4+1);
    m_size = size;
    m_offset = offset;
    std::memcpy(this->data(), data, size);
}

Buffer::~Buffer()
{
}

char* Buffer::data()
{
    return (char*)(m_buffer.data())+m_offset;
}

const char* Buffer::data() const
{
    return (const char*)(m_buffer.data())+m_offset;
}

size_t Buffer::size() const
{
    return m_size;
}

char* Buffer::acquire(size_t size)
{
    // If we have space in front
    if (m_offset >= size) {
        m_acquiredOffset = 0;
        return (char*)m_buffer.data();
    }
    // If we have space at back
    const auto sizeAtBack = m_buffer.size()*4-m_offset-m_size;
    if (sizeAtBack >= size) {
        m_acquiredOffset = m_offset+m_size;
        m_acquiredOffset += m_acquiredOffset%4;
        return (char*)m_buffer.data()+m_acquiredOffset;
    }
    // Create space
    LOG_F(1, "Buffer reallocated");
    m_buffer.resize(m_buffer.size()+(size-sizeAtBack)/4+1);
    m_acquiredOffset = m_offset+m_size;
    m_acquiredOffset += m_acquiredOffset%4;
    return (char*)m_buffer.data()+m_acquiredOffset;
}

void Buffer::commit(size_t size)
{
    m_offset = m_acquiredOffset;
    m_size = size;
}

void Buffer::prepend(const char* data, uint32_t size)
{
    if (m_offset >= size) {
        m_offset -= size;
        m_size += size;
        std::memcpy(this->data(), data, size);
    } else {
        auto dest = acquire(m_size+size);
        std::memcpy(dest, data, size);
        std::memcpy(dest+size, this->data(), m_size);
        commit(m_size+size);
    }
}

void Buffer::grow(size_t size)
{
    m_size = std::min(m_buffer.size()*4, size);
}

void Buffer::clear()
{
    m_offset = 0;
    m_size = 0;
}

template <class T>
std::list<T> Buffer::split(size_t size) const
{
    std::list<T> buffers;
    for (size_t i = 0; i < m_size; i += size) {
        buffers.emplace_back((char*)m_buffer.data()+m_offset+i, size);
    }
    return buffers;
}

void Buffer::trimFront(size_t size)
{
    if (size > m_size) {
        return;
    }
    m_offset += size;
    m_size -= size;
}

} // namespace core
} // namespace coro
