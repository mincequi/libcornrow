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
        LOG_F(WARNING, "Header invalid");
        return {};
    }

    coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(buffer.data());
    rtpHeader->sequenceNumber = boost::endian::big_to_native(rtpHeader->sequenceNumber);
    rtpHeader->timestamp = boost::endian::big_to_native(rtpHeader->timestamp);
    if (!rtpHeader->marker || rtpHeader->payloadType < 96) {
        buffer.clear();
        LOG_F(WARNING, "Header invalid");
        return {};
    }
    if (m_lastSequenceNumber+1 != rtpHeader->sequenceNumber) {
        LOG_F(WARNING, "Sequence discontinuity: %d, %d", m_lastSequenceNumber, rtpHeader->sequenceNumber);
    }
    m_lastSequenceNumber = rtpHeader->sequenceNumber;

    auto payloadOffset = rtpHeader->size();
    if ((buffer.data()+payloadOffset)[0] != 0 || (buffer.data()+payloadOffset)[1] != 1) {
        buffer.clear();
        LOG_F(WARNING, "AC3 header invalid");
        return {};
    }

    payloadOffset += 2;
    buffer.trimFront(payloadOffset);

    return { audio::AudioCodec::Ac3 };
}

} // namespace rtp
} // namespace coro
