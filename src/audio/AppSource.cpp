#include "audio/AppSource.h"

namespace coro {
namespace audio {

void AppSource::pushBuffer(const Conf& _conf, Buffer& buffer)
{
    auto conf = _conf;
    auto next = m_next;
    while (next && (conf.codec != Codec::Invalid)) {
        conf = next->process(conf, buffer);
        next = next->next();
    }
}

} // namespace audio
} // namespace coro
