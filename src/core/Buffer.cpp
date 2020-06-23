/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Buffer.h"

#include <cstring>
#include <iostream>

#include <core/Node.h>
#include <core/BufferPool.h>
#include <loguru/loguru.hpp>

namespace coro {
namespace core {

BufferPtr Buffer::create(size_t reservedSize, const Node* caller)
{
    return BufferPool::instance().acquire(reservedSize, caller);
}

Buffer::Buffer(size_t size)
{
    // Add 3 to round up to next multiple of 4.
    m_buffer.resize((size + 3) / 4);
}

Buffer::Buffer(const char* data, size_t size, size_t reservedSize, size_t offset)
{
    m_buffer.resize(std::max(size+offset, reservedSize)/4+1);
    m_size = size;
    m_offset = offset;
    std::memcpy(this->data(), data, size);
}

Buffer::~Buffer()
{
}

bool Buffer::isValid() const
{
    // @TODO(mawe): also check conf here, as soon as we pass conf with buffer.
    return size() > 0;
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

size_t Buffer::capacity() const
{
    return m_buffer.capacity() * 4;
}

char* Buffer::acquire(size_t size, const core::Node* caller) const
{
    // If we have space in front
    if (m_offset >= size) {
        m_acquiredOffset = 0;
        return (char*)m_buffer.data();
    }

    // If we have space at back
    const auto sizeAtBack = m_buffer.size() * 4 - m_offset - m_size;
    if (sizeAtBack >= size) {
        m_acquiredOffset = m_offset + m_size;
        m_acquiredOffset += m_acquiredOffset % 4;
        return (char*)m_buffer.data() + m_acquiredOffset;
    }

    // Create space
    auto orgSize = m_buffer.size() * 4;
    auto newSize = m_buffer.size() + (size - sizeAtBack) / 4 + 1;
    m_buffer.resize(newSize);
    m_acquiredOffset = m_offset + m_size;
    m_acquiredOffset += m_acquiredOffset % 4;
    if (caller && newSize > m_buffer.capacity()) {
        LOG_F(INFO, "%s reallocated buffer. %zu -> %zu bytes", caller->name(), orgSize, m_buffer.size()*4);
    } else if (newSize > m_buffer.capacity()) {
        LOG_F(INFO, "buffer reallocated. %zu -> %zu bytes", orgSize, m_buffer.size()*4);
    }
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
        std::memcpy(m_buffer.data()+m_offset, data, size);
    } else {
        auto dest = acquire(m_size+size);
        std::memcpy(dest, data, size);
        std::memcpy(dest+size, m_buffer.data()+m_offset, m_size);
        commit(m_size+size);
    }
}

void Buffer::grow(size_t size)
{
    m_size = std::min(m_buffer.size()*4, size);
}

void Buffer::shrink(size_t size)
{
    m_size = std::min(m_size, size);
}

void Buffer::clear()
{
    m_offset = 0;
    m_size = 0;
}

void Buffer::trimFront(size_t size)
{
    if (size > m_size) {
        return;
    }
    m_offset += size;
    m_size -= size;
}

void Buffer::trimBack(size_t size)
{
    if (size > m_size) {
        return;
    }
    m_size -= size;
}

} // namespace core
} // namespace coro
