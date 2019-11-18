#pragma once

#include <coro/core/Sink.h>

typedef struct _snd_pcm snd_pcm_t;

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

    void start() override;
    void stop() override;

    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    snd_pcm_t* m_pcm = nullptr;
    AudioConf  m_conf;
};

} // namespace audio
} // namespace coro
