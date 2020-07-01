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

class AnyCap {
};

class NoCap {
};

using AudioFloatCap = audio::AudioCapRaw<float>;

// @TODO(mawe): not sure, if creating a variant is the right thing here.
class Cap : public std::variant<core::AnyCap, core::NoCap, audio::AudioCap, audio::AudioCapRaw<float>, audio::AudioCapRaw<int16_t>>
{
public:
    template<class OutCaps, class InCaps>
    static constexpr bool canIntersect(const OutCaps& outCaps, const InCaps& inCaps)
    {
        for (const auto& outCap : outCaps) {
            for (const auto& inCap : inCaps) {
                if (std::holds_alternative<AnyCap>(outCap.second) || std::holds_alternative<AnyCap>(inCap.first)) {
                    return true;
                } else if (std::holds_alternative<audio::AudioCap>(outCap.second) && std::holds_alternative<audio::AudioCap>(inCap.first)) {
                    if (audio::AudioCap::intersect(std::get<audio::AudioCap>(outCap.second), std::get<audio::AudioCap>(inCap.first)).isValid()) {
                        return true;
                    }
                } else if (std::holds_alternative<audio::AudioCapRaw<float>>(outCap.second) && std::holds_alternative<audio::AudioCapRaw<float>>(inCap.first)) {
                    if (audio::AudioCapRaw<float>::intersect(std::get<audio::AudioCapRaw<float>>(outCap.second), std::get<audio::AudioCapRaw<float>>(inCap.first)).isValid()) {
                        return true;
                    }
                } else if (std::holds_alternative<audio::AudioCapRaw<int16_t>>(outCap.second) && std::holds_alternative<audio::AudioCapRaw<int16_t>>(inCap.first)) {
                    if (audio::AudioCapRaw<int16_t>::intersect(std::get<audio::AudioCapRaw<int16_t>>(outCap.second), std::get<audio::AudioCapRaw<int16_t>>(inCap.first)).isValid()) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

} // namespace core
} // namespace coro
