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

#include <dns_sd.h>
#include <map>
#include <string>

namespace coro {
namespace zeroconf {

struct ZeroConfService;

class ZeroConfServer
{
public:
    ZeroConfServer();
    ~ZeroConfServer();

    bool registerService(const ZeroConfService& service);
    void unregisterService(const std::string& name);

private:
    void unregisterServices();

    std::map<std::string, DNSServiceRef>  m_services;
};

} // namespace zeroconf
} // namespace coro
