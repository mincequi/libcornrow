#include "audio/SbcDecoder.h"

#include "audio/AudioBuffer.h"
#include "loguru/loguru.hpp"
#include "rtp/RtpTypes.h"

#include <sbc/sbc.h>

namespace coro {
namespace audio {

static SampleRate toCoroFrequency(int i)
{
    switch (i) {
    case SBC_FREQ_16000: return SampleRate::Rate16000;
    case SBC_FREQ_32000: return SampleRate::Rate32000;
    case SBC_FREQ_44100: return SampleRate::Rate44100;
    case SBC_FREQ_48000: return SampleRate::Rate48000;
    default: return SampleRate::Invalid;
    }
    return SampleRate::Invalid;
}

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
    auto payloadOffset = 0;

    if (conf.isRtpPayloaded) {
        coro::rtp::RtpHeader* rtpHeader = (coro::rtp::RtpHeader*)(buffer.data());
        if (!rtpHeader->isValidSbc()) {
            LOG_F(WARNING, "RTP header invalid");
            m_conf.codec = Codec::Invalid;
            goto end;
        }
        //LOG_F(INFO, "seq: %i, csrcCount: %i, extension: %i", rtpHeader->sequenceNumber, rtpHeader->csrcCount, rtpHeader->extension);

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
        payloadOffset += 1;
    }

    {
        auto res = -1;
        auto newBuffer = buffer.acquire(buffer.size()*5);
        size_t totalSize = buffer.size()-payloadOffset;
        size_t readBytes = 0;
        size_t writtenBytes = 0;
        while (totalSize > readBytes) {
            size_t writtenSize;
            res = sbc_decode(m_sbc, buffer.data()+payloadOffset+readBytes, buffer.size(),
                             newBuffer+writtenBytes, buffer.size(), &writtenSize);
            LOG_IF_F(WARNING, res == -1, "Data stream too short");
            LOG_IF_F(WARNING, res == -2, "Sync byte incorrect");
            LOG_IF_F(WARNING, res == -3, "CRC8 incorrect");
            LOG_IF_F(WARNING, res == -4, "Bitpool value out of bounds");
            if (res < 0) {
                break;
            }

            readBytes += res;
            writtenBytes += writtenSize;
        }

        buffer.commit(writtenBytes);
        if (res >= 0) {
            m_conf.channels = m_sbc->mode == SBC_MODE_MONO ? Channels::Mono : Channels::Stereo;
            m_conf.codec = Codec::RawInt16;
            m_conf.rate = toCoroFrequency(m_sbc->frequency);
            m_conf.isRtpPayloaded = false;
        }
    }
end:
    return m_conf;
}

} // namespace audio
} // namespace coro

