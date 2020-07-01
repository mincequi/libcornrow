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

#include <coro/core/SourceSelector.h>

#include <coro/core/Source.h>
#include <loguru/loguru.hpp>

#include <list>
#include <map>

namespace coro {
namespace core {

using namespace std::placeholders;

class SourceSelectorPrivate {
public:
    std::list<Source*> sources;

    void onSourceReady(bool ready, Source* const source) {
        // Source is not ready (and stopped)
        if (!ready) {
            LOG_F(INFO, "%s stopped", source->name());
        }

        // If another one is running, do nothing.
        for (auto s : sources) {
            if (s->isStarted()) {
                LOG_F(2, "Another source runnig: %s", s->name());
                return;
            }
        }

        // If another one wants to start, start it.
        for (auto s : sources) {
            if (s->isReady()) {
                LOG_F(INFO, "Starting %s", s->name());
                s->start();
            }
        }
    }
};

SourceSelector::SourceSelector() :
    d(new SourceSelectorPrivate)
{
}

SourceSelector::~SourceSelector()
{
    delete d;
}

void SourceSelector::addSource(Source& source)
{
    // Stop any provided source.
    source.stop();
    source.m_isControlled = true;

    // If another one is running, stop given one.
    //for (auto s : d->sources) {
    //    if (s->isStarted()) {
    //        LOG_F(INFO, "%s running, stopping: %s", s->name(), source.name());
    //        source.stop();
    //    }
    //}

    source.setReadyCallback(std::bind(&SourceSelectorPrivate::onSourceReady, d, _1, _2));
    d->sources.push_back(&source);
}

void SourceSelector::removeSource(Source& source)
{
    source.setReadyCallback(nullptr);
    d->sources.remove(&source);
    source.m_isControlled = false;
}

} // namespace core
} // namespace coro
