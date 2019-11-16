#pragma once

#include <coro/audio/Types.h>

namespace coro {
namespace audio {

class Conf
{
public:
    Codec      codec;
    SampleRate rate;
    Channels   channels;
    bool       isRtpPayloaded = false;
};

} // namespace audio
} // namespace coro
