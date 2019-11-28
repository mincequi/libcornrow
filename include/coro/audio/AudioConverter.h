#pragma once

#include <coro/audio/Node.h>

namespace coro {
namespace audio {

template <class InT, class OutT>
class AudioConverter : public Node
{
public:
    AudioConverter();
    virtual ~AudioConverter();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        if (std::is_same<InT, int16_t>::value) {
            return {{ { Codec::RawInt16 } }};
        } else if (std::is_same<InT, float>::value) {
            return {{ { Codec::RawFloat32 } }};
        }
        return {{ { } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        if (std::is_same<OutT, int16_t>::value) {
            return {{ { Codec::RawInt16 } }};
        } else if (std::is_same<OutT, float>::value) {
            return {{ { Codec::RawFloat32 } }};
        }
        return {{ { } }};
    }

protected:
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:

};

} // namespace audio
} // namespace coro
