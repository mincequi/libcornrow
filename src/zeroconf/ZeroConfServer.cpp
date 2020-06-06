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

namespace coro {
namespace zeroconf {

#define MDNS_RECORD  "tp=UDP", "sm=false", "ek=1", "et=0,1", "cn=0,1", "ch=2", \
                "ss=16", "sr=44100", "vn=3", "txtvers=1", "pw=false"

ZeroConfServer::ZeroConfServer()
{
}

ZeroConfServer::~ZeroConfServer()
{
    unregisterServices();
}

void ZeroConfServer::unregisterServices()
{
    for (auto& kv : m_services) {
        DNSServiceRefDeallocate(kv.second);
    }
    m_services.clear();
}

bool ZeroConfServer::registerService(const ZeroConfService& service)
{
    std::string txtRecord;
    for (const auto& kv : service.txtRecords) {
        txtRecord += kv.first.size() + kv.second.size() + 1;
        txtRecord += kv.first + "=" + kv.second;
    }

    std::cerr << txtRecord;

    const char *record[] = { MDNS_RECORD, NULL };
    uint16_t length = 0;
    const char **field;

    // Concatenate string contained i record into buf.
    for (field = record; *field; ++field) {
        length += strlen(*field) + 1; // One byte for length each time
    }

    char *buf = new char[length * sizeof(char)];
    char *p = buf;

    for (field = record; *field; ++field) {
        char * newp = stpcpy(p + 1, *field);
        *p = newp - p - 1;
        p = newp;
    }

    DNSServiceRef dnssref;
    DNSServiceErrorType error;
    error = DNSServiceRegister(&dnssref,
                               0,
                               kDNSServiceInterfaceIndexAny,
                               service.name.c_str(),
                               service.type.c_str(),
                               "",
                               NULL,
                               htobe16(service.port),
                               txtRecord.size(),
                               txtRecord.c_str(),
                               //length,
                               //buf,
                               NULL,
                               NULL);
    free(buf);

    if (error != kDNSServiceErr_NoError) {
        std::cerr << "dns-sd: DNSServiceRegister error:  " << error;
        return false;
    }

    m_services[service.name] = dnssref;
    return true;
}

void ZeroConfServer::unregisterService(const std::string& name)
{
    auto it = m_services.find(name);
    if (it != m_services.end()) {
        DNSServiceRefDeallocate(it->second);
        m_services.erase(it);
    }
}

} // namespace zeroconf
} // namespace coro
