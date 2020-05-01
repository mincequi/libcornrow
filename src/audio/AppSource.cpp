#include "audio/AppSource.h"

namespace coro {
namespace audio {

AppSource::AppSource()
{
}

AppSource::~AppSource()
{
}

const char* AppSource::name() const
{
    return "AppSource";
}

AudioConf AppSource::doProcess(const AudioConf& _conf, AudioBuffer& buffer)
{
    auto conf = _conf;
    auto next = m_next;
    while (next && (conf.codec != AudioCodec::Invalid) && buffer.size()) {
        if (!next->isBypassed()) {
            conf = next->process(conf, buffer);
        }
        next = next->next();
    }

    return conf;
}

} // namespace audio
} // namespace coro
