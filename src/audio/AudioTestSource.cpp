#include <coro/audio/AudioTestSource.h>

#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

AudioTestSource::AudioTestSource(const AudioConf& audioConf,
                                 uint32_t numFramesPerBuffer,
                                 uint32_t numBuffers)
    : m_conf(audioConf),
      m_numFramesPerBuffer(numFramesPerBuffer),
      m_numBuffers(numBuffers),
      m_buffer(m_numFramesPerBuffer * toInt(m_conf.channels) * size(m_conf.codec))
{
}

AudioTestSource::~AudioTestSource()
{
}

const char* AudioTestSource::name() const
{
    return "AudioTestSource";
}

void AudioTestSource::onStart()
{
    for (uint32_t i = 0; i < m_numBuffers; ++i) {

        pushBuffer(m_conf, m_buffer);

    }
}

} // namespace audio
} // namespace coro
