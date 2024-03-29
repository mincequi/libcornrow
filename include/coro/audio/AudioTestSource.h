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
#include <coro/core/Source.h>

namespace coro {
namespace audio {

template <AudioCodec AC, SampleRate SR, Channels C>
class AudioTestSource : public core::Source
{
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{
                { { core::NoCap {} }, // in
                  { AudioCap { AC, SR, C } } } // out
               }};
    }

    struct Config {
        uint16_t numFramesPerBuffer = 352;
        uint16_t numBuffers = 125;
    };

    AudioTestSource(const Config& config = Config());
    virtual ~AudioTestSource();

    const char* name() const override;

private:
    void onStart() override;
    void onStop() override;
    void onProcess(core::BufferPtr& buffer) override;

    Config m_config;
    AudioConf m_audioConfig = { AC, SR, C };
    core::BufferPtr m_buffer;
};

} // namespace audio
} // namespace coro

#include "AudioTestSource.cpp"
