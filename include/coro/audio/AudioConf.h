#pragma once

#include <coro/audio/AudioTypes.h>

namespace coro {
namespace audio {

class AudioConf
{
public:
    Codec       codec = Codec::Invalid;
    SampleRate  rate = SampleRate::Invalid;
    ChannelFlags channels = Channels::Invalid;
    bool        isRtpPayloaded = false;

    int frameSize() const;

    bool operator==(const AudioConf& other) const;
    bool operator!=(const AudioConf& other) const;
};

} // namespace audio
} // namespace coro
