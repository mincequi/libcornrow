#pragma once

#include <stdint.h>

namespace coro {
namespace audio {
namespace spdif {

const uint16_t ac3PeriodSize = 1536;
const uint16_t ac3BufferSize = 15360;
const uint16_t ac3FrameSize = 6144;

class SpdifAc3Header {
public:
    SpdifAc3Header(const char* ac3Data, uint32_t size) {

        uint32_t sizeInBits = size*8;

#if __BYTE_ORDER == __LITTLE_ENDIAN
        preamble[5] = ac3Data[5] & 0x7;
        preamble[6] = sizeInBits & 0xFF;
        preamble[7] = sizeInBits >> 8;
#else
        preamble[4] = ac3Data[5] & 0x7;
        preamble[7] = size & 0xFF;
        preamble[6] = size >> 8;
#endif

    }

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t preamble[8] = { 0x72, 0xF8, 0x1F, 0x4E, 0x1, 0x0, 0x0, 0x0 };
#else
    uint8_t preamble[] = { 0xF8, 0x72, 0x4E, 0x1F, 0x0, 0x1, 0x0, 0x0 };
#endif

    static inline uint32_t size() { return 8; }
} __attribute__ ((packed));

} // namespace spdif
} // namespace audio
} // namespace coro
