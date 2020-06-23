#include <coro/pi/PiHdmiAudioSink.h>

#include <assert.h>
#include <cstring>
#include <iostream>

using namespace coro::audio;

int main()
{
    coro::core::Buffer buffer;
    coro::pi::PiHdmiAudioSink piSink;
    coro::core::Sink& sink = piSink;

    for (int i = 0; i < 10000; ++i) {
        static const size_t bufferSize = 1536;
        auto data = buffer.acquire(bufferSize);
        for (size_t j = 0; j < bufferSize; ++j) {
            data[j] = rand()%256 - 128;
        }
        buffer.commit(bufferSize);

        sink.process( { AudioCodec::RawInt16,
                        SampleRate::Rate44100,
                        Channels::Quad },
                      buffer);
    }
}
