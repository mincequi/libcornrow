#pragma once

#include <coro/core/Sink.h>

#include <string>

typedef struct _snd_pcm snd_pcm_t;

namespace coro {
namespace audio {

class AlsaSink : public core::Sink
{
public:
    static constexpr std::array<AudioCap,1> inCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::Ac3 } }};
    }

    AlsaSink();
    virtual ~AlsaSink();

    void start(const AudioConf& conf);
    void stop() override;

    void setDevice(const std::string& device);

    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    // alsa members
    bool open(const AudioConf& conf);
    bool setHwParams(const AudioConf& conf);
    bool setSwParams();
    bool write(const char* samples, uint32_t bytesCount);
    bool recover(int err);
    //
    static void doAc3Payload(AudioBuffer& buffer);

    snd_pcm_t* m_pcm = nullptr;
    AudioConf  m_conf;

    std::string m_device = "default";
};

} // namespace audio
} // namespace coro
