#include <assert.h>
#include <cstring>
#include <iostream>

#include <coro/audio/AlsaSink.h>

#include <asio/steady_timer.hpp>

#define private public
#define protected public
#include "../src/audio/ScreamSource.cpp"

using namespace coro;
using namespace coro::audio;

AudioBuffer testBuffer;
volatile size_t testSize;
volatile size_t padding;

class TestSink : public core::Sink
{
public:
    static constexpr std::array<AudioCaps,1> inCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::Ac3 } }};
    }

    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override
    {
        //assert(buffer.size() == testData.size());
        //assert(!memcmp(testData.data(), buffer.data(), buffer.size()));

        buffer.clear();
        return conf;
    }
};

int main()
{
    ScreamSource source;
    source.setReadyCallback([&](Source* const, bool ready) {
        if (ready) source.start();
    });
    AlsaSink     sink;
    Node::link(source, sink);

    source.m_udpWorker->m_ioService.run();
}
