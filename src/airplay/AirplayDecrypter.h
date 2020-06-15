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

#include <string>
#include <openssl/aes.h>

#include <coro/audio/AudioNode.h>

namespace coro {
namespace airplay {

class AirplayDecrypter : public audio::AudioNode
{
public:
    //static constexpr std::array<core::Cap,1> inCaps() {
    static constexpr std::array<audio::AudioCap,1> inCaps() {
        return {{ audio::AudioCap { audio::AudioCodec::Alac,
                            audio::SampleRate::Rate44100,
                            audio::Channels::Stereo,
                            core::CapFlag::Encrypted } }};
    }

    //static constexpr std::array<core::Cap,1> inCaps() {
    static constexpr std::array<audio::AudioCap,1> outCaps() {
        return {{ audio::AudioCap{ audio::AudioCodec::Alac,
                    audio::SampleRate::Rate44100,
                    audio::Channels::Stereo } }};
    }

    AirplayDecrypter();

    void init(const std::string& key, const std::string& iv);

private:
    const char* name() const;

    audio::AudioConf onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    void initKey(const std::string& key);
    void initIv(const std::string& iv);

    void decrypt(const char* in, char* out, int length);

    AES_KEY m_aesKey;
    std::string m_iv;
};

} // namespace airplay
} // namespace core
