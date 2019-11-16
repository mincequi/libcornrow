#pragma once

#include <cstdint>
#include <vector>

namespace coro {
namespace core {

class Buffer
{
public:
    Buffer();
    ~Buffer();

    uint8_t* data();
    size_t size() const;

    uint8_t* acquire(size_t size);
    void commit(size_t newSize);

protected:
    std::vector<uint8_t> m_buffer;
    uint8_t* m_data = m_buffer.data();
    size_t   m_size = m_buffer.size();

    uint8_t* m_acquiredData = nullptr;
};

} // namespace core
} // namespace coro
