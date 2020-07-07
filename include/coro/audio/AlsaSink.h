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

#include <coro/core/Sink.h>

#include <string>

typedef struct _snd_pcm snd_pcm_t;

namespace coro {
namespace audio {

class AlsaSink : public core::Sink
{
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{{ { AudioCapRaw<int16_t> {} },
                   { core::NoCap {} }
               }}};
    }

    AlsaSink();
    virtual ~AlsaSink();

    void start(const AudioConf& conf);

    void setDevice(const std::string& device);

private:
    const char* name() const override;
    void onStart() override;
    void onStop() override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;

    // alsa members
    bool open(const AudioConf& conf);
    bool openSimple(const AudioConf& conf);
    bool write(const char* samples, uint32_t bytesCount);
    void writeSimple(const char* samples, uint32_t bytesCount);
    bool recover(int err);

    snd_pcm_t* m_pcm = nullptr;
    AudioConf  m_conf;

    std::string m_device = "default";
};

} // namespace audio
} // namespace coro
