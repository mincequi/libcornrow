#pragma once

#include <coro/core/Sink.h>

namespace coro {
namespace pi {

class PiHdmiAudioSink : public core::Sink
{
public:
    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::AudioCodec::RawInt16 } }};
    }

    PiHdmiAudioSink();
    virtual ~PiHdmiAudioSink();

    void start(const audio::AudioConf& conf);
    void stop() override;

    void setDevice(const std::string& device);

    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

private:
    bool open(const audio::AudioConf& conf);
    bool write(const char* samples, uint32_t bytesCount);

    audio::AudioConf  m_conf;
};

} // namespace pi
} // namespace coro
