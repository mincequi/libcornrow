#include "audio/AudioConverter.h"

#include "loguru/loguru.hpp"

namespace coro {
namespace audio {

AudioConverter::AudioConverter()
{
}

AudioConverter::~AudioConverter()
{
}

AudioConf AudioConverter::process(const AudioConf& conf, AudioBuffer& buffer)
{
    //auto newBuffer = buffer.acquire(buffer.size()*2);
    for (size_t i = 0; i < buffer.size(); i+=size(conf.codec)) {
        //*(newBuffer+i*4) = buffer.data()
    }

    auto _conf = conf;
    _conf.codec = Codec::RawFloat32;
    return _conf;
}

} // namespace audio
} // namespace coro

