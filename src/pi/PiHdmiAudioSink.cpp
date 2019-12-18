#include "pi/PiHdmiAudioSink.h"

#include <cstdint>
#include <cstring>
#include <loguru/loguru.hpp>

namespace coro {
namespace pi {

PiHdmiAudioSink::PiHdmiAudioSink()
{
}

PiHdmiAudioSink::~PiHdmiAudioSink()
{
}


audio::AudioConf PiHdmiAudioSink::process(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
{

    return conf;
}

bool PiHdmiAudioSink::open(const audio::AudioConf& conf)
{


    return true;
}


bool PiHdmiAudioSink::write(const char* samples, uint32_t bytesCount)
{


    return true;
}

} // namespace pi
} // namespace coro
