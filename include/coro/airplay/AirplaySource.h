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

#include <coro/core/Source.h>

namespace coro {
namespace airplay {

class AirplaySource : public core::Source
{
public:
    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ { audio::AudioCodec::RawInt16,
                    audio::SampleRate::Rate44100,
                    audio::Channels::Stereo } }};
    }

    struct Config {
        std::string name;
        uint16_t port = 0;
        uint16_t bufferTimeMs = 2000;
    };
    AirplaySource(const Config& config);
    virtual ~AirplaySource();

private:
    const char* name() const override;

    class AirplaySourcePrivate* const d;
};

} // namespace airplay
} // namespace coro
