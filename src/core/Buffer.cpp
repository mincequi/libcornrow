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

#include <audio/AudioBuffer.h>
#include <core/Node.h>
#include <loguru/loguru.hpp>

template std::list<coro::audio::AudioBuffer> coro::core::Buffer::split(size_t) const;

namespace coro {
namespace core {

Buffer::Buffer(size_t size)
{
    m_buffer.resize(size/4+1);
    m_size = 0;
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

char* Buffer::acquire(size_t size, const core::Node* caller) const
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
    auto orgSize = m_buffer.size()*4;
    m_buffer.resize(m_buffer.size()+(size-sizeAtBack) / 2 /*4*/ +1); // let's try 2 instead of 4 to reduce reallocs.
    m_acquiredOffset = m_offset+m_size;
    m_acquiredOffset += m_acquiredOffset%4;
    if (caller) {
        LOG_F(INFO, "%s reallocates buffer. %zu -> %zu bytes", caller->name(), orgSize, m_buffer.size()*4);
    } else {
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

void Buffer::trimBack(size_t size)
{
    if (size > m_size) {
        return;
    }
    m_size -= size;
}

} // namespace core
} // namespace coro
