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
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{{ { AnyCap {} }, // in
                   { NoCap {} } // out
               }}};
    }

    AppSink();
    ~AppSink();

	using StartCallback = std::function<void()>;
	void setStartCallback(StartCallback callback);

    using StopCallback = std::function<void()>;
    void setStopCallback(StopCallback callback);

	using ProcessCallback = std::function<void(core::BufferPtr& buffer)>;
	void setProcessCallback(ProcessCallback callback);

private:
    virtual const char* name() const override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onProcess(core::BufferPtr& buffer) override;

    class AppSinkPrivate* const d;
};

} // namespace core
} // namespace coro
