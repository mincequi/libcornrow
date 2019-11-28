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
        return {{ { audio::Codec::RawFloat32 | audio::Codec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::Codec::RawFloat32 | audio::Codec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    void setVolume(float volume);

    void setFilters(const std::vector<Filter> filters);
    std::vector<Filter> filters();

private:
    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    float               m_volume = 1.0;
    std::deque<TBiquad<float, float>> m_tBiquads;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
