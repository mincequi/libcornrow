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

#include <coro/core/Node.h>
#include <coro/audio/AudioCaps.h>

namespace coro {
namespace rtp {

class RtpHeader;

template<audio::AudioCodec codec>
class RtpDecoder : public core::Node
{
public:
    RtpDecoder();
    ~RtpDecoder();

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{{ { core::AnyCap {} }, // in
                   { audio::AudioCap { codec,
                                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                                    audio::Channels::Stereo } } // out
               }}};
    }

private:
    const char* name() const override;

    audio::AudioConf onProcess(const audio::AudioConf& conf, core::Buffer& buffer) override;
    void onStop() override;
    audio::AudioConf onProcessCodec(const RtpHeader& header, core::Buffer& buffer);

    bool     m_isFlushed = true;
    uint16_t m_seq = 0;
};

} // namespace rtp
} // namespace coro
