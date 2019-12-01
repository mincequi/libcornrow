#include "audio/AudioConverter.h"

#include "loguru/loguru.hpp"

namespace coro {
namespace audio {

template class AudioConverter<int16_t, float>;
template class AudioConverter<float, int16_t>;

template<class InT, class OutT>
AudioConverter<InT,OutT>::AudioConverter()
{
}

template<class InT, class OutT>
AudioConverter<InT,OutT>::~AudioConverter()
{
}

template<>
AudioConf AudioConverter<int16_t,float>::process(const AudioConf& conf, AudioBuffer& buffer)
{
    float* to = (float*)buffer.acquire(buffer.size()*2);
    //int16_t* from = (int16_t*)buffer.data();
    uint8_t* from = (uint8_t*)buffer.data();

    for (size_t i = 0; i < buffer.size()/size(conf.codec); ++i) {
        //int16_t tmp = *((int16_t*)(from));
        //*to = *from/32767.0;
        int16_t tmp = *((int16_t*)(from));
        *to = tmp/32767.0;
        ++to;
        from += size(conf.codec);
    }

    buffer.commit(buffer.size()*2);

    auto _conf = conf;
    _conf.codec = Codec::RawFloat32;
    return _conf;
}

template<>
AudioConf AudioConverter<float,int16_t>::process(const AudioConf& conf, AudioBuffer& buffer)
{
    int16_t* to = (int16_t*)buffer.acquire(buffer.size()/2);
    float* from = (float*)buffer.data();

    for (size_t i = 0; i < buffer.size()/size(conf.codec); ++i) {
        if (*from >= 1.0f) {
            *to = 32767;
        } else if (*from < -1.0f) {
            *to = -32768;
        } else {
            *to = (int16_t)((*from)*32767.0f);;
        }

        ++to;
        ++from;
    }
    buffer.commit(buffer.size()/2);

    auto _conf = conf;
    _conf.codec = Codec::RawInt16;
    return _conf;
}

template <typename T, typename U>
AudioConf AudioConverter<T,U>::process(const AudioConf& conf, AudioBuffer& buffer)
{
    U* to = (U*)buffer.acquire(buffer.size()*sizeof(U)/sizeof(T));
    T* from = (T*)buffer.data();

    for (size_t i = 0; i < buffer.size()/sizeof(T); ++i) {
        *to = (U)(*from);
        ++to;
        ++from;
    }

    buffer.commit(buffer.size()*sizeof(U)/sizeof(T));

    return conf;
}

} // namespace audio
} // namespace coro

