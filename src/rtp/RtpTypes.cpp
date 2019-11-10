#include "RtpTypes.h"

namespace coro {
namespace rtp {

bool RtpHeader::isValidSbc() const
{
    //uint16_t version:2;
    //uint16_t padding:1;
    //uint16_t extension:1;
    //uint16_t csrcCount:4;
    //uint16_t marker:1;
    //uint16_t payloadType:7;

    // Marker bit MUST not be set
    if (marker) {
        return false;
    }

    // Payload type
    if (payloadType < 96) {
        return false;
    }

    return true;
}

bool RtpSbcHeader::isValid() const
{
    // frameCount shall never be zero
    if (frameCount == 0) {
        return false;
    }

    // rfa SHOULD never be set
    //if (rfa) {
    //    return false;
    //}

    // if fragmented, first and last cannot occur at the same time
    if (isFragmented && isFirstSegment && isLastFragment) {
        return false;
    }

    // if not fragmented, start and end shall be zero
    if (!isFragmented && (isFirstSegment || isLastFragment)) {
        return false;
    }

    return true;
}

bool SbcFrameHeader::isValid() const
{
    if (syncWord != 0x9c) {
        return false;
    }

    return true;
}

} // namespace rtp
} // namespace coro
