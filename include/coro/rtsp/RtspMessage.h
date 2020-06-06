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

#include <map>
#include <string>

namespace coro {
namespace rtsp {

using RtspHeaders = std::map<std::string, std::string>;

class RtspMessage
{
public:
    static RtspMessage createRequest(const std::string& buffer);
    static RtspMessage createResponse(const std::string& CSeq);

    const std::string& method() const;
    std::string& header(const std::string& key);
    std::string header(const std::string& key) const;
    const std::string& body() const;

    std::string serialize() const;

private:
    RtspMessage();

    void parse(const std::string& buffer);

    std::string m_method;
    RtspHeaders m_headers;
    std::string m_body;
};

} // namespace rtsp
} // namespace coro
