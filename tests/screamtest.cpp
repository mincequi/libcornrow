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
#include <rtp/RtpDecoder.h>

#include <asio/steady_timer.hpp>

#define private public
#define protected public
#include "../src/audio/ScreamSource.cpp"

using namespace coro;
using namespace coro::audio;

AudioBuffer testBuffer;
volatile size_t testSize;
volatile size_t padding;


int main()
{
    core::UdpSource::Config config;
    config.port = 4010;
    config.mtu = 3000;
    core::UdpSource source(config);
    source.setReadyCallback([&](Source* const, bool ready) {
        if (ready) source.start();
    });
    rtp::RtpDecoder rtpDecoder;
    AudioDecoderFfmpeg ac3Decoder;
    AlsaSink     sink;
    audio::AudioConverter<float, int16_t> converter;
    audio::Node::link(source, rtpDecoder);
    audio::Node::link(rtpDecoder, ac3Decoder);
    audio::Node::link(ac3Decoder, converter);
    audio::Node::link(converter, sink);

    // If we start mainloop here, we have to disable it in UdpSource
    source.m_ioService.run();
}
