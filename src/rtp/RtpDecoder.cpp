/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
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

#include <rtp/RtpDecoder.h>

#include <rtp/RtpTypes.h>

#include <assert.h>
#include <cstring>
#include <boost/endian/arithmetic.hpp>
#include <loguru/loguru.hpp>

namespace coro {
namespace rtp {

template class RtpDecoder<audio::AudioCodec::Ac3>;
template class RtpDecoder<audio::AudioCodec::Alac>;
template class RtpDecoder<audio::AudioCodec::Sbc>;

template<audio::AudioCodec codec>
RtpDecoder<codec>::RtpDecoder()
{
}

template<audio::AudioCodec codec>
RtpDecoder<codec>::~RtpDecoder()
{
}

template<audio::AudioCodec codec>
const char* RtpDecoder<codec>::name() const
{
    return "RtpDecoder";
}

template<audio::AudioCodec codec>
audio::AudioConf RtpDecoder<codec>::onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
{
    if (buffer.size() < 12) { // RtpHeader 12 bytes
        buffer.clear();
        LOG_F(WARNING, "Header invalid");
        return {};
    }

    coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(buffer.data());
    rtpHeader->sequenceNumber = boost::endian::big_to_native(rtpHeader->sequenceNumber);
    rtpHeader->timestamp = boost::endian::big_to_native(rtpHeader->timestamp);
    if (rtpHeader->payloadType < 96) {
        buffer.clear();
        LOG_F(WARNING, "Header invalid");
        return {};
    }

    if (m_isFlushed) {
        m_isFlushed = false;
        m_seq = rtpHeader->sequenceNumber;
        LOG_F(INFO, "Sequence starts at: %d", m_seq);
    } else if (++m_seq != rtpHeader->sequenceNumber) {
        LOG_F(WARNING, "Sequence discontinuous. %d, %d", m_seq, rtpHeader->sequenceNumber);
        m_seq = rtpHeader->sequenceNumber;
    }

    return onProcessCodec(*rtpHeader, buffer);
}

template<audio::AudioCodec codec>
void RtpDecoder<codec>::onStop()
{
    m_isFlushed = true;
}

template<>
audio::AudioConf RtpDecoder<audio::AudioCodec::Ac3>::onProcessCodec(const rtp::RtpHeader& header, audio::AudioBuffer& buffer)
{
    if (buffer.size() < header.size() + 2) { // RtpHeader 12 bytes + Ac3Header 2 bytes
        buffer.clear();
        LOG_F(WARNING, "AC3 header invalid");
        return {};
    }

    if (!header.marker || header.payloadType < 96) {
        buffer.clear();
        LOG_F(WARNING, "AC3 header invalid");
        return {};
    }

    if ((buffer.data() + header.size())[0] != 0 || (buffer.data() + header.size())[1] != 1) {
        buffer.clear();
        LOG_F(WARNING, "AC3 header invalid");
        return {};
    }

    buffer.trimFront(header.size() + 2);

    return { audio::AudioCodec::Ac3 };
}

template<>
audio::AudioConf RtpDecoder<audio::AudioCodec::Sbc>::onProcessCodec(const rtp::RtpHeader& header, audio::AudioBuffer& buffer)
{
    if (buffer.size() < header.size() + 1) { // RtpHeader 12 bytes + SbcHeader 1 byte
        LOG_F(WARNING, "SBC header invalid");
        return {};
    }

    if (!header.isValidSbc()) {
        LOG_F(WARNING, "SBC header invalid");
        return {};
    }

    coro::rtp::RtpSbcHeader* rtpSbcHeader = (coro::rtp::RtpSbcHeader*)(buffer.data() + header.size());
    if (!rtpSbcHeader->isValid()) {
        LOG_F(WARNING, "SBC header invalid");
        return {};
    }

    if (rtpSbcHeader->isFragmented) {
        LOG_F(WARNING, "Fragmented packet(s) not supported");
        return {};
    }

    //LOG_F(2, "Sequence number: %d", header.sequenceNumber);

    buffer.trimFront(header.size() + 1);

    return { audio::AudioCodec::Sbc };
}

template<audio::AudioCodec codec>
audio::AudioConf RtpDecoder<codec>::onProcessCodec(const rtp::RtpHeader& header, audio::AudioBuffer& buffer)
{
    buffer.trimFront(header.size());

    return { codec };
}

} // namespace rtp
} // namespace coro
