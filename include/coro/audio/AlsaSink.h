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
        return {{ { Codec::RawInt16 } }};
    }

    AlsaSink();
    virtual ~AlsaSink();

    void start(const AudioConf& conf);
    void stop() override;

    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    snd_pcm_t* m_pcm = nullptr;
    AudioConf  m_conf;

    int         m_driverId;
    ao_device   *m_aoDevice = nullptr;
    ao_option   *m_aoOptions = nullptr;
};

} // namespace audio
} // namespace coro
