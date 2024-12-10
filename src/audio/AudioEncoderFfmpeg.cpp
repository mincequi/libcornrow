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

#include "audio/AudioEncoderFfmpeg.h"

#include <assert.h>
#include <cstring>
#include <loguru/loguru.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
}

namespace coro {
namespace audio {

static AVCodecID toFfmpegCodecId(AudioCodec codec) {
    switch(codec) {
    case AudioCodec::Ac3: return AV_CODEC_ID_AC3;
    case AudioCodec::Eac3: return AV_CODEC_ID_EAC3;
//    case AudioCodec::Wav: return AV_CODEC_ID_MS;
    default: return AV_CODEC_ID_NONE;
    }
}

AudioEncoderFfmpeg::AudioEncoderFfmpeg(AudioCodec codec)
    : m_codec(codec) {
    //avcodec_register_all();
}

AudioEncoderFfmpeg::~AudioEncoderFfmpeg() {
    if (m_context) {
        // This will also free the encoder.
        avcodec_free_context(&m_context);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_partialFrame) {
        av_frame_free(&m_partialFrame);
    }
}

void AudioEncoderFfmpeg::setBitrate(uint16_t kbps) {
    m_bitrateKbps = kbps;
}

const char* AudioEncoderFfmpeg::name() const {
    return "AudioEncoderFfmpeg";
}

void AudioEncoderFfmpeg::onStop() {
    core::Buffer buffer;
    process(m_conf, buffer);
}

AudioConf AudioEncoderFfmpeg::onProcess(const AudioConf& conf, core::Buffer& _buffer) {
    if (m_conf != conf) {
        m_conf = conf;
        updateConf();
    }

    // 0. If buffer is empty, flush encoder
    if (_buffer.size() == 0) {
        if (m_partialFrame) {
            pushFrame(m_partialFrame);
            m_partialFrame = nullptr;
        }
        avcodec_send_frame(m_context, nullptr);
    }

    // 1. fill partial frame and push if it is completed now
    if (m_partialFrame) {
        if (fillFrame(m_partialFrame, _buffer)) {
            pushFrame(m_partialFrame);
            m_partialFrame = nullptr;
        } else {
            //LOG_F(INFO, "Not enough data to complete frame");
            return conf;
        }
    }

    // 2. produce frames and push until buffer is empty
    AVFrame* frame = createFrame();
    while (fillFrame(frame, _buffer)) {
        pushFrame(frame);
        frame = createFrame();
    }
    m_partialFrame = frame;

    assert(_buffer.size() == 0);

    return conf;
}

void AudioEncoderFfmpeg::updateConf() {
    if (m_context) {
        // This will also free the encoder.
        avcodec_free_context(&m_context);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_partialFrame) {
        av_frame_free(&m_partialFrame);
    }

    auto ffCodecId = toFfmpegCodecId(m_codec);
    auto encoder = avcodec_find_encoder(ffCodecId);
    m_packet = av_packet_alloc();
    m_context = avcodec_alloc_context3(encoder);
    auto channel_layout = (m_conf.channels & Channels::Stereo) ? (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT) : 0;
    channel_layout |= (m_conf.channels & Channels::RearStereo) ? (AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT) : 0;
    channel_layout |= (m_conf.channels & Channels::Center) ? (AV_CH_FRONT_CENTER) : 0;
    channel_layout |= (m_conf.channels & Channels::Lfe) ? (AV_CH_LOW_FREQUENCY) : 0;
    av_channel_layout_from_mask(&m_context->ch_layout, channel_layout);
    m_context->sample_fmt = AV_SAMPLE_FMT_FLTP; // Most codecs only support planar formats
    m_context->sample_rate = toInt(m_conf.rate);
    m_context->bit_rate = m_bitrateKbps*1000;

    int ret = avcodec_open2(m_context, encoder, NULL);
    LOG_IF_F(ERROR, ret < 0, "Error opening codec: %d", ret);
}

void AudioEncoderFfmpeg::freeBuffer(void* opaque, uint8_t*) {
    core::Buffer* buffer = static_cast<core::Buffer*>(opaque);
    if (buffer) {
        delete buffer;
    }
}

AVFrame* AudioEncoderFfmpeg::createFrame() const {
    // Prepare frame
    auto frame = av_frame_alloc();
    frame->format = m_context->sample_fmt;
    frame->ch_layout = m_context->ch_layout;
    // frame->channels = m_context->channels;
    frame->sample_rate = m_context->sample_rate;
    frame->nb_samples = 0;

    const int bytesPerFrame = m_context->frame_size * m_context->ch_layout.nb_channels * av_get_bytes_per_sample(m_context->sample_fmt);
    auto buffer = new core::Buffer(bytesPerFrame*2);
    frame->extended_data = frame->data;
    frame->extended_data[0] = (uint8_t*)buffer->acquire(bytesPerFrame);
    frame->linesize[0] = bytesPerFrame/m_context->ch_layout.nb_channels;
    for (int i = 1; i < frame->ch_layout.nb_channels; i++) {
        frame->extended_data[i] = frame->extended_data[i-1] + frame->linesize[0];
    }
    frame->buf[0] = av_buffer_create(NULL, 0, AudioEncoderFfmpeg::freeBuffer, buffer, 0);

    return frame;
}

AVFrame* AudioEncoderFfmpeg::fillFrame(AVFrame* frame, core::Buffer& buffer) {
    const int sampleSize = av_get_bytes_per_sample(m_context->sample_fmt);
    volatile const int samplesToFill = std::min(m_context->frame_size - frame->nb_samples, (int)buffer.size()/frame->ch_layout.nb_channels/sampleSize);

    switch (sampleSize) {
    case 2: {
        const uint16_t* idata = (const uint16_t*)buffer.data();
        for (int i = frame->nb_samples; i < samplesToFill+frame->nb_samples; i++) {
            for (int j = 0; j < frame->ch_layout.nb_channels; j++) {
                ((uint16_t*)frame->extended_data[j])[i] = idata[j];
            }
            idata += frame->ch_layout.nb_channels;
        }
        break;
    }
    case 4: {
        const uint32_t* idata = (const uint32_t*)buffer.data();
        for (int i = frame->nb_samples; i < samplesToFill+frame->nb_samples; i++) {
            for (int j = 0; j < frame->ch_layout.nb_channels; j++) {
                ((uint32_t*)frame->extended_data[j])[i] = idata[j];
            }
            idata += frame->ch_layout.nb_channels;
        }
        break;
    }
    default:
        return nullptr;
    }

    frame->nb_samples += samplesToFill;
    buffer.trimFront(samplesToFill*sampleSize*frame->ch_layout.nb_channels);

    if (frame->nb_samples == m_context->frame_size) {
        return frame;
    }

    return nullptr;
}

void AudioEncoderFfmpeg::pushFrame(AVFrame* frame) {
    int ret = avcodec_send_frame(m_context, frame);
    LOG_IF_F(WARNING, ret == AVERROR(EAGAIN), "No input accepted in current state");
    LOG_IF_F(WARNING, ret == AVERROR_EOF, "Encoder flushed, no new frames can be sent to it");
    LOG_IF_F(WARNING, ret == AVERROR(EINVAL), "Codec not opened, refcounted_frames not set");
    LOG_IF_F(WARNING, ret == AVERROR(ENOMEM), "Failed to add packet to internal queue");
    av_frame_free(&frame);

    ret = avcodec_receive_packet(m_context, m_packet);
    LOG_IF_F(WARNING, ret == AVERROR(EAGAIN), "No output available in current state");
    LOG_IF_F(WARNING, ret == AVERROR_EOF, "Encoder flushed, no more output packets");
    LOG_IF_F(WARNING, ret == AVERROR(EINVAL), "Codec not opened");
    // @TODO(mawe): why is this buffer local? Why 16332 bytes? Can be done dynamically?
    core::Buffer buffer(16332);
    auto newData = buffer.acquire(m_packet->size);
    std::memcpy(newData, m_packet->data, m_packet->size);
    buffer.commit(m_packet->size);
    av_packet_unref(m_packet);

    // Search for next node, that is not bypassed.
    auto _next = next();
    while (_next && _next->isBypassed()) {
        _next = _next->next();
    }

    if (!_next) {
        LOG_F(ERROR, "No further element after encoder");
        return;
    }

    auto _conf = m_conf;
    _conf.codec = AudioCodec::Ac3;
    _next->process(_conf, buffer);
}

} // namespace audio
} // namespace coro
