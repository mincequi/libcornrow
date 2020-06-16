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

#include <coro/core/Caps.h>
#include <coro/core/Node.h>

#include <functional>

namespace coro {
namespace core {

class AppSink : public Node
{
public:
    //static constexpr std::array<core::Cap,1> inCaps() {
    static constexpr std::array<audio::AudioCap,1> inCaps() {
        return {{ { } }};
    }

    AppSink();
    ~AppSink();

    using ProcessCallback = std::function<void(const audio::AudioConf&, audio::AudioBuffer& buffer)>;
    void setProcessCallback(ProcessCallback callback);

    using FlushCallback = std::function<void()>;
    void setFlushCallback(FlushCallback callback);

private:
    virtual audio::AudioConf onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;
    virtual void onStop() override;

    class AppSinkPrivate* const d;
};

} // namespace core
} // namespace coro
