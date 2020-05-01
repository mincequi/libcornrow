#pragma once

#include <coro/audio/AudioTypes.h>

namespace coro {
namespace audio {

class AudioConf
{
public:
    AudioCodec  codec = AudioCodec::Invalid;
    SampleRate  rate = SampleRate::Invalid;
    ChannelFlags channels = Channels::Invalid;
    bool        isRtpPayloaded = false;

    uint32_t frameSize() const;

    bool operator==(const AudioConf& other) const;
    bool operator!=(const AudioConf& other) const;
};

} // namespace audio
} // namespace coro
