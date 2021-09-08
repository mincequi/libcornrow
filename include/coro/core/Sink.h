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

#include <coro/core/Node.h>

namespace coro {
namespace core {

class Sink : public Node {
public:
    Sink();
    virtual ~Sink();

    /**
     * @brief Start Sink
     *
     * Starts the sink and begins to pull buffers from the chain. This will
     * cause the pipeline to operator in "pull mode". In contrast you can also
     * start a source within your pipeline, which will cause to operate in
     * "push mode".
     */
    void start();
    void stop();
};

} // namespace core
} // namespace coro
