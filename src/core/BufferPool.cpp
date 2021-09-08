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

#include "core/BufferPool.h"

#include "core/Buffer.h"
#include "loguru/loguru.hpp"

#include <functional>
#include <list>

namespace coro {
namespace core {

using namespace std::placeholders;

static std::list<Buffer*> s_buffers;

void BufferDeleter::operator()(Buffer* buffer) {
    if (!buffer) {
        return;
    }
    buffer->clear();
    s_buffers.push_back(buffer);
}

BufferPool::BufferPool() {
}

BufferPool::~BufferPool() {
}

BufferPool& BufferPool::instance() {
    static BufferPool bufferPool;
    return bufferPool;
}

BufferPtr BufferPool::acquire(size_t size, const core::Node* caller) const {
    Buffer* buffer = nullptr;
    for (auto b : s_buffers) {
        // @TODO(mawe): for now simple strategy: use first element that can hold the desired size.
        if (b->capacity() >= size) {
            buffer = b;
            s_buffers.remove(b);
            break;
        }
    }
    // If no valid buffer found, acquire new one.
    if (!buffer) {
        LOG_F(INFO, "New buffer created. size: %zu", size);
        buffer = new Buffer(size);
    }

    return BufferPtr(buffer, BufferDeleter());
}

} // namespace core
} // namespace coro
