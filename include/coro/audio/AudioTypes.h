#pragma once

#include <cstdint>
#include <coro/core/Flags.h>

namespace coro {
namespace audio {

enum class AudioCodec : uint16_t
{
    Invalid     = 0x0000,

    RawInt16    = 0x0001,
    RawFloat32  = 0x0002,

    Sbc = 0x0004,
    Aac = 0x0008,
    Mp3 = 0x0010,
    Ac3 = 0x0020,
    Aptx = 0x0040,

    RtpPayload = 0x8000
};
using Codecs = core::Flags<AudioCodec>;
DECLARE_OPERATORS_FOR_FLAGS(Codecs)
int size(AudioCodec codec);

enum class SampleRate : uint8_t
{
    Invalid = 0,
    Rate16000 = 0x01,
    Rate32000 = 0x02,
    Rate44100 = 0x04,
    Rate48000 = 0x08,
    Rate88200 = 0x10,
    Rate96000 = 0x20,
    Rate176400 = 0x40,
    Rate192000 = 0x80
};
using SampleRates = core::Flags<SampleRate>;
DECLARE_OPERATORS_FOR_FLAGS(SampleRates)
int toInt(SampleRate rate);

enum class Channels : uint8_t
{
    Invalid = 0,
    Mono    = 0x01,
    Center  = Mono,
    Stereo  = 0x02,
    Lfe     = 0x04,
    RearStereo = 0x08,
    RearCenter = 0x10,
    Quad    = Stereo | RearStereo,
    Surround50 = Quad | Center,
    Surround51 = Surround50 | Lfe,

};
using ChannelFlags = core::Flags<Channels>;
DECLARE_OPERATORS_FOR_FLAGS(ChannelFlags)
int toInt(ChannelFlags channels);

} // namespace audio
} // namespace coro
