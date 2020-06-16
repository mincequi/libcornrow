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

class AudioTestSource : public core::Source
{
public:
    static constexpr std::array<AudioCap,1> outCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::RawFloat32 } }};
    }

    AudioTestSource(const AudioConf& audioConf = { AudioCodec::RawInt16, SampleRate::Rate44100, Channels::Stereo },
                    uint32_t numFramesPerBuffer = 44100,
                    uint32_t numBuffers = 100);
    virtual ~AudioTestSource();

    const char* name() const override;

private:
    void onStart() override;

    AudioConf   m_conf;
    uint32_t    m_numFramesPerBuffer;
    uint32_t    m_numBuffers;
    AudioBuffer m_buffer;
};

} // namespace audio
} // namespace coro
