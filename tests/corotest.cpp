#include <coro/audio/AudioBuffer.h>

#include <assert.h>
#include <cstring>
#include <iostream>

using namespace coro::audio;

int main()
{
    AudioBuffer buffer;
    auto data = buffer.acquire(65536);
    for (uint32_t i = 0; i < 32768; ++i) {
        *(uint16_t*)(data+(2*i)) = i;
    }
    buffer.commit(32768);
    assert(buffer.size() == 32768);

    auto splitted = buffer.split(256);
    assert(splitted.size() == 128);

    uint16_t i = 0;
    for (AudioBuffer& b : splitted) {
        assert(b.size() == 256);
        assert(*(uint16_t*)b.data() == i*128);
        ++i;
    }
}
