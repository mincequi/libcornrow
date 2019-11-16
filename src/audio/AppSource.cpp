#include "audio/AppSource.h"

namespace coro {
namespace audio {

AppSource::AppSource()
{
}

AppSource::~AppSource()
{
}

AudioConf AppSource::pushBuffer(const AudioConf& _conf, AudioBuffer& buffer)
{
    auto conf = _conf;
    auto next = m_next;
    while (next && (conf.codec != Codec::Invalid)) {
        conf = next->process(conf, buffer);
        next = next->next();
    }

    return conf;
}

} // namespace audio
} // namespace coro
