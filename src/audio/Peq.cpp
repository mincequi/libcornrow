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

audio::AudioConf Peq::onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
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
