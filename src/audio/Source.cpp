#include "audio/Source.h"

namespace coro {
namespace audio {

Source::Source()
{
}

Source::~Source()
{
}

void Source::start()
{
    m_isStarted = true;
    // Start timer to watch
}

void Source::stop()
{
    m_isStarted = false;
    m_wantsToStart = false;
}

bool Source::isStarted() const
{
    return m_isStarted;
}

bool Source::wantsToStart() const
{
    return m_wantsToStart;
}

void Source::setWantsToStart(bool wts)
{
    m_wantsToStart = wts;
    m_mutex.lock();
    if (m_wantsToStartCallback) {
        m_wantsToStartCallback(this, wts);
    }
    m_mutex.unlock();
}

void Source::setWantsToStartCallback(WantsToStartCallback callback)
{
    m_mutex.lock();
    m_wantsToStartCallback = callback;
    m_mutex.unlock();
}

void Source::pushBuffer(const AudioConf& _conf, AudioBuffer& buffer)
{
    auto conf = _conf;
    auto next = m_next;
    while (next && (conf.codec != AudioCodec::Invalid) && buffer.size()) {
        if (!next->isBypassed()) {
            conf = next->process(conf, buffer);
            ++m_bufferCount;
        }
        next = next->next();
    }
}

} // namespace audio
} // namespace coro
