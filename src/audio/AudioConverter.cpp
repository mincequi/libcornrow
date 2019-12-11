#include "audio/AudioConverter.h"

#include "loguru/loguru.hpp"

#include <cstring>

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
    char* to = buffer.acquire(buffer.size()*2);
    char* from = buffer.data();

    // @TODO(mawe): remove memcpy, since memory alignment is now done in AudioBuffer.
    for (size_t i = 0; i < buffer.size()/size(conf.codec); ++i) {
        int16_t tmp;
        std::memcpy(&tmp, from+(i*2), 2);
        float f = tmp/32767.0;
        std::memcpy(to+(i*4), &f, 4);
    }

    buffer.commit(buffer.size()*2);

    auto _conf = conf;
    _conf.codec = AudioCodec::RawFloat32;
    return _conf;
}

template<>
AudioConf AudioConverter<float,int16_t>::process(const AudioConf& conf, AudioBuffer& buffer)
{
    char* to = buffer.acquire(buffer.size()/2);
    char* from = buffer.data();

    // @TODO(mawe): remove memcpy, since memory alignment is now done in AudioBuffer.
    for (size_t i = 0; i < buffer.size()/size(conf.codec); ++i) {
        float f;
        std::memcpy(&f, from+(i*4), 4);
        int16_t temp;

        if (f >= 1.0f) {
            temp = 32767;
        } else if (f < -1.0f) {
            temp = -32768;
        } else {
            temp = (int16_t)(f*32767.0f);
        }

        std::memcpy(to+(i*2), &temp, 2);
    }

    buffer.commit(buffer.size()/2);

    auto _conf = conf;
    _conf.codec = AudioCodec::RawInt16;
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

