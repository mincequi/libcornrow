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

#include <assert.h>
#include <cstring>
#include <iostream>

#include <coro/audio/AlsaSink.h>
#include <coro/audio/AudioConverter.h>
#include <coro/audio/AudioDecoderFfmpeg.h>
#include <coro/core/Mainloop.h>
#include <coro/core/UdpSource.h>
#include <coro/rtp/RtpDecoder.h>

using namespace coro;
using namespace coro::audio;
using namespace coro::core;

int main()
{
    core::UdpSource::Config config;
    config.port = 4010;
    config.mtu = 3000;
    config.multicastGroup = boost::asio::ip::address_v4::from_string("239.255.77.77").to_uint();
    core::UdpSource source(config);
    source.setReadyCallback([&](bool ready, Source* const) {
        if (ready) source.start();
    });
    rtp::RtpDecoder<AudioCodec::Ac3> rtpDecoder;
    AudioDecoderFfmpeg<AudioCodec::Ac3> ac3Decoder;
    AlsaSink     sink;
    audio::AudioConverter<float, int16_t> converter;
    audio::AudioNode::link(source, rtpDecoder);
    audio::AudioNode::link(rtpDecoder, ac3Decoder);
    audio::AudioNode::link(ac3Decoder, converter);
    audio::AudioNode::link(converter, sink);

    core::Mainloop& mainloop = core::Mainloop::instance();
    while (true) {
        usleep(1000);
        mainloop.poll();
    }
}
