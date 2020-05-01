#pragma once

#include <coro/core/Node.h>
#include <coro/audio/AudioCaps.h>

namespace coro {
namespace rtp {

class RtpDecoder : public core::Node
{
public:
    RtpDecoder();
    ~RtpDecoder();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::AudioCodecs::Any,
                    audio::SampleRates::Any,
                    audio::ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodec::Ac3,
                    audio::SampleRate::Rate48000 | audio::SampleRate::Rate44100,
                    audio::Channels::Stereo } }};
    }

private:
    audio::AudioConf doProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    uint16_t m_lastSequenceNumber = 0;
};

} // namespace rtp
} // namespace coro
