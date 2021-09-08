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

// @TODO(mawe): Temporary use audio types
#include <coro/audio/AudioConf.h>
#include <coro/core/Buffer.h>
#include <coro/core/Caps.h>

namespace coro {
namespace core {

class Node {
public:
    template<class Node1, class Node2>
    static std::enable_if_t<Cap::canIntersect(Node1::caps(), Node2::caps())>
    link(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    /// Return name of node
    virtual const char* name() const = 0;

    /// Return next node
    Node* next() const;

    /// Obsolete process method
    audio::AudioConf process(const audio::AudioConf& conf, core::Buffer& buffer);

    /**
     * @brief isBypassed
     * @return Whether node is bypassed
     */
    bool isBypassed() const;

    /**
     * @brief setIsBypassed
     * @param isBypassed
     */
    void setIsBypassed(bool isBypassed);

    /**
     * @brief isStarted
     *
     * Check if this node is started (defaults to true).
     *
     * @return Whether node is started
     */
    virtual bool isStarted() const;

protected:
    void process(core::BufferPtr& buffer);

    virtual void onStart();
    virtual void onStop();
    virtual audio::AudioConf onProcess(const audio::AudioConf& conf, core::Buffer& buffer);
    virtual void onProcess(core::BufferPtr& buffer);

private:
    Node* m_next = nullptr;
    bool m_isBypassed = false;

    friend class Source;
};

} // namespace core
} // namespace coro
