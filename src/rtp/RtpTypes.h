#pragma once

#include <stdint.h>

namespace coro {
namespace rtp {

class RtpHeader {
public:
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint16_t csrcCount:4;
    uint16_t extension:1;
    uint16_t padding:1;
    uint16_t version:2;
    uint16_t payloadType:7;
    uint16_t marker:1;
#else
    uint16_t version:2;
    uint16_t padding:1;
    uint16_t extension:1;
    uint16_t csrcCount:4;
    uint16_t marker:1;
    uint16_t payloadType:7;
#endif
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
    uint32_t csrc[16];  // optional
    uint32_t headerExtension; // optional

    bool isValidSbc() const;
    uint32_t size() const; // max 80 bytes
} __attribute__ ((packed));

class RtpSbcHeader {
public:
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t frameCount:4;
    uint8_t reserved:1;
    uint8_t isLastFragment:1;
    uint8_t isFirstSegment:1;
    uint8_t isFragmented:1;
#else
    uint8_t isFragmented:1;
    uint8_t isFirstSegment:1;
    uint8_t isLastFragment:1;
    uint8_t rfa:1;
    uint8_t frameCount:4;
#endif

    bool isValid() const;

} __attribute__ ((packed));

class SbcFrameHeader {
public:
    uint8_t syncWord;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t subBandCount:1;
    uint8_t allocMethod:1;
    uint8_t channelMode:2;
    uint8_t blockSize:2;
    uint8_t sampleRate:2;
#else
    uint8_t sampleRate:2;
    uint8_t blockSize:2;
    uint8_t channelMode:2;
    uint8_t allocMethod:1;
    uint8_t subBandCount:1;
#endif

    uint8_t bitPool;
    uint8_t crcCheck;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t reserved:1;
    uint8_t join:7;
#else
    uint8_t join:7;
    uint8_t reserved:1;
#endif

    bool isValid() const;

} __attribute__ ((packed));

} // namespace rtp
} // namespace coro
