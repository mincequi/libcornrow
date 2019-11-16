#pragma once

#include <coro/audio/AudioTypes.h>

namespace coro {
namespace audio {

class AudioConf
{
public:
    Codec      codec;
    SampleRate rate;
    Channels   channels;
    bool       isRtpPayloaded = false;
};

} // namespace audio
} // namespace coro
