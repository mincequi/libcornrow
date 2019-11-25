#include "audio/AudioConverter.h"

#include "loguru/loguru.hpp"

namespace coro {
namespace audio {

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
    int16_t* from = (int16_t*)buffer.data();
    for (size_t i = 0; i < buffer.size()/size(conf.codec); ++i) {
        *to = (float)*from/(float)0x8000;
        ++to;
        ++from;
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
        *to = (int16_t)(*from)*0x8000;
        ++to;
        ++from;
    }

    buffer.commit(buffer.size()/2);
    auto _conf = conf;
    _conf.codec = Codec::RawInt16;
    return _conf;
}

} // namespace audio
} // namespace coro

