/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    Eac3 = 0x0040,
    Aptx = 0x0080,
    Alac = 0x0100,
    Wav = 0x0200,

    RtpPayload = 0x8000,    // @TODO(mawe): remove RTP payload flag here

    Unknown = 0x4000
};
using AudioCodecs = core::Flags<AudioCodec>;
DECLARE_OPERATORS_FOR_FLAGS(AudioCodecs)
uint8_t size(AudioCodec codec);

enum class SampleRate : uint8_t
{
    Invalid = 0,
    //Rate16000 = 0x01,
    RateUnknown = 0x01,
    Rate32000 = 0x02,
    Rate44100 = 0x04,
    Rate48000 = 0x08,
    //Rate88200 = 0x10,
    Rate96000 = 0x20,
    //Rate176400 = 0x40,
    Rate192000 = 0x80
};
using SampleRates = core::Flags<SampleRate>;
DECLARE_OPERATORS_FOR_FLAGS(SampleRates)
uint32_t toInt(SampleRate rate);

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
uint8_t toInt(ChannelFlags channels);

} // namespace audio
} // namespace coro
