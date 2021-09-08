/*
 * Copyright (C) 2021 Manuel Weichselbaumer <mincequi@web.de>
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
#include <coro/core/Sink.h>

namespace coro {
namespace core {

class TcpClientSink : public Sink {
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{
                { { AnyCap {} },    // in
                  { NoCap {} }      // out
                }
            }};
    }

    struct Config {
        std::string host;
        uint16_t port;
    };

    TcpClientSink(const Config& config);
    virtual ~TcpClientSink();

    virtual bool isStarted() const override;

private:
    virtual const char* name() const override;
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onProcess(core::BufferPtr& buffer) override;

    class TcpClientSinkPrivate* const d;
};

} // namespace core
} // namespace coro
