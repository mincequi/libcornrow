#include "audio/AudioConf.h"

namespace coro {
namespace audio {

uint32_t AudioConf::frameSize() const
{
    return toInt(channels)*size(codec);
}

bool AudioConf::operator==(const AudioConf& other) const
{
    return codec == other.codec &&
            rate == other.rate &&
            channels == other.channels &&
            isRtpPayloaded == other.isRtpPayloaded;
}

bool AudioConf::operator!=(const AudioConf& other) const
{
    return !operator==(other);
}

} // namespace audio
} // namespace coro
