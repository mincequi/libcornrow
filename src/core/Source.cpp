/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <coro/core/Source.h>

namespace coro {
namespace core {

Source::Source()
{
}

Source::~Source()
{
}

void Source::start()
{
    m_isStarted = true;
    onStart();
}

void Source::stop()
{
    m_isStarted = false;

    Node* _next = this;
    while (_next) {
        if (!_next->isBypassed()) {
            _next->onStop();
        }
        _next = _next->next();
    }

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

void Source::pushBuffer(const audio::AudioConf& _conf, audio::AudioBuffer& buffer)
{
    // If source wants to push buffers, we consider it ready.
    //if (!isReady()) {
        setReady(true);
    //}

    // @TODO(mawe): currently, sources are started per default. This will change.
    if (isStarted()) {
        process(_conf, buffer);
    }
}

} // namespace core
} // namespace coro
