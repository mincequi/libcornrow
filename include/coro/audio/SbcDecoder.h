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

#include <coro/audio/AudioNode.h>

typedef struct sbc_struct sbc_t;

namespace coro {
namespace audio {

class SbcDecoder : public AudioNode
{
public:
    SbcDecoder();
    ~SbcDecoder();

    static constexpr std::array<audio::AudioCap,1> inCaps() {
        return {{ { AudioCodec::Sbc | AudioCodec::RtpPayload, SampleRates::Any, ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ { AudioCodec::RawInt16,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    Channels::Mono | Channels::Stereo } }};
    }

protected:
    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    AudioConf m_conf;

    sbc_t* m_sbc;
};

} // namespace audio
} // namespace coro
