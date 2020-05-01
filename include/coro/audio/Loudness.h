#pragma once

#include <coro/audio/Node.h>

#include "TBiquad.h"

#include <mutex>

namespace coro
{
namespace audio
{

class Loudness : public audio::Node

{
public:
    Loudness();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::AudioCodec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    void setLevel(uint8_t phon);

    void setVolume(float volume);

private:
    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    float   m_headroom = 1.0;
    float   m_volume = 1.0;

    TBiquad<float, float>   m_tpk1;
    TBiquad<float, float>   m_tpk2;
    TBiquad<float, float>   m_ths;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
