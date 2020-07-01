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

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{
                { { AudioCap { AudioCodec::Sbc | AudioCodec::RtpPayload }}, // in
                  { AudioCapRaw<int16_t> {  // out
                               SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                               Channels::Mono | Channels::Stereo } }}
               }};
    }

protected:
    const char* name() const override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;

private:
    AudioConf m_conf;

    sbc_t* m_sbc;
};

} // namespace audio
} // namespace coro
