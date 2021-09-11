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

#include <string>

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;

namespace coro {
namespace audio {

template<audio::AudioCodec>
class AudioDecoderFfmpeg : public AudioNode {
public:
    AudioDecoderFfmpeg();
    ~AudioDecoderFfmpeg();

    static constexpr std::array<std::pair<core::Cap, core::Cap>, 3> caps() {
        return {{
                { { AudioCap { AudioCodec::Ac3 }}, // in
                  { AudioCap { AudioCodec::RawFloat32, // out
                               SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 } }},
                { { AudioCap { AudioCodec::Ac3 }}, // in
                  { AudioCapRaw<float> {
                               SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 } }},
                { { AudioCap { AudioCodec::Alac }}, // in
                  { AudioCap { AudioCodec::RawInt16, // out
                               SampleRate::Rate44100 } }}
            }};
    }

    //AudioCapRaw<float> rawCap = AudioCapRaw<float>{ SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000 };

    // Provide codec-specific format description
    // <format> <format specific parameters>
    void init(const std::string& data);

private:
    const char* name() const override;

    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;
    AudioConf onProcessCodec(core::Buffer& buffer);

    void updateConf();

    template<typename T>
    void interleave(const AVFrame* in, core::Buffer& out);

    std::string m_codecData;
    AudioConf m_conf;

    AVCodecContext* m_context = nullptr;
};

} // namespace audio
} // namespace coro
