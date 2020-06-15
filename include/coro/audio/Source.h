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

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioNode.h>

#include <atomic>
#include <functional>
#include <mutex>

namespace coro {
namespace audio {

class AudioBuffer;
class AudioConf;

class Source : public AudioNode
{
public:
    static constexpr std::array<AudioCap,0> outCaps() { return {}; }

    Source();
    virtual ~Source();

    Source(const Source&) = delete;

    void poll();
    void start();
    void stop();
    virtual bool isStarted() const;
    virtual bool isReady() const;
    virtual void setReady(bool wts);

    using ReadyCallback = std::function<void(Source* const, bool)>;
    void setReadyCallback(ReadyCallback callback);

protected:
    void pushBuffer(const AudioConf& conf, AudioBuffer& buffer);

    virtual void doPoll();
    virtual void onStart();
    virtual void onStop();

    std::mutex  m_mutex;
    ReadyCallback m_isReadyCallback;

private:
    std::atomic_bool m_isStarted = false;
    std::atomic_bool m_isReady = false;
};

} // namespace audio
} // namespace coro
