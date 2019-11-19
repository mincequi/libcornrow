#pragma once

#include <coro/audio/Node.h>

typedef struct sbc_struct sbc_t;

namespace coro {
namespace audio {

class AudioConverter : public Node
{
public:
    AudioConverter();
    virtual ~AudioConverter();

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { Codec::RawInt16 | Codec::RawFloat32 } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { Codec::RawInt16 | Codec::RawFloat32 } }};
    }

protected:
    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:

};

} // namespace audio
} // namespace coro
