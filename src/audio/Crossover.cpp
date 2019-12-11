#include "audio/Crossover.h"

namespace coro
{
namespace audio
{

Crossover::Crossover()
    : m_filter( { FilterType::Crossover, 3000.0f, 0.0f, 0.5f } ),
      m_lfe(false),
      m_lp(2, 2),
      m_hp(2, 2),
      m_lfeLp(1, 2),
      m_lfeHp(2, 2)
{
    updateCrossover();
    updateLfe();
}

/*
void Crossover::class_init(Gst::ElementClass<Crossover> *klass)
{
    klass->set_metadata("Audio Crossover",
                        "Filter/Effect/Audio",
                        "Realizes an audio crossover",
                        "Manuel Weichselbaumer <mincequi@web.de>");

    // Create sink pad template
    Glib::ustring sinkString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)2, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    auto sinkCaps = Gst::Caps::create_from_string(sinkString);
    klass->add_pad_template(Gst::PadTemplate::create("sink", Gst::PAD_SINK, Gst::PAD_ALWAYS, sinkCaps));

    // Create source pad template
    // Stereo output for passthrough
    Glib::ustring sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)2, "
                "layout=(string)interleaved, "
                "format=(string)%1; ", GST_AUDIO_NE(F32));
    auto srcCaps = Gst::Caps::create_from_string(sourceString);

    // L+R+LFE
    sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)3, "
                "channel-mask=(bitmask)0xb, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    srcCaps->append(Gst::Caps::create_from_string(sourceString));

    // L+R+C
    sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)3, "
                "channel-mask=(bitmask)0x7, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    srcCaps->append(Gst::Caps::create_from_string(sourceString));

    // L+R+RL+RR
    sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)4, "
                "channel-mask=(bitmask)0x33, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    srcCaps->append(Gst::Caps::create_from_string(sourceString));

    // L+R+C+RL+RR
    sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)5, "
                "channel-mask=(bitmask)0x37, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    srcCaps->append(Gst::Caps::create_from_string(sourceString));

    // L+R+LFE+RL+RR
    sourceString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int){44100,48000}, "
                "channels=(int)5, "
                "channel-mask=(bitmask)0x3b, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    srcCaps->append(Gst::Caps::create_from_string(sourceString));

    klass->add_pad_template(Gst::PadTemplate::create("src", Gst::PAD_SRC, Gst::PAD_ALWAYS, srcCaps));

    GST_BASE_TRANSFORM_CLASS(klass->gobj())->passthrough_on_same_caps = true;
    // We never transform in-place. We are either passing through or extending the outgoing buffer
    GST_BASE_TRANSFORM_CLASS(klass->gobj())->transform_ip_on_passthrough = false;
}
*/

void Crossover::setFilter(const Filter& f)
{
    if (m_filter == f) {
        return;
    }

    m_mutex.lock();
    m_filter = f;
    if (m_filter.isValid()) {
        updateCrossover();
    }
    m_mutex.unlock();
}

/*
float Crossover::frequency()
{
    m_mutex.lock();
    float freq = m_frequency;
    m_mutex.unlock();

    return freq;
}
*/

void Crossover::setLfe(bool enable)
{
    m_mutex.lock();
    m_lfe = enable;
    updateLfe();
    m_mutex.unlock();
}

bool Crossover::lfe()
{
    m_mutex.lock();
    bool l = m_lfe;
    m_mutex.unlock();

    return l;
}

AudioConf Crossover::process(const AudioConf& conf, AudioBuffer& buffer)
{
    if (!m_filter.isValid()) {
        return conf;
    }

    auto outData = buffer.acquire(buffer.size()*2);
    auto inData = buffer.data();
    const auto frameCount = buffer.size()/conf.frameSize();

    // Front channels
    m_lp.process((float*)inData, (float*)outData, frameCount, 2, 4);
    float* out = (float*)outData;
    if (m_lowGain < 1.0f) {
        for (uint i = 0; i < frameCount; ++i) {
            for (uint c = 0; c < 2; ++c) {
                *out *= m_lowGain;
                ++out;
            }
            out += 4-2;
        }
    }
    // Rear channels
    m_hp.process((float*)inData, (float*)outData+2, frameCount, 2, 4);
    out = (float*)outData+2;
    if (m_highGain < 1.0f) {
        for (uint i = 0; i < frameCount; ++i) {
            for (uint c = 0; c < 2; ++c) {
                *out *= m_highGain;
                ++out;
            }
            out += 4-2;
        }
    }

    buffer.commit(buffer.size()*2);

    auto _conf = conf;
    _conf.channels = Channels::Quad;
    return _conf;
}

/*
bool Crossover::isFrequencyValid() const
{
    return (m_filter >= 20.0 && m_filter <= 20000.0);
}
*/

void Crossover::updateCrossover()
{
    m_lowGain = m_filter.g > 0.0 ? pow(10, (-m_filter.g/20.0)) : 1.0;
    m_highGain = m_filter.g < 0.0 ? pow(10, (m_filter.g/20.0)) : 1.0;

    m_lp.setCascadeCount(m_filter.q <= 0.5f ? 1 : 2);
    m_lp.setFilter({ FilterType::LowPass, m_filter.f, 0.0, m_filter.q });
    m_hp.setCascadeCount(m_filter.q <= 0.5f ? 1 : 2);
    m_hp.setFilter({ FilterType::HighPass, m_filter.f, 0.0, m_filter.q });
}

void Crossover::updateLfe()
{
    m_lfeLp.setFilter({ FilterType::LowPass, 80.0, 0.0, M_SQRT1_2 });
    m_lfeHp.setFilter({ FilterType::HighPass, 80.0, 0.0, M_SQRT1_2 });
}

} // namespace audio
} // namespace coro
