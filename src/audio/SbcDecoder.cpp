#include "audio/SbcDecoder.h"

#include "audio/AudioBuffer.h"
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

AudioConf SbcDecoder::process(const AudioConf& conf, AudioBuffer& buffer)
{
    auto payload = buffer.data();
    auto payloadOffset = 0;

    if (conf.isRtpPayloaded) {
        coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(buffer.data());
        if (!rtpHeader->isValidSbc()) {
            LOG_F(WARNING, "RTP header invalid");
            m_conf.codec = Codec::Invalid;
            goto end;
        }
        //LOG_F(INFO, "seq: %i, csrcCount: %i, extension: %i", rtpHeader->sequenceNumber, rtpHeader->csrcCount, rtpHeader->extension);

        payload += rtpHeader->size();
        payloadOffset += rtpHeader->size();
        coro::rtp::RtpSbcHeader* rtpSbcHeader = (coro::rtp::RtpSbcHeader*)(buffer.data()+rtpHeader->size());
        if (!rtpSbcHeader->isValid()) {
            LOG_F(WARNING, "RTP SBC header invalid");
            m_conf.codec = Codec::Invalid;
            goto end;
        }
        if (rtpSbcHeader->isFragmented) {
            LOG_F(WARNING, "Fragmented packet(s) not supported");
            goto end;
        }
        payload += 1;
        payloadOffset += 1;
    }

    {
        auto newBuffer = buffer.acquire(buffer.size()*5);
        size_t newBufferSize;
        auto res = sbc_decode(m_sbc, payload, buffer.size()-payloadOffset,
                              newBuffer, buffer.size()*5, &newBufferSize);
        buffer.commit(newBufferSize);
        LOG_IF_F(WARNING, res == -1, "Data stream too short");
        LOG_IF_F(WARNING, res == -2, "Sync byte incorrect");
        LOG_IF_F(WARNING, res == -3, "CRC8 incorrect");
        LOG_IF_F(WARNING, res == -4, "Bitpool value out of bounds");
    }

end:
    return m_conf;
}

} // namespace audio
} // namespace coro

