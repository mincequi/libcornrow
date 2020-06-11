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

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Source.h>
#include <coro/core/UdpSource.h>

namespace coro {
namespace audio {

class ScreamSource : public core::UdpSource
{
public:
    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ { audio::AudioCodec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    ScreamSource();
    virtual ~ScreamSource();

private:
    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    const char* name() const override;
};

} // namespace audio
} // namespace coro
