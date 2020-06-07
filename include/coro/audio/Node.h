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
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/AudioTypes.h>
#include <coro/core/Node.h>

namespace coro {
namespace audio {

class Node : public core::Node
{
public:
    Node();
    virtual ~Node();

    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr InCaps intersect(const InCaps& in, const OutCaps& out);

protected:
    friend class core::Node;
};

template<class InCaps, class OutCaps>
constexpr InCaps Node::intersect(const InCaps& in, const OutCaps& out)
{
    InCaps intersection;
    uint8_t index = 0;
    for (const auto& i : in) {
        for (const auto& o : out) {
            auto caps = AudioCaps::intersect(i, o);
            if (caps) {
                intersection.at(index++) = caps;
            }
        }
    }
    return intersection;
}

} // namespace audio
} // namespace coro
