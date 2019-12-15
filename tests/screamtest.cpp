#include <assert.h>
#include <cstring>
#include <iostream>

#include <coro/core/Sink.h>

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
    //    ScreamSource source;
    //    source.start();
    //    source.m_udpWorker->m_ioService.stop();
    //    TestSink    sink;

    //    Node::link(source, sink);

    //    assert(source.m_udpWorker->m_buffer.size() == 5+1152);

    //    auto data = source.m_udpWorker->m_buffer.data();

    for (int i = 0; i < 1000; ++i) {
        testSize = rand() % 2000 + 1;
        padding = rand() % 8;

        auto data = (unsigned char*)testBuffer.acquire(testSize);
        testBuffer.commit(testSize);
        for (size_t j = 0; j < testSize; ++j) {
            data[j] = rand() % 256;
        }

        assert(testBuffer.size() == testSize);
        assert(!memcmp(testBuffer.data(), data, testSize));
    }
}
