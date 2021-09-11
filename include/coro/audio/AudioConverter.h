/*
 * Copyright (C) 2021 Manuel Weichselbaumer <mincequi@web.de>
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

namespace coro {
namespace audio {

template <class InT, class OutT>
class AudioConverter : public AudioNode {
public:
    AudioConverter();
    virtual ~AudioConverter();

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 3> caps() {
        return {{
                { { AudioCapRaw<InT> { } }, // in
                  { AudioCapRaw<OutT> { } }},

                { { AudioCap { AudioCodec::RawInt16 } }, // in
                  { AudioCap { AudioCodec::RawFloat32 } }},

                { { AudioCap { AudioCodec::RawFloat32 } }, // in
                  { AudioCap { AudioCodec::RawInt16 } }}
               }};
    }

private:
    const char* name() const override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;
};

} // namespace audio
} // namespace coro
