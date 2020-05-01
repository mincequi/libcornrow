#pragma once

#include <coro/core/Sink.h>

#include <functional>

namespace coro {
namespace audio {

class AudioAppSink : public core::Sink
{
public:
    static constexpr std::array<AudioCaps,1> inCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::RawFloat32 } }};
    }

    AudioAppSink();
    virtual ~AudioAppSink();

    using ProcessCallback = std::function<void(const AudioConf&, const AudioBuffer&)>;
    void setProcessCallback(ProcessCallback callback);

private:
    AudioConf doProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    ProcessCallback m_callback;
};

} // namespace audio
} // namespace coro
