#define protected public
#include "../include/coro/audio/AudioConverter.h"
#undef protected

#include <assert.h>
#include <cstring>

using namespace coro::audio;

int main()
{
    // Create converters
    AudioConverter<int16_t, float> m_intToFloat;
    AudioConverter<float, int16_t> m_floatToInt;

    // Create buffer
    AudioBuffer buffer;
    uint16_t* data = (uint16_t*)buffer.acquire(5*sizeof(int16_t));
    int16_t intData[] = { 0, 16384, -16384, -32768, 8192, 4096, 2048, 1024, 512, 256 };
    float   floatData[] = { 0.0, 0.5, -0.5, -1.0, 0.25, 0.125 };
    for (int i = 0; i < 5; ++i) {
        *data = intData[i];
        ++data;
    }
    buffer.commit(5*sizeof(int16_t));

    // Process buffer
    m_intToFloat.process(AudioConf{ Codec::RawInt16 }, buffer);

    // Test buffer after conversion to float
    assert(buffer.size() == 5*sizeof(float));
    volatile float* fdata = (float*)buffer.data();
    for (int i = 0; i < 5; ++i) {
        assert(*fdata == floatData[i]);
        ++fdata;
    }

    // Process float buffer
    m_floatToInt.process(AudioConf{ Codec::RawFloat32 }, buffer);

    // Test buffer after conversion to int16
    assert(buffer.size() == 5*sizeof(int16_t));
    volatile int16_t* idata = (int16_t*)buffer.data();
    for (int i = 0; i < 5; ++i) {
        assert((int16_t)*idata == intData[i]);
        ++idata;
    }


    // Fill float buffer
    float* bData = (float*)buffer.acquire(5*sizeof(float));
    float  fData[] = { 0.5, -0.5, -1.0, 1.1, -1.1 };
    for (int i = 0; i < 5; ++i) {
        *bData = fData[i];
        ++bData;
    }
    buffer.commit(5*sizeof(float));

    // Process float buffer
    m_floatToInt.process(AudioConf{ Codec::RawFloat32 }, buffer);

    std::vector<int16_t> overShoots;
    // Test buffer after conversion to int16
    assert(buffer.size() == 5*sizeof(int16_t));
    idata = (int16_t*)buffer.data();
    for (int i = 0; i < 5; ++i) {
        int16_t j = *idata;
        overShoots.push_back(j);
        ++idata;
    }

    return 0;
}
