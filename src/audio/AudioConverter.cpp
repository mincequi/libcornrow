/*
 * Copyright (C) 2021 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio/AudioConverter.h"

#include "loguru/loguru.hpp"

#include <cstring>

namespace coro {
namespace audio {

template<class InT, class OutT>
AudioConverter<InT,OutT>::AudioConverter() {
}

template<class InT, class OutT>
AudioConverter<InT,OutT>::~AudioConverter() {
}

template<class InT, class OutT>
const char* AudioConverter<InT,OutT>::name() const {
    return "AudioConverter";
}

template<>
AudioConf AudioConverter<int16_t,float>::onProcess(const AudioConf& conf, core::Buffer& buffer) {
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
AudioConf AudioConverter<float,int16_t>::onProcess(const AudioConf& conf, core::Buffer& buffer) {
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
AudioConf AudioConverter<T,U>::onProcess(const AudioConf& conf, core::Buffer& buffer) {
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

template class coro::audio::AudioConverter<float, int16_t>;
template class coro::audio::AudioConverter<int16_t, float>;
