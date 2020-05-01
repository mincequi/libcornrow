#include "audio/AudioAppSink.h"

#include <cstdint>
#include <cstring>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

AudioAppSink::AudioAppSink()
{
}

AudioAppSink::~AudioAppSink()
{
}

void AudioAppSink::setProcessCallback(ProcessCallback callback)
{
    m_callback = callback;
}

AudioConf AudioAppSink::doProcess(const AudioConf& conf, AudioBuffer& buffer)
{
    if (m_callback) {
        m_callback(conf, buffer);
    }

    return conf;
}

} // namespace audio
} // namespace coro
