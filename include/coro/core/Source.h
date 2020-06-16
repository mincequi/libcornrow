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

#pragma once

#include <coro/core/Node.h>

#include <atomic>
#include <functional>
#include <mutex>

namespace coro {
namespace audio {
class AudioBuffer;
class AudioConf;
}
namespace core {

class Source : public core::Node
{
public:
    Source();
    virtual ~Source();

    // @TODO(mawe): make these private and let SourceController call these.
    void start();
    void stop();

    bool isStarted() const;
    bool isReady() const;
    void setReady(bool wts);

    using ReadyCallback = std::function<void(bool, Source* const)>;
    void setReadyCallback(ReadyCallback callback);

protected:
    void pushBuffer(const audio::AudioConf& conf, audio::AudioBuffer& buffer);

private:
    std::mutex  m_mutex;
    ReadyCallback m_isReadyCallback;

    std::atomic_bool m_isStarted = true;
    std::atomic_bool m_isReady = false;

    friend class SourceController;
};

} // namespace core
} // namespace coro
