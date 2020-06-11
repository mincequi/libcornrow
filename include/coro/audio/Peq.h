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

#include "TBiquad.h"

#include <deque>
#include <mutex>

namespace coro
{
namespace audio
{

class Peq : public audio::AudioNode
{
public:
    Peq();

    static constexpr std::array<audio::AudioCap,1> inCaps() {
        return {{ { AudioCodec::RawFloat32,
                    SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ { AudioCodec::RawFloat32,
                    SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    Channels::Stereo } }};
    }

    void setVolume(float volume);

    void setFilters(const std::vector<Filter> filters);
    std::vector<Filter> filters();

private:
    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    float               m_volume = 1.0;
    std::deque<TBiquad<float, float>> m_tBiquads;

    AudioConf m_conf;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
