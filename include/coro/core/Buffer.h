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

#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <vector>

namespace coro {
namespace core {

class Buffer;
class Node;

struct BufferDeleter {
    void operator()(Buffer* buffer);
};
using BufferPtr = std::unique_ptr<Buffer, BufferDeleter>;

class Buffer
{
public:
    static BufferPtr create(size_t reservedSize = 0, const Node* caller = nullptr);

    Buffer(size_t reservedSize = 0);
    /**
     * Construct new buffer with given data, which gets deeply copied.
     *
     * @param data data to be deeply copied
     * @param size size of data to be deeply copied
     * @param reservedSize overall reserved size to prevent reallocs
     * @param offset offset to prepend to buffer (to later add headers, etc)
     */
    Buffer(const char* data, size_t size, size_t reservedSize = 0, size_t offset = 0);
    virtual ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = default;

    /**
     * @brief isValid
     */
    bool isValid() const;

    // @TODO(mawe): change to unsigned char (uint8_t)?
    // openssl, apple alac prefer unsigned.
    // also std::byte inherits from unsigned char.
    char* data();
    const char* data() const;
    size_t size() const;

    size_t capacity() const;

    char* acquire(size_t size, const core::Node* caller = nullptr) const;
    void commit(size_t newSize);

    /**
     * @brief prepend
     * @param data
     * @param size
     */
    void prepend(const char* data, uint32_t size);

    /**
     * @brief grow
     */
    void grow(size_t size);

    /**
     * @brief shrink
     */
    void shrink(size_t size);

    /**
     * @brief clear
     */
    void clear();

    /**
     * @brief trimFront
     */
    void trimFront(size_t size);

    /**
     * @brief trimBack
     */
    void trimBack(size_t size);

private:
    // @TODO(mawe): move these members to Private class
    mutable std::vector<int32_t> m_buffer; // use int32_t internally to align to 4 byte borders
    size_t   m_size = 0;
    size_t   m_offset = 0;
    mutable size_t   m_acquiredOffset = 0;

    friend class BufferPool;
};

} // namespace core
} // namespace coro
