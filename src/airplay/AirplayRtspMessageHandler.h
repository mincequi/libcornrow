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

#include <coro/rtsp/RtspMessage.h>
#include <coro/rtsp/RtspMessageHandler.h>

#include <array>
#include <cstddef>

namespace coro {
namespace airplay {

class AirplayRtspMessageHandler : public rtsp::RtspMessageHandler
{
public:
    AirplayRtspMessageHandler(uint16_t rtpReceiverPort);

private:
    void onOptions(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const override;
    void onAnnounce(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const override;
    void onSetup(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const override;

    void onAppleChallenge(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const;

    uint16_t m_rtpReceiverPort = 0;
};

} // namespace airplay
} // namespace coro
