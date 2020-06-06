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

class RtspMessage;

class RtspMessageHandler
{
public:
    RtspMessageHandler();
    virtual ~RtspMessageHandler();

    void onMessage(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;

protected:
    // Standard RtspRequests
    virtual void onOptions(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onDescribe(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onSetup(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onPlay(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onPause(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onRecord(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onAnnounce(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onTeardown(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onGetParameter(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;
    virtual void onSetParameter(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;

    // Proprietary RtspRequests
    virtual void onFlush(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;

    // Other RtspRequests
    virtual void onUnknown(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const;

private:
    class RtspMessageHandlerPrivate* const d;
};

} // namespace rtsp
} // namespace coro
