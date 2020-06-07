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

#include <coro/airplay/AirPlaySource.h>
#include <coro/audio/AlsaSink.h>
#include <coro/core/Mainloop.h>

#include <unistd.h>

using namespace coro;

int main()
{
    airplay::AirPlaySource::Config config { "myAirplay" };
    airplay::AirPlaySource source(config);
    audio::AlsaSink      sink;
    audio::Node::link(source, sink);

    core::Mainloop& mainloop = core::Mainloop::instance();
    while (true) {
        usleep(1000);
        mainloop.poll();
    }
}
