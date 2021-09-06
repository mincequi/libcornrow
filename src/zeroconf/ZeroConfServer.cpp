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

#include "ZeroConfServer.h"

#include "ZeroConfService.h"

#include <cstring>
#include <iostream>

#if defined(__APPLE__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

namespace coro {
namespace zeroconf {

ZeroconfServer::ZeroconfServer()
{
}

ZeroconfServer::~ZeroconfServer()
{
    unregisterServices();
}

void ZeroconfServer::unregisterServices()
{
    for (auto& kv : m_services) {
        DNSServiceRefDeallocate(kv.second);
    }
    m_services.clear();
}

bool ZeroconfServer::registerService(const ZeroConfService& service)
{
    std::string txtRecord;
    for (const auto& kv : service.txtRecords) {
        txtRecord += kv.first.size() + kv.second.size() + 1;
        txtRecord += kv.first + "=" + kv.second;
    }

    DNSServiceRef dnssref;
    DNSServiceErrorType error = DNSServiceRegister(
                &dnssref,
                0,
                kDNSServiceInterfaceIndexAny,
                service.name.c_str(),
                service.type.c_str(),
                "",
                NULL,
                htons(service.port),
                txtRecord.size(),
                txtRecord.c_str(),
                NULL,
                NULL);

    if (error != kDNSServiceErr_NoError) {
        std::cerr << "dns-sd: DNSServiceRegister error:  " << error;
        return false;
    }

    m_services[service.name] = dnssref;
    return true;
}

void ZeroconfServer::unregisterService(const std::string& name)
{
    auto it = m_services.find(name);
    if (it != m_services.end()) {
        DNSServiceRefDeallocate(it->second);
        m_services.erase(it);
    }
}

} // namespace zeroconf
} // namespace coro
