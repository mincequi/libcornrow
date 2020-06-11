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

#include <coro/audio/ScreamSource.h>

#include <thread>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <asio/steady_timer.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

// 1st byte denotes the sampling rate.
//      Bit 7 specifies the base rate: 0 for 48kHz, 1 for 44,1kHz.
//      Other bits specify the multiplier for the base rate.
// 2nd byte denotes the sampling width, in bits.
// 3rd byte denotes the number of channels being transferred.
// 4th and 5th bytes make up the DWORD dwChannelMask from Microsofts WAVEFORMATEXTENSIBLE structure,
//      describing the mapping of channels to speaker positions.

class ScreamHeader
{
public:
    uint8_t rateMultiplier:7;
    uint8_t baseRate:1;
    uint8_t sampleSize;
    uint8_t channelCount;
    uint16_t channelMask;

    bool isValid() const
    {
        return (rateMultiplier == 1 &&
                sampleSize == 16 &&
                channelCount == 2);
    }
    size_t size() const { return 5; }
};

ScreamSource::ScreamSource()
    : core::UdpSource( { 4010,
                       3, // 3 padding,
                       5+(1152*7), // 5 header, 1152 payload, +2*payload (int to float), +2*2*payload (float to crossover
                       boost::asio::ip::address_v4::from_string("239.255.77.77").to_uint() } )
{
}

ScreamSource::~ScreamSource()
{
}

AudioConf ScreamSource::onProcess(const AudioConf&, AudioBuffer& buffer)
{
    if (buffer.size() != (5+1152)) {
        LOG_F(ERROR, "invalid bytes count: %zu", buffer.size());
        buffer.clear();
        return {};
    }

    auto header = (ScreamHeader*)(buffer.data());
    if (!header->isValid()) {
        LOG_F(ERROR, "invalid scream header: we only support 44.1/48 khz, S16, Stereo");
        buffer.clear();
        return {};
    }

    buffer.trimFront(header->size());
    return { audio::AudioCodec::RawInt16,
                header->baseRate ? SampleRate::Rate44100 : SampleRate::Rate48000,
                audio::Channels::Stereo };
}

const char* ScreamSource::name() const
{
    return "ScreamSource";
}

} // namespace audio
} // namespace coro
