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

#include <core/Node.h>
#include <core/BufferPool.h>
#include <loguru/loguru.hpp>

#include <cstring>
#include <iostream>
#include <vector>

namespace coro {
namespace core {

class BufferPrivate {
public:
    mutable std::vector<int32_t> buffer; // use int32_t internally to align to 4 byte borders
    size_t   size = 0;
    size_t   offset = 0;
    mutable size_t  acquiredOffset = 0;
    audio::AudioConf audioConf;
};

BufferPtr Buffer::create(size_t reservedSize, const Node* caller) {
    return BufferPool::instance().acquire(reservedSize, caller);
}

Buffer::Buffer(size_t size)
    : d(new BufferPrivate) {
    // Add 3 to round up to next multiple of 4.
    d->buffer.resize((size + 3) / 4);
}

Buffer::Buffer(const char* data, size_t size, size_t reservedSize, size_t offset) :
    d(new BufferPrivate) {
    d->buffer.resize(std::max(size+offset, reservedSize)/4+1);
    d->size = size;
    d->offset = offset;
    std::memcpy(this->data(), data, size);
}

Buffer::~Buffer() {
    delete d;
}

bool Buffer::isValid() const {
    // @TODO(mawe): also check conf here, as soon as we pass conf with buffer.
    return size() > 0;
}

char* Buffer::data() {
    return (char*)(d->buffer.data())+d->offset;
}

const char* Buffer::data() const {
    return (const char*)(d->buffer.data())+d->offset;
}

size_t Buffer::size() const {
    return d->size;
}

size_t Buffer::capacity() const {
    return d->buffer.capacity() * 4;
}

char* Buffer::acquire(size_t size, const core::Node* caller) const {
    // If we have space in front
    if (d->offset >= size) {
        d->acquiredOffset = 0;
        return (char*)d->buffer.data();
    }

    // If we have space at back
    const auto sizeAtBack = d->buffer.size() * 4 - d->offset - d->size - 3;
    if (sizeAtBack >= size) {
        d->acquiredOffset = d->offset + d->size;
        d->acquiredOffset += d->acquiredOffset % 4;
        return (char*)d->buffer.data() + d->acquiredOffset;
    }

    // Create space
    auto orgSize = d->buffer.capacity() * 4;    // size in bytes
    auto newSize = d->buffer.size() + (size - sizeAtBack) / 4; // + 1; //@TODO(mawe): let's see if +1 is actually needed
    auto isReallocated = d->buffer.capacity() < newSize;
    d->buffer.resize(newSize);
    d->acquiredOffset = d->offset + d->size;
    d->acquiredOffset += d->acquiredOffset % 4;
    if (caller && isReallocated) {
        LOG_F(INFO, "%s reallocated buffer. %zu -> %zu bytes", caller->name(), orgSize, d->buffer.capacity()*4);
    } else if (isReallocated) {
        LOG_F(INFO, "Buffer reallocated. %zu -> %zu bytes", orgSize, d->buffer.capacity()*4);
    }
    return (char*)d->buffer.data() + d->acquiredOffset;
}

void Buffer::commit(size_t size) {
    d->offset = d->acquiredOffset;
    d->size = size;
}

void Buffer::prepend(const char* data, uint32_t size) {
    if (d->offset >= size) {
        d->offset -= size;
        d->size += size;
        std::memcpy(d->buffer.data()+d->offset, data, size);
    } else {
        auto dest = acquire(d->size+size);
        std::memcpy(dest, data, size);
        std::memcpy(dest+size, d->buffer.data()+d->offset, d->size);
        commit(d->size+size);
    }
}

void Buffer::grow(size_t size) {
    d->size = std::min(d->buffer.size()*4, size);
}

void Buffer::shrink(size_t size) {
    d->size = std::min(d->size, size);
}

void Buffer::clear() {
    d->offset = 0;
    d->size = 0;

    LOG_F(INFO, "Buffer cleared");
}

void Buffer::trimFront(size_t size) {
    if (size > d->size) {
        return;
    }
    d->offset += size;
    d->size -= size;
}

void Buffer::trimBack(size_t size) {
    if (size > d->size) {
        return;
    }
    d->size -= size;
}

audio::AudioConf& Buffer::audioConf() {
    return d->audioConf;
}

} // namespace core
} // namespace coro
