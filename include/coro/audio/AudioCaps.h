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

#include <coro/audio/AudioTypes.h>
#include <coro/core/Types.h>

namespace coro {
namespace audio {

class AudioCap
{
public:
    static constexpr AudioCap intersect(const AudioCap& in, const AudioCap& out)
    {
        // If in is RTP payloaded, but out does not accept it, we fail
        if (in.codecs.testFlag(AudioCodec::RtpPayload) && !out.codecs.testFlag(AudioCodec::RtpPayload)) {
            return AudioCap { AudioCodec::Invalid };
        }
        return { (in.codecs & out.codecs), (in.rates & out.rates), (in.channels & out.channels) };
    }

    AudioCodecs codecs = AudioCodecs::Any;
    SampleRates rates = SampleRates::Any;
    ChannelFlags channels = ChannelFlags::Any;
    core::CapFlags  flags = 0;

    constexpr bool isValid() const {
        return codecs && rates && channels;
    }
};

template <typename T>
class AudioCapRaw
{
public:
    static constexpr AudioCapRaw<T> intersect(const AudioCapRaw<T>& in, const AudioCapRaw<T>& out) {
        // If in is RTP payloaded, but out does not accept it, we fail
        if (in.flags != out.flags) {
            return AudioCapRaw<T> { SampleRate::Invalid };
        }
        return AudioCapRaw<T> { (in.rates & out.rates), (in.channels & out.channels) };
    }

    SampleRates rates = SampleRates::Any;
    ChannelFlags channels = ChannelFlags::Any;
    core::CapFlags  flags = 0;

    constexpr bool isValid() const {
        return rates && channels;
    }
};

} // namespace audio
} // namespace coro
