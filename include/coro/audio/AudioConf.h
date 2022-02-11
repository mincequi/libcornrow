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

namespace coro {
namespace audio {

class AudioConf {
public:
    AudioCodec  codec = AudioCodec::Invalid;
    SampleRate  rate = SampleRate::Invalid;
    ChannelFlags channels = Channels::Invalid;
    bool        isRtpPayloaded = false;

    uint32_t frameSize() const;

    bool operator==(const AudioConf& other) const;
    bool operator!=(const AudioConf& other) const;
};

} // namespace audio
} // namespace coro
