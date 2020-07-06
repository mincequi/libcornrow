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

#include "airplay/AirplaySource.h"

#include "airplay/AirplayDecrypter.h"
#include "airplay/AirplayRtspMessageHandler.h"

#include <audio/AudioDecoderFfmpeg.h>
#include <core/AppSink.h>
#include <core/UdpSource.h>
#include <rtp/RtpDecoder.h>
#include <rtsp/RtspServer.h>
#include <zeroconf/ZeroConfServer.h>
#include <zeroconf/ZeroConfService.h>

namespace coro {
namespace airplay {

class Airplay2Source;

class Airplay2SourcePrivate
{
public:
    Airplay2SourcePrivate(Airplay2Source& _p, const AirplaySource::Config& config);

    Airplay2Source& p;

    core::UdpSource audioReceiver;
    core::UdpSource controlReceiver;

    rtp::RtpDecoder<audio::AudioCodec::Alac> rtpDecoder;
    AirplayDecrypter decrypter;
    audio::AudioDecoderFfmpeg<audio::AudioCodec::Alac> decoder;
    core::AppSink   appSink;

    AirplayRtspMessageHandler rtspMessageHandler;
    rtsp::RtspServer rtspServer;
    zeroconf::ZeroconfServer zeroconfServer;
};

} // namespace airplay
} // namespace coro
