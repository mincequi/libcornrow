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

#include "core/AppSink.h"

namespace coro {
namespace core {

class AppSinkPrivate
{
public:
    AppSink::ProcessCallback callback = nullptr;
};

AppSink::AppSink()
    : d(new AppSinkPrivate)
{
}

AppSink::~AppSink()
{
    delete d;
}

void AppSink::setProcessCallback(ProcessCallback callback)
{
    d->callback = callback;
}

audio::AudioConf AppSink::onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
{
    if (d->callback) {
        d->callback(conf, buffer);
    }

    return {};
}

} // namespace core
} // namespace coro
