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

#include <mutex>

#include "AudioNode.h"
#include "TBiquad.h"

namespace coro {
namespace audio {

class Crossover : public audio::AudioNode
{
public:
    Crossover();

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{{
                { AudioCapRaw<float> { SampleRate::Rate44100 | SampleRate::Rate48000 } }, // in
                { AudioCapRaw<float> { SampleRate::Rate44100 | SampleRate::Rate48000 } }  // out
               }}};
    }

    void setFilter(const Filter& f);
    //float frequency();

    void setLfe(bool enable);
    bool lfe();

private:
    const char* name() const override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;

    //bool isFrequencyValid() const;
    void updateCrossover();
    void updateLfe();

    template<typename InFrame, typename OutFrame>
    static void processLfe(InFrame* inFrames, OutFrame* outFrames, std::uint32_t frameCount)
    {
        for (std::uint32_t i = 0; i < frameCount; ++i) {
            outFrames[i].lfe = inFrames[i].left*M_SQRT1_2/*0.5*/ + inFrames[i].right*M_SQRT1_2/*0.5*/;
        }
    }

    Filter  m_filter;
    bool    m_lfe;

    TBiquad<float, float> m_lp;
    TBiquad<float, float> m_hp;
    TBiquad<float, float> m_lfeLp;
    TBiquad<float, float> m_lfeHp;

    float m_lowGain = 1.0f;
    float m_highGain = 1.0f;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
