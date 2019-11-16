#include "audio/SbcDecoder.h"

#include "audio/Buffer.h"
#include "loguru/loguru.hpp"
#include "rtp/RtpTypes.h"

#include <sbc/sbc.h>

namespace coro {
namespace audio {

SbcDecoder::SbcDecoder()
    : m_sbc(new sbc_t)
{
    sbc_init(m_sbc, 0);
}

SbcDecoder::~SbcDecoder()
{
    sbc_finish(m_sbc);
}

Conf SbcDecoder::process(const Conf& conf, Buffer& buffer)
{
    auto payload = buffer.data();

    if (conf.isRtpPayloaded) {
        coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(payload);
        if (!rtpHeader->isValidSbc()) {
            LOG_F(WARNING, "Invalid RTP header");
            m_conf.codec = Codec::Invalid;
            goto end;
        }

        payload = buffer.data()+sizeof(rtpHeader);
    }

    auto newBuffer = buffer.acquire(buffer.size()*5);
    size_t newBufferSize;
    sbc_decode(m_sbc, payload, buffer.size(),
               newBuffer, buffer.size()*5, &newBufferSize);
    buffer.commit(newBufferSize);

end:
    return m_conf;
}

} // namespace audio
} // namespace coro

