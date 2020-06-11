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

#include <array>
#include <variant>

namespace coro {
namespace core {

class AnyCap
{
};

class Cap : public std::variant<audio::AudioCap, core::AnyCap>
{
public:
    template<class OutCaps, class InCaps>
    static constexpr bool canIntersect(const OutCaps& outCaps, const InCaps& inCaps)
    {
        for (const auto& outCap : outCaps) {
            for (const auto& inCap : inCaps) {
                if (std::holds_alternative<audio::AudioCap>(outCap) && std::holds_alternative<audio::AudioCap>(inCap)) {
                    if (audio::AudioCap::intersect(std::get<audio::AudioCap>(outCap), std::get<audio::AudioCap>(inCap)).isValid()) {
                        return true;
                    }
                } else if (std::holds_alternative<AnyCap>(outCap) || std::holds_alternative<AnyCap>(inCap)) {
                        return true;
                }
            }
        }
        return false;
    }
};

} // namespace core
} // namespace coro
