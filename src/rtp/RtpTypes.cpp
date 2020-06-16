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

    // Marker bit MUST NOT be set
    if (marker) {
        return false;
    }

    // Payload type
    if (payloadType < 96) {
        return false;
    }

    return true;
}

uint32_t RtpHeader::size() const
{
    uint8_t size = sizeof(RtpHeader);
    size -= (16-csrcCount) * 4;
    size -= (1-extension) * 4;
    return size;
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
