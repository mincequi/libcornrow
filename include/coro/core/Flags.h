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
#include <limits>
#include <type_traits>

namespace coro {
namespace core {

class Flag
{
    int i;
public:
    constexpr inline Flag(int value) noexcept : i(value) {}
    constexpr inline operator int() const noexcept { return i; }
};

template<typename Enum>
class Flags
{
    static_assert(std::is_enum<Enum>::value, "Flags is only useable on enum types.");

    struct Private;
    typedef int (Private::*Zero);

public:
    // Declare type, depending on signedness
    typedef typename std::conditional<std::is_unsigned<Enum>::value, unsigned int, int>::type Int;

    typedef Enum enum_type;

    constexpr inline Flags(Enum f) : i(Int(f)) {}
    constexpr inline Flags(Zero = 0) : i(0) {}
    constexpr inline Flags(Flag f) : i(f) {}
    constexpr inline Flags(Int f) : i(f) {}
    constexpr static Int Any = Flags(std::numeric_limits<Int>::max());

    inline Flags &operator&=(int mask) { i &= mask; return *this; }
    inline Flags &operator&=(unsigned int mask) { i &= mask; return *this; }
    inline Flags &operator&=(Enum mask) { i &= Int(mask); return *this; }
    inline Flags &operator|=(Flags f) { i |= f.i; return *this; }
    inline Flags &operator|=(Enum f) { i |= Int(f); return *this; }
    inline Flags &operator^=(Flags f) { i ^= f.i; return *this; }
    inline Flags &operator^=(Enum f) { i ^= Int(f); return *this; }

    constexpr inline operator Int() const { return i; }

    constexpr inline Flags operator|(Flags f) const { return Flags(Flag(i | f.i)); }
    constexpr inline Flags operator|(Enum f) const { return Flags(Flag(i | Int(f))); }
    constexpr inline Flags operator^(Flags f) const { return Flags(Flag(i ^ f.i)); }
    constexpr inline Flags operator^(Enum f) const { return Flags(Flag(i ^ Int(f))); }
    constexpr inline Flags operator&(int mask) const { return Flags(Flag(i & mask)); }
    constexpr inline Flags operator&(unsigned int mask) const { return Flags(Flag(i & mask)); }
    constexpr inline Flags operator&(Enum f) const { return Flags(Flag(i & Int(f))); }
    constexpr inline Flags operator~() const { return Flags(Flag(~i)); }

    constexpr inline bool operator!() const { return !i; }

    constexpr inline bool testFlag(Enum f) const { return (i & Int(f)) == Int(f) && (Int(f) != 0 || i == Int(f) ); }

private:
    Int i;
};

} // namespace core
} // namespace coro

#define DECLARE_OPERATORS_FOR_FLAGS(Enum) \
constexpr inline coro::core::Flags<Enum::enum_type> operator|(Enum::enum_type f1, Enum::enum_type f2) \
{ return coro::core::Flags<Enum::enum_type>(f1) | f2; } \
constexpr inline coro::core::Flags<Enum::enum_type> operator|(Enum::enum_type f1, coro::core::Flags<Enum::enum_type> f2) \
{ return f2 | f1; }


