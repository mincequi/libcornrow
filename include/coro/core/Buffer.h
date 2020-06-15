#pragma once

#include <cstdint>
#include <list>
#include <vector>

namespace coro {
namespace audio {
class AudioBuffer;
}
namespace core {

class Node;

class Buffer
{
public:
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

    // @TODO(mawe): change to unsigned char (uint8_t)?
    // openssl, apple alac prefer unsigned.
    char* data();
    const char* data() const;
    size_t size() const;

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

    template <class T>
    std::list<T> split(size_t size) const;

    /**
     * @brief trimFront
     */
    void trimFront(size_t size);

    /**
     * @brief trimBack
     */
    void trimBack(size_t size);

protected:
    mutable std::vector<int32_t> m_buffer; // use int32_t internally to align to 4 byte borders
    size_t   m_size = 0;
    size_t   m_offset = 0;
    mutable size_t   m_acquiredOffset = 0;
};

} // namespace core
} // namespace coro
