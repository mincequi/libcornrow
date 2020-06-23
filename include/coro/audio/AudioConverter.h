#pragma once

#include <coro/audio/AudioNode.h>

namespace coro {
namespace audio {

template <class InT, class OutT>
class AudioConverter : public AudioNode
{
public:
    AudioConverter();
    virtual ~AudioConverter();

    static constexpr std::array<audio::AudioCap,1> inCaps() {
        if (std::is_same<InT, int16_t>::value) {
            return {{ { AudioCodec::RawInt16 } }};
        } else if (std::is_same<InT, float>::value) {
            return {{ { AudioCodec::RawFloat32 } }};
        }
        return {{ { } }};
    }

    static constexpr std::array<audio::AudioCap,1> outCaps() {
        if (std::is_same<OutT, int16_t>::value) {
            return {{ { AudioCodec::RawInt16 } }};
        } else if (std::is_same<OutT, float>::value) {
            return {{ { AudioCodec::RawFloat32 } }};
        }
        return {{ { } }};
    }

private:
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;
};

} // namespace audio
} // namespace coro
