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

#include <coro/core/Mainloop.h>

#include "MainloopPrivate.h"

namespace coro {
namespace core {

Mainloop::Mainloop() :
    d(MainloopPrivate::instance())
{
}

Mainloop::~Mainloop()
{
}

Mainloop& Mainloop::instance()
{
    static Mainloop mainloop;
    return mainloop;
}

void Mainloop::poll()
{
    // This has to be called, if io_context ran out of work.
    //d.ioContext.restart();
    // Better poll one event here (instead of all).
    // Due to network buffering, this behaves better.
    d.ioContext.poll_one();
}

void Mainloop::run()
{
    d.ioContext.run();
}

} // namespace core
} // namespace coro
