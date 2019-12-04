#pragma once

#include <coro/core/Sink.h>

typedef struct _snd_pcm snd_pcm_t;
struct ao_device;
struct ao_option;

namespace coro {
namespace audio {

class AlsaSink : public core::Sink
{
public:
    static constexpr std::array<AudioCaps,1> inCaps() {
        return {{ { Codec::RawInt16 | Codec::Ac3 } }};
    }

    AlsaSink();
    virtual ~AlsaSink();

    void start(const AudioConf& conf);
    void stop() override;

    void setDevice(const std::string& device);

    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    // alsa members
    bool open(const AudioConf& conf);
    void close();
    bool setHwParams(const AudioConf& conf);
    bool setSwParams();
    bool write(const char* samples, uint32_t bytesCount);
    bool recover(int err);
    //
    void doAc3Payload(AudioBuffer& buffer);

    snd_pcm_t* m_pcm = nullptr;
    AudioConf  m_conf;

    std::string m_device = "default";

    int         m_driverId;
    ao_device   *m_aoDevice = nullptr;
    ao_option   *m_aoOptions = nullptr;
};

} // namespace audio
} // namespace coro
