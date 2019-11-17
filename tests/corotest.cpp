#include <coro/audio/AudioBuffer.h>

#include <assert.h>
#include <cstring>
#include <iostream>

using namespace coro::audio;

int main()
{
    AudioBuffer buffer;
    auto data = buffer.acquire(512);
    for (uint16_t i = 0; i <= 255; ++i) {
        *(data+i) = i;
    }
    buffer.commit(256);
    assert(buffer.size() == 256);

    auto splitted = buffer.split(1);
    assert(splitted.size() == 256);

    uint16_t i = 0;
    for (AudioBuffer& b : splitted) {
        assert(b.size() == 1);
        assert(*b.data() == i);
        ++i;
    }
}
