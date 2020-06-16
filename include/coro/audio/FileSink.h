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

#include <fstream>
#include <coro/core/Sink.h>

namespace coro {
namespace audio {

class FileSink : public core::Sink
{
public:
    static constexpr std::array<AudioCap,1> inCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::RawFloat32 | AudioCodec::Ac3 } }};
    }

    FileSink();
    virtual ~FileSink();

    void setFileName(const std::string& fileName);

private:
    void onStart() override;
    void onStop() override;
    AudioConf onProcess(const AudioConf& conf, AudioBuffer& buffer) override;

    std::string m_fileName;
    std::ofstream m_file;
};

} // namespace audio
} // namespace coro
