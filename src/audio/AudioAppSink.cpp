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

#include "audio/AudioAppSink.h"

#include <cstdint>
#include <cstring>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

AudioAppSink::AudioAppSink()
{
}

AudioAppSink::~AudioAppSink()
{
}

void AudioAppSink::setProcessCallback(ProcessCallback callback)
{
    m_callback = callback;
}

AudioConf AudioAppSink::doProcess(const AudioConf& conf, AudioBuffer& buffer)
{
    if (m_callback) {
        m_callback(conf, buffer);
    }

    return conf;
}

} // namespace audio
} // namespace coro
