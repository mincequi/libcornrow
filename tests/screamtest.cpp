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
