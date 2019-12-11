#include "audio/Loudness.h"

#include <iostream>

namespace coro
{
namespace audio
{

Loudness::Loudness()
    : m_tpk1(2, 1),
      m_tpk2(2, 1),
      m_ths(2, 1)
{
}

void Loudness::setLevel(uint8_t phon)
{
    // Filter 1> t: pk, f: 35.5, q: 0.56, g: <phon>/40 * 12db
    // Filter 2> t: pk, f: 100,  q: 0.25, g: <phon>/40 * 9db
    // Filter 3> t: hs, f: 1000, q: 0.8,  g: <phon>/40 * 9db
    m_tpk1.setFilter({ FilterType::Peak,         35.5f, phon*0.3f,   0.56f });
    m_tpk2.setFilter({ FilterType::Peak,        100.0f, phon*0.225f, 0.25f });
    m_ths.setFilter( { FilterType::HighShelf, 10000.0f, phon*0.225f, 0.80f });

    // Headroom generator: <phon> * -0,425 (actually 0,475).
    m_headroom = pow(10, (phon*-0.425)/20.0);
}

void Loudness::setVolume(float volume)
{
    m_volume = volume;
}

audio::AudioConf Loudness::process(const audio::AudioConf& conf, audio::AudioBuffer& buffer)
{
    // Loudness generates headroom, check which one is lower
    float volume = std::min(m_volume, m_headroom);

    // No volume, no headroom -> no processing.
    if (volume == 1.0f) {
        return conf;
    }

    int channelCount = audio::toInt(conf.channels);
    int frameCount = buffer.size()/conf.frameSize();
    float* data = (float*)buffer.data();

    // Apply volume
    for (int i = 0; i < frameCount*audio::toInt(conf.channels); ++i) {
        data[i] *= volume;
    }

    // If there is no headroom, we do not have loudness set.
    if (m_headroom == 1.0f) {
        return conf;
    }

    m_mutex.lock();

    m_tpk1.setRate(audio::toInt(conf.rate));
    m_tpk2.setRate(audio::toInt(conf.rate));
    m_ths.setRate(audio::toInt(conf.rate));
    m_tpk1.process((float*)buffer.data(), (float*)buffer.data(), frameCount, channelCount, channelCount);
    m_tpk2.process((float*)buffer.data(), (float*)buffer.data(), frameCount, channelCount, channelCount);
    m_ths.process((float*)buffer.data(), (float*)buffer.data(), frameCount,  channelCount, channelCount);

    m_mutex.unlock();

    return conf;
}

} // namespace audio
} // namespace coro
