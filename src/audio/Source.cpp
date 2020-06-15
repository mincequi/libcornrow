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
    onPoll();
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
        m_isReadyCallback(ready, this);
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
    // @TODO(mawe): currently, sources are started per default. This will change.
    if (isStarted()) {
        process(_conf, buffer);
    }
}

void Source::onPoll()
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
