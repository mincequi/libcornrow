#include <rtp/RtpDecoder.h>

#include <rtp/RtpTypes.h>

#include <assert.h>
#include <cstring>
#include <boost/endian/arithmetic.hpp>
#include <loguru/loguru.hpp>

namespace coro {
namespace rtp {

RtpDecoder::RtpDecoder()
{
}

RtpDecoder::~RtpDecoder()
{
}

audio::AudioConf RtpDecoder::doProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
{
    if (buffer.size() < 14) { // RtpHeader 12 bytes + Ac3Header 2 bytes
        buffer.clear();
        LOG_F(WARNING, "RTP header invalid");
        return {};
    }

    coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(buffer.data());
    rtpHeader->sequenceNumber = boost::endian::big_to_native(rtpHeader->sequenceNumber);
    rtpHeader->timestamp = boost::endian::big_to_native(rtpHeader->timestamp);
    if (!rtpHeader->marker || rtpHeader->payloadType < 96) {
        buffer.clear();
        LOG_F(WARNING, "RTP header invalid");
        return {};
    }
    if (m_lastSequenceNumber+1 != rtpHeader->sequenceNumber) {
        LOG_F(WARNING, "RTP sequence discontinuity: %d, %d", m_lastSequenceNumber, rtpHeader->sequenceNumber);
    }
    m_lastSequenceNumber = rtpHeader->sequenceNumber;

    auto payloadOffset = rtpHeader->size();
    if ((buffer.data()+payloadOffset)[0] != 0 || (buffer.data()+payloadOffset)[1] != 1) {
        buffer.clear();
        LOG_F(WARNING, "RTP AC3 header invalid");
        return {};
    }

    payloadOffset += 2;
    buffer.trimFront(payloadOffset);

    return { audio::AudioCodec::Ac3 };
}

} // namespace rtp
} // namespace coro
