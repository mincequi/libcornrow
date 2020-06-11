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

#include <coro/rtsp/RtspMessageHandler.h>
#include <coro/rtsp/RtspMessage.h>

#include <loguru/loguru.hpp>

#include <functional>

using namespace std::placeholders;

namespace coro {
namespace rtsp {

class RtspMessageHandlerPrivate
{
public:
    RtspMessageHandlerPrivate()
    {
    }

    std::map<std::string, std::function<void(const RtspMessage&, RtspMessage*, uint32_t)>> methodMap;
};

RtspMessageHandler::RtspMessageHandler() :
    d(new RtspMessageHandlerPrivate)
{
    d->methodMap["OPTIONS"] = std::bind(&RtspMessageHandler::onOptions, this, _1, _2, _3);
    d->methodMap["DESCRIBE"] = std::bind(&RtspMessageHandler::onDescribe, this, _1, _2, _3);
    d->methodMap["SETUP"] = std::bind(&RtspMessageHandler::onSetup, this, _1, _2, _3);
    d->methodMap["PLAY"] = std::bind(&RtspMessageHandler::onPlay, this, _1, _2, _3);
    d->methodMap["PAUSE"] = std::bind(&RtspMessageHandler::onPause, this, _1, _2, _3);
    d->methodMap["RECORD"] = std::bind(&RtspMessageHandler::onRecord, this, _1, _2, _3);
    d->methodMap["ANNOUNCE"] = std::bind(&RtspMessageHandler::onAnnounce, this, _1, _2, _3);
    d->methodMap["TEARDOWN"] = std::bind(&RtspMessageHandler::onTeardown, this, _1, _2, _3);
    d->methodMap["GET_PARAMETER"] = std::bind(&RtspMessageHandler::onGetParameter, this, _1, _2, _3);
    d->methodMap["SET_PARAMETER"] = std::bind(&RtspMessageHandler::onSetParameter, this, _1, _2, _3);
}

RtspMessageHandler::~RtspMessageHandler()
{
    delete d;
}

void RtspMessageHandler::onMessage(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    auto it = d->methodMap.find(request.method());
    if (it == d->methodMap.end()) {
        onUnknown(request, response, ipAddress);
        return;
    }

    LOG_F(1, "method handled: %s", request.method().c_str());
    it->second(request, response, ipAddress);
}

// Standard RtspRequests
void RtspMessageHandler::onOptions(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    response->header("Public") = "OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, RECORD, "
                                 "ANNOUNCE, TEARDOWN, GET_PARAMETER, SET_PARAMETER";
}

void RtspMessageHandler::onDescribe(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onSetup(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onPlay(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onPause(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onRecord(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onAnnounce(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onTeardown(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onGetParameter(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

void RtspMessageHandler::onSetParameter(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

// Proprietary RtspRequests
void RtspMessageHandler::onFlush(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    onUnknown(request, response, ipAddress);
}

// Other RtspRequests
void RtspMessageHandler::onUnknown(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    LOG_F(WARNING, "method unknown: %s", request.method().c_str());
}

} // namespace rtsp
} // namespace coro
