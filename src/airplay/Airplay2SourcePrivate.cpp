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

#include "airplay/Airplay2SourcePrivate.h"

#include "airplay/AirplayDecrypter.h"
#include "airplay/AirplayRtspMessageHandler.h"

#include <audio/AudioDecoderFfmpeg.h>
#include <core/AppSink.h>
#include <core/UdpSource.h>
#include <loguru/loguru.hpp>
#include <rtp/RtpDecoder.h>
#include <rtsp/RtspServer.h>
#include <zeroconf/ZeroConfServer.h>
#include <zeroconf/ZeroConfService.h>

namespace coro {
namespace airplay {

Airplay2SourcePrivate::Airplay2SourcePrivate(Airplay2Source& _p, const AirplaySource::Config& config) :
    p(_p),
    rtspMessageHandler(controlReceiver.port(),
                       audioReceiver,
                       rtpDecoder,
                       decrypter,
                       decoder),
    rtspServer(rtspMessageHandler)
{
    std::map<std::string, std::string> txtRecords;
    txtRecords["tp"] = "UDP";
    txtRecords["sm"] = "false";
    txtRecords["ek"] = "1";
    txtRecords["et"] = "0,1";
    txtRecords["cn"] = "0,1";
    txtRecords["ch"] = "2";
    txtRecords["ss"] = "16";
    txtRecords["sr"] = "44100";
    txtRecords["vn"] = "3";
    txtRecords["txtvers"] = "1";
    txtRecords["pw"] = "false";

    zeroconfServer.registerService( { "010203040506@" + config.name,
                                      "_raop._tcp",
                                      rtspServer.port(),
                                      txtRecords } );
    core::Node::link(audioReceiver, rtpDecoder);
    core::Node::link(rtpDecoder, decrypter);
    core::Node::link(decrypter, decoder);
    core::Node::link(decoder, appSink);

    LOG_F(INFO, "Reception started. name: %s, rtsp port: %d, rtp port: %d", config.name.c_str(), rtspServer.port(), audioReceiver.port());
}

} // namespace airplay
} // namespace coro
