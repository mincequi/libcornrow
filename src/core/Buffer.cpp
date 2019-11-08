#include "Buffer.h"

#include <xxhash.h>

namespace coro {
namespace core {

uint32_t Buffer::hash(GstBuffer* buffer)
{
    auto memory = gst_buffer_get_all_memory(buffer);
    GstMapInfo mapInfo;
    gst_memory_map(memory, &mapInfo, GST_MAP_READ);
    uint32_t hash = XXH32(mapInfo.data, mapInfo.size, 0);
    gst_memory_unmap(memory, &mapInfo);

    return hash;
}

} // namespace core
} // namespace coro
