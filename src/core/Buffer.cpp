#include "core/Buffer.h"

namespace coro {
namespace core {

Buffer::Buffer()
{
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

} // namespace core
} // namespace coro
