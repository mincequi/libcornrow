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

#include "audio/Peq.h"

#include <iostream>

namespace coro {
namespace audio {

Peq::Peq()
{
}

void Peq::setVolume(float volume)
{
    m_volume = volume;
}

void Peq::setFilters(const std::vector<Filter> filters)
{
    m_mutex.lock();
    m_tBiquads.resize(filters.size(), TBiquad<float, float>(2, 1, toInt(m_conf.rate)));
    for (size_t i = 0; i < filters.size(); ++i) {
        m_tBiquads.at(i).setFilter(filters.at(i));
    }
    m_mutex.unlock();
}

std::vector<Filter> Peq::filters()
{
    m_mutex.lock();
    std::vector<Filter> filters;
    filters.reserve(m_tBiquads.size());
    for (const auto& biquad : m_tBiquads) {
        filters.push_back(biquad.m_filter);
    }
    m_mutex.unlock();

    return filters;
}

const char* Peq::name() const
{
    return "PEQ";
}

audio::AudioConf Peq::onProcess(const audio::AudioConf& conf, core::Buffer& buffer)
{
    uint frameCount = buffer.size()/conf.frameSize();
    m_mutex.lock();
    m_conf = conf;
    for (auto& biquad : m_tBiquads) {
        biquad.setRate(toInt(conf.rate));
        biquad.process((float*)buffer.data(), (float*)buffer.data(), frameCount, audio::toInt(conf.channels), audio::toInt(conf.channels));
    }
    m_mutex.unlock();

    return conf;
}

} // namespace audio
} // namespace coro
