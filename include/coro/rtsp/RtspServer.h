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

#include <cstdint>

namespace coro {
namespace rtsp {

class RtspMessageHandler;

class RtspServer
{
public:
    RtspServer(RtspMessageHandler& handler,
               uint16_t port = 0);
    virtual ~RtspServer();

    uint16_t port() const;

private:
    class RtspServerPrivate* const d;
};

} // namespace rtsp
} // namespace coro
