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

#include <coro/airplay/AirPlaySource.h>
#include <coro/core/UdpSource.h>
#include <rtsp/RtspMessageHandler.h>
#include <rtsp/RtspServer.h>
#include <zeroconf/ZeroConfServer.h>
#include <zeroconf/ZeroConfService.h>

#include "airplay/AirplayDecryptor.h"
#include "airplay/AirplayRtspMessageHandler.h"

namespace coro {
namespace airplay {

class AirPlaySourcePrivate
{
public:
    AirPlaySourcePrivate(const AirPlaySource::Config& config) :
        rtspMessageHandler(audioReceiver.port(), controlReceiver.port(), decryptor),
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

        zeroConfServer.registerService( { "010203040506@" + config.name,
                                          "_raop._tcp",
                                          rtspServer.port(),
                                          txtRecords } );
    }

    core::UdpSource audioReceiver;
    core::UdpSource controlReceiver;
    AirplayDecryptor decryptor;
    AirplayRtspMessageHandler rtspMessageHandler;
    rtsp::RtspServer rtspServer;
    zeroconf::ZeroConfServer zeroConfServer;
};

AirPlaySource::AirPlaySource(const AirPlaySource::Config& config)
    : d(new AirPlaySourcePrivate(config))
{
}

AirPlaySource::~AirPlaySource()
{
    delete d;
}

/*
AudioConf AirPlaySource::doProcess(const AudioConf&, AudioBuffer& buffer)
{
    return { audio::AudioCodec::RawInt16,
                SampleRate::Rate44100,
                audio::Channels::Stereo };
}
*/

const char* AirPlaySource::name() const
{
    return "AirPlaySource";
}

void AirPlaySource::doPoll()
{

}

} // namespace airplay
} // namespace coro
