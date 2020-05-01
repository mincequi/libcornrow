#pragma once

#include <coro/core/Sink.h>

struct AUDIOPLAY_STATE_T;

namespace coro {
namespace pi {

class PiHdmiAudioSink : public core::Sink
{
public:
    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::AudioCodec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Quad } }};
    }

    PiHdmiAudioSink();
    virtual ~PiHdmiAudioSink();

private:
    const char* name() const override;
    void stop() override;
    audio::AudioConf doProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    AUDIOPLAY_STATE_T*  m_handle = nullptr;
    audio::AudioConf  m_conf;
};

} // namespace pi
} // namespace coro
