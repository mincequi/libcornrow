#pragma once

#include <coro/audio/Node.h>

typedef struct sbc_struct sbc_t;

namespace coro {
namespace audio {

class SbcDecoder : public Node
{
public:
    SbcDecoder();
    ~SbcDecoder();

    static constexpr std::array<audio::Caps,1> inCaps() {
        return {{ { Codec::Sbc | Codec::RtpPayload, SampleRates::Any, ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::Caps,1> outCaps() {
        return {{ { Codec::RawInt16,
                    SampleRate::Rate16000 | SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    Channels::Mono | Channels::Stereo } }};
    }

protected:
    Conf process(const Conf& conf, Buffer& buffer) override;

private:
    Conf m_conf;

    sbc_t* m_sbc;
};

} // namespace audio
} // namespace coro
