#pragma once

#include <mutex>

#include "Node.h"
#include "TBiquad.h"

namespace coro
{
namespace audio
{

class Crossover : public audio::Node
{
public:
    Crossover();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::AudioCodec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Quad } }};
    }

    void setFrequency(float f);
    float frequency();

    void setLfe(bool enable);
    bool lfe();

private:
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

    bool isFrequencyValid() const;
    void updateCrossover();
    void updateLfe();

    template<typename InFrame, typename OutFrame>
    static void processLfe(InFrame* inFrames, OutFrame* outFrames, std::uint32_t frameCount)
    {
        for (std::uint32_t i = 0; i < frameCount; ++i) {
            outFrames[i].lfe = inFrames[i].left*M_SQRT1_2/*0.5*/ + inFrames[i].right*M_SQRT1_2/*0.5*/;
        }
    }

    float  m_frequency;
    bool   m_lfe;

    TBiquad<float, float> m_lp;
    TBiquad<float, float> m_hp;
    TBiquad<float, float> m_lfeLp;
    TBiquad<float, float> m_lfeHp;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
