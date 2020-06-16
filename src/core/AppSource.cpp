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

#include "core/AppSource.h"

namespace coro {
namespace core {

AppSource::AppSource()
{
}

AppSource::~AppSource()
{
}

const char* AppSource::name() const
{
    return "AppSource";
}

audio::AudioConf AppSource::onProcess(const audio::AudioConf& _conf, audio::AudioBuffer& buffer)
{
    auto conf = _conf;
    auto _next = next();
    while (_next && (conf.codec != audio::AudioCodec::Invalid) && buffer.size()) {
        if (!_next->isBypassed()) {
            conf = _next->process(conf, buffer);
        }
        _next = _next->next();
    }

    return conf;
}

} // namespace core
} // namespace coro
