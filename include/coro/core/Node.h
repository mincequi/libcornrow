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

#include <memory>

namespace coro {
namespace core {

class Node
{
public:
    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<canIntersect(Node1::outCaps(), Node2::inCaps())>
    link(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect2(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<Cap::canIntersect(Node1::outCaps(), Node2::inCaps())>
    link2(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    // @TODO(mawe): make this pure virtual to enforce nodes to give a name.
    virtual const char* name() const;

    Node* next() const;

    audio::AudioConf process(const audio::AudioConf& conf, core::Buffer& buffer);

    bool isBypassed() const;
    void setIsBypassed(bool);

protected:
    virtual void onStart();
    virtual void onStop();
    virtual audio::AudioConf onProcess(const audio::AudioConf& conf, core::Buffer& buffer);
    virtual void onProcess(core::BufferPtr& buffer);

private:
    void process(core::BufferPtr& buffer);

    Node* m_next = nullptr;
    bool m_isBypassed = false;

    friend class Source;
};

template<class InCaps, class OutCaps>
constexpr bool Node::canIntersect(const InCaps& in, const OutCaps& out)
{
    for (const auto& i : in) {
        for (const auto& o : out) {
            if (audio::AudioCap::intersect(i, o).isValid()) {
                return true;
            }
        }
    }
    return false;
}

} // namespace core
} // namespace coro
