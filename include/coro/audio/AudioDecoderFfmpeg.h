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
class AudioDecoderFfmpeg : public AudioNode
{
public:
    AudioDecoderFfmpeg();
    ~AudioDecoderFfmpeg();

    static constexpr std::array<audio::AudioCap, 1> inCaps() {
        return {{ { AudioCodec::Ac3
                            //| AudioCodec::Eac3
                            | AudioCodec::Alac,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    static constexpr std::array<audio::AudioCap, 1> outCaps() {
        return {{ { AudioCodec::RawInt16 |AudioCodec::RawFloat32,
                    SampleRate::Rate32000 | SampleRate::Rate44100 | SampleRate::Rate48000,
                    ChannelFlags::Any } }};
    }

    // Provide codec-specific format description
    // <format> <format specific parameters>
    void init(const std::string& data);

private:
    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;
    AudioConf onProcessCodec(AudioBuffer& buffer);

    void updateConf();

    std::string m_codecData;
    AudioConf m_conf;

    AVCodecContext* m_context = nullptr;
};

} // namespace audio
} // namespace coro
