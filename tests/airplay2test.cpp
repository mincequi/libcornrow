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

#include <coro/airplay/Airplay2Source.h>
#include <coro/audio/AlsaSink.h>

#include "../src/airplay/AirplayDecrypter.h"

#include <unistd.h>

using namespace coro;

#include <array>
#include <iostream>
#include <variant>
#include <tgmath.h>

using namespace std;

using Caps1 = coro::audio::AudioCap;
using Caps2 = coro::core::AnyCap;

class Caps : public variant<Caps1, Caps2>
{
public:
    template<class OutCaps, class InCaps>
    static constexpr bool canIntersect(const OutCaps& outCaps, const InCaps& inCaps)
    {
        for (const auto& outCap : outCaps) {
            for (const auto& inCap : inCaps) {
                if (holds_alternative<Caps1>(outCap) && holds_alternative<Caps1>(inCap)) {
                    if (Caps1::intersect(std::get<Caps1>(outCap),
                                         std::get<Caps1>(inCap)).isValid()) {
                        return true;
                    }
                } /*else if (holds_alternative<Caps2>(outCap) && holds_alternative<Caps2>(inCap)) {
                    if (Caps2::intersect(std::get<Caps2>(outCap),
                                         std::get<Caps2>(inCap)).isValid()) {
                        return true;
                    }
                }*/
            }
        }
        return false;
    }
};

class Node
{
public:
    template<class T1, class T2>
    enable_if_t<Caps::canIntersect(T1::outCaps(), T2::inCaps())>
    static link(T1& prev, T2& next)
    {
        prev.m_next = &next;
    }

private:
    Node* m_next = nullptr;
};

int main()
{
    airplay::AirPlay2Source source;
    audio::AlsaSink      sink;
    audio::AudioNode::link(source, sink);

    while (true) {
        usleep(1000);
        source.poll();
    }
}
