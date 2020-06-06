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

#include <coro/rtsp/RtspMessage.h>

#include <regex>
#include <loguru/loguru.hpp>

namespace coro {
namespace rtsp {

RtspMessage RtspMessage::createRequest(const std::string& buffer)
{
    RtspMessage message;
    message.parse(buffer);

    return message;
}

RtspMessage RtspMessage::createResponse(const std::string& CSeq)
{
    RtspMessage message;
    message.m_headers["CSeq"] = CSeq;

    return message;
}

RtspMessage::RtspMessage()
{
}

const std::string& RtspMessage::method() const
{
    return m_method;
}

std::string& RtspMessage::header(const std::string& key)
{
    return m_headers[key];
}

std::string RtspMessage::header(const std::string& key) const
{
    if (m_headers.count(key)) {
        return m_headers.at(key);
    }

    return {};
}

const std::string& RtspMessage::body() const
{
    return m_body;
}

std::string RtspMessage::serialize() const
{
    std::stringstream ss;
    ss << "RTSP/1.0 200 OK";
    for (const auto& kv : m_headers) {
        ss << "\r\n" << kv.first << ": " << kv.second;
    }
    ss << "\r\nAudio-Jack-Status: connected; type=analog\r\n\r\n";

    return ss.str();
}

void RtspMessage::parse(const std::string& buffer)
{
    m_method.clear();
    m_headers.clear();
    m_body.clear();

    std::string s(buffer);
    std::regex rx("(\\w+) (\\S+) (\\S+)");
    std::smatch match;
    if (std::regex_search(s, match, rx) && match.size() == 4) {
        m_method = match[1].str();
    }

    rx.assign("(?:\\r\\n(\\S+)\\: ([\\S ]+))");
    while (std::regex_search(s, match, rx) && match.size() == 3) {
        m_headers[match[1].str()] = match[2].str();
        s = match.suffix().str();
    }
}

} // namespace rtsp
} // namespace coro
