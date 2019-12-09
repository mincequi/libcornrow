#pragma once

#include <coro/audio/Node.h>

#include "TBiquad.h"

#include <deque>
#include <mutex>

namespace coro
{
namespace audio
{

class Peq : public audio::Node
{
public:
    Peq();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { Codec::RawFloat32,
                    SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { Codec::RawFloat32,
                    SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    Channels::Stereo } }};
    }

    void setVolume(float volume);

    void setFilters(const std::vector<Filter> filters);
    std::vector<Filter> filters();

private:
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

    float               m_volume = 1.0;
    std::deque<TBiquad<float, float>> m_tBiquads;

    AudioConf m_conf;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
