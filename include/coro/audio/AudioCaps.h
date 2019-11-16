#pragma once

#include <coro/core/Caps.h>
#include <coro/audio/Types.h>

namespace coro {
namespace audio {

class Caps //: public core::Caps
{
public:
    Codecs      codecs = Codecs::Any;
    SampleRates rates = SampleRates::Any;
    ChannelFlags channels = ChannelFlags::Any;

private:
    template<class In, class Out>
    static constexpr bool canIntersect(const In& in, const Out& out);

    friend class Node;
};

template<class In, class Out>
constexpr bool Caps::canIntersect(const In& in, const Out& out)
{
    // If in is RTP payloaded, but out does not accept it, we fail
    if (in.codecs.testFlag(Codec::RtpPayload) && !out.codecs.testFlag(Codec::RtpPayload)) {
        return false;
    }
    return ((in.codecs & out.codecs) &&
            (in.rates & out.rates) &&
            (in.channels & out.channels));
}

} // namespace audio
} // namespace coro
