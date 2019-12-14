#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/Source.h>

namespace coro {
namespace audio {

class UdpWorker;

class ScreamSource : public Source
{
public:
    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    ScreamSource();
    virtual ~ScreamSource();

    std::string name() const override;

private:
    UdpWorker* m_udpWorker;
};

} // namespace audio
} // namespace coro
