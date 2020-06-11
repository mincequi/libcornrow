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

#include <stdint.h>

#include <coro/core/Flags.h>

namespace coro {
namespace core {

enum /*class*/ MediaType : uint8_t
{
    Audio = 0x01,
    Video = 0x02,
    Container = 0x04,
};
typedef Flags<MediaType> MediaTypes;

enum class CapFlag : uint8_t
{
    Invalid         = 0x00,
    RtpPayloaded    = 0x01,
    Encrypted       = 0x02
};
using CapFlags = core::Flags<CapFlag>;
DECLARE_OPERATORS_FOR_FLAGS(CapFlags)

} // namespace core
} // namespace coro
