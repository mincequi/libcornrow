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

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{
                { { AudioCapRaw<InT> { } }, // in
                  { AudioCapRaw<OutT> { } }}
               }};
    }

private:
    const char* name() const override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;
};

} // namespace audio
} // namespace coro
