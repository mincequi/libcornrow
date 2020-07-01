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

#include <functional>

namespace coro {
namespace audio {

class AudioAppSink : public core::Sink
{
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 2> caps() {
        return {
            {{{ AudioCapRaw<int16_t> {} }, { core::NoCap {} }},
            {{ AudioCapRaw<float> {} }, { core::NoCap {} }}
        }};
    }

    AudioAppSink();
    virtual ~AudioAppSink();

    using ProcessCallback = std::function<void(const AudioConf&, const core::Buffer&)>;
    void setProcessCallback(ProcessCallback callback);

private:
    const char* name() const override;
    AudioConf onProcess(const AudioConf& conf, core::Buffer& buffer) override;

    ProcessCallback m_callback;
};

} // namespace audio
} // namespace coro
