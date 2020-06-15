#include "audio/Source.h"

namespace coro {
namespace audio {

Source::Source()
{
}

Source::~Source()
{
}

void Source::poll()
{
    doPoll();
}

void Source::start()
{
    m_isStarted = true;
    onStart();
}

void Source::stop()
{
    m_isStarted = false;
    onStop();

    setReady(false);
}

bool Source::isStarted() const
{
    return m_isStarted;
}

bool Source::isReady() const
{
    return m_isReady;
}

void Source::setReady(bool ready)
{
    m_isReady = ready;
    m_mutex.lock();
    if (m_isReadyCallback) {
        m_isReadyCallback(this, ready);
    }
    m_mutex.unlock();
}

void Source::setReadyCallback(ReadyCallback callback)
{
    m_mutex.lock();
    m_isReadyCallback = callback;
    m_mutex.unlock();
}

void Source::pushBuffer(const AudioConf& _conf, AudioBuffer& buffer)
{
    auto conf = _conf;
    auto next = m_next;
    while (next && (conf.codec != AudioCodec::Invalid) && buffer.size()) {
        if (!next->isBypassed()) {
            conf = next->process(conf, buffer);
        }
        next = next->next();
    }
}

void Source::doPoll()
{
}

void Source::onStart()
{
}

void Source::onStop()
{
}

} // namespace audio
} // namespace coro
