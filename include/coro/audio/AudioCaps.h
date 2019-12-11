#pragma once

#include <coro/audio/AudioTypes.h>

namespace coro {
namespace audio {
class Node;

class AudioCaps
{
public:
    static constexpr AudioCaps intersect(const AudioCaps& in, const AudioCaps& out);

    Codecs      codecs = Codecs::Any;
    SampleRates rates = SampleRates::Any;
    ChannelFlags channels = ChannelFlags::Any;

    constexpr bool isValid() const {
        return codecs && rates && channels;
    }
};

constexpr AudioCaps AudioCaps::intersect(const AudioCaps& in, const AudioCaps& out)
{
    // If in is RTP payloaded, but out does not accept it, we fail
    if (in.codecs.testFlag(AudioCodec::RtpPayload) && !out.codecs.testFlag(AudioCodec::RtpPayload)) {
        return AudioCaps { AudioCodec::Invalid };
    }
    return { (in.codecs & out.codecs), (in.rates & out.rates), (in.channels & out.channels) };
}

} // namespace audio
} // namespace coro
