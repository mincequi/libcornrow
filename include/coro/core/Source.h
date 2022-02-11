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

#include <coro/audio/AudioConf.h>

namespace coro {
namespace core {

class Source : public core::Node
{
public:
    Source();
    virtual ~Source();

    // @TODO(mawe): make these private and let SourceSelector call these.
    /**
     * @brief Start Source
     *
     * Starts the source and begins to feed buffers to then chain. This will
     * cause the pipeline to operator in "push mode". In contrast you can also
     * start a sink wihtin your pipeline, which will cause to operate in
     * "pull mode".
     */
    void start();
    void stop();

    bool isStarted() const override;
    bool isReady() const;
    void setReady(bool ready);

    using ReadyCallback = std::function<void(bool, Source* const)>;
    virtual void setReadyCallback(ReadyCallback callback);

protected:
	void pushConfig(const audio::AudioConf& config);
    void pushBuffer(core::BufferPtr& buffer);

private:
    std::mutex  m_mutex;
    ReadyCallback m_isReadyCallback;

    std::atomic_bool m_isControlled = false;
    std::atomic_bool m_isStarted = true;
    std::atomic_bool m_isReady = false;

	audio::AudioConf m_config;

    friend class SourceSelector;
};

} // namespace core
} // namespace coro
