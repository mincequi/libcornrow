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

#include <coro/core/Sink.h>

struct AUDIOPLAY_STATE_T;

namespace coro {
namespace pi {

class PiHdmiAudioSink : public core::Sink
{
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{
                { { audio::AudioCapRaw<int16_t> {
                            audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                                    audio::Channels::Quad } },
                  { core::NoCap {} } }
               }};
    }

    PiHdmiAudioSink();
    virtual ~PiHdmiAudioSink();

private:
    const char* name() const override;
    void onStop() override;
    audio::AudioConf onProcess(const audio::AudioConf& conf, core::Buffer& buffer) override;

    AUDIOPLAY_STATE_T*  m_handle = nullptr;
    audio::AudioConf  m_conf;
};

} // namespace pi
} // namespace coro
