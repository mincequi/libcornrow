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
#include "sdp/Sdp.h"

#include <regex>
#include <loguru/loguru.hpp>

namespace coro {
namespace rtsp {

class RtspMessagePrivate
{
public:
    RtspMessagePrivate() {}

    std::string method;
    RtspHeaders headers;
    std::string body;

    sdp::Sdp sdp;
};

RtspMessage RtspMessage::deserializeRequest(const std::string& buffer)
{
    RtspMessage message;
    message.parse(buffer);

    std::string buf(buffer);
    std::istringstream ss(buf);
    message.d->sdp = sdp::Sdp::deserialize(ss);

    return message;
}

RtspMessage RtspMessage::createResponse(const std::string& CSeq)
{
    RtspMessage message;
    message.d->headers["CSeq"] = CSeq;

    return message;
}

RtspMessage::RtspMessage()
    : d(new RtspMessagePrivate)
{
}

RtspMessage::~RtspMessage()
{
    delete d;
}

const std::string& RtspMessage::method() const
{
    return d->method;
}

std::string& RtspMessage::header(const std::string& key)
{
    return d->headers[key];
}

std::string RtspMessage::header(const std::string& key) const
{
    if (d->headers.count(key)) {
        return d->headers.at(key);
    }

    return {};
}

const std::string& RtspMessage::body() const
{
    return d->body;
}

const sdp::Sdp& RtspMessage::sdp() const
{
    return d->sdp;
}

std::string RtspMessage::serialize() const
{
    std::stringstream ss;
    ss << "RTSP/1.0 200 OK";
    for (const auto& kv : d->headers) {
        ss << "\r\n" << kv.first << ": " << kv.second;
    }
    ss << "\r\n\r\n"; // This is important!

    return ss.str();
}

void RtspMessage::parse(const std::string& buffer)
{
    d->method.clear();
    d->headers.clear();
    d->body.clear();

    std::string s(buffer);
    std::regex rx("(\\w+) (\\S+) (\\S+)");
    std::smatch match;
    if (std::regex_search(s, match, rx) && match.size() == 4) {
        d->method = match[1].str();
    }

    rx.assign("(?:\\r\\n(\\S+)\\: ([\\S ]+))");
    while (std::regex_search(s, match, rx) && match.size() == 3) {
        d->headers[match[1].str()] = match[2].str();
        s = match.suffix().str();
    }
}

} // namespace rtsp
} // namespace coro
