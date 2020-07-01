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

#pragma once

#include <coro/audio/AudioNode.h>

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;

namespace coro {
namespace audio {

class AudioEncoderFfmpeg : public AudioNode
{
public:
    AudioEncoderFfmpeg(AudioCodec codec);
    ~AudioEncoderFfmpeg();

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 2> caps() {
        return {{
                {{ AudioCapRaw<float> { SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 } },
                   { AudioCap { AudioCodec::Ac3 | AudioCodec::Eac3 } }},
                // We can be bypassed, so also accept inCaps as OutCaps
                { { AudioCapRaw<float> { SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 } },
                   { AudioCapRaw<float> { SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 } }}
            }};
    }

    /**
     * @brief setBitrate
     * @param kbps
     */
    void setBitrate(uint16_t kbps);

private:
    const char* name() const override;
    void onStop() override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;

    void updateConf();

    static void freeBuffer(void *opaque, uint8_t *data);

    AVFrame* createFrame() const;
    AVFrame* fillFrame(AVFrame* frame, core::Buffer& buffer);
    void     pushFrame(AVFrame* frame);

    AudioCodec m_codec = AudioCodec::Invalid;
    AudioConf m_conf;
    uint16_t m_bitrateKbps = 320;

    AVCodecContext* m_context = nullptr;
    AVFrame* m_partialFrame = nullptr;
    AVPacket* m_packet      = nullptr;
};

} // namespace audio
} // namespace coro
