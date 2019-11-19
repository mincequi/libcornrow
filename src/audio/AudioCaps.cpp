#pragma once

#include <audio/Node.h>

namespace coro {
namespace audio {

/*
static constexpr bool AudioCaps::canIntersect(const coro::audio::Node& in, const coro::audio::Node& out)
{
    // If in is RTP payloaded, but out does not accept it, we fail
    if (in.codecs.testFlag(Codec::RtpPayload) && !out.codecs.testFlag(Codec::RtpPayload)) {
        return false;
    }
    return ((in.codecs & out.codecs) &&
            (in.rates & out.rates) &&
            (in.channels & out.channels));
}
*/

} // namespace audio
} // namespace coro
