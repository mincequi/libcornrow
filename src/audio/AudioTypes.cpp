#include "audio/AudioTypes.h"

namespace coro {
namespace audio {

uint8_t size(AudioCodec codec)
{
    switch (codec) {
    case AudioCodec::Invalid: return 0;
    case AudioCodec::RawFloat32: return 4;
    default: return 2;
    }
    return 0;
}

uint32_t toInt(SampleRate rate)
{
    switch (rate) {
    case SampleRate::Invalid: return -1;
    case SampleRate::Rate16000: return 16000;
    case SampleRate::Rate32000: return 32000;
    case SampleRate::Rate44100: return 44100;
    case SampleRate::Rate48000: return 48000;
    case SampleRate::Rate88200: return 88200;
    case SampleRate::Rate96000: return 96000;
    case SampleRate::Rate176400: return 176400;
    case SampleRate::Rate192000: return 192000;
    }
    return -1;
}

uint8_t toInt(ChannelFlags channels)
{
    int i = 0;
    if (channels.testFlag(Channels::Mono))      i+=1;
    if (channels.testFlag(Channels::Stereo))    i+=2;
    if (channels.testFlag(Channels::Lfe))       i+=1;
    if (channels.testFlag(Channels::RearStereo))    i+=2;

    return i;
}

} // namespace audio
} // namespace coro
