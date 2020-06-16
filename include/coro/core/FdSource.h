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

#include <coro/core/Source.h>

namespace coro {
namespace core {

class FdSource : public core::Source
{
public:
    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ { audio::AudioCodecs::Any,
                    audio::SampleRates::Any,
                    audio::ChannelFlags::Any } }};
    }

    FdSource();
    ~FdSource();

    void init(int fd, uint16_t blockSize);

private:
    const char* name() const override;

    class FdSourcePrivate* const d;
    friend class FdSourcePrivate;
};

} // namespace core
} // namespace coro
