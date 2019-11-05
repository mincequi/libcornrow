#include "Loudness.h"

#include <iostream>

namespace GstDsp
{

Loudness::Loudness(GstAudioFilter *obj)
    : Glib::ObjectBase(typeid(Loudness)),
      Gst::AudioFilter(obj),
      m_pk1(2, 1),
      m_pk2(2, 1),
      m_hs(2, 1)
{
    set_in_place(true);
    GST_BASE_TRANSFORM_GET_CLASS(Gst::BaseTransform::gobj())->transform_ip = &transform_ip;
}

void Loudness::class_init(Gst::ElementClass<Loudness> *klass)
{
    klass->set_metadata("Loudness Filter",
                        "Filter/Effect/Audio",
                        "Applies an equal-loudness contour to perceive constant loudness for different volume levels",
                        "Manuel Weichselbaumer <mincequi@web.de>");

    Glib::ustring capsString = Glib::ustring::compose(
                "audio/x-raw, "
                "layout=(string)interleaved, "
                "rate=(int){44100,48000}, "
                "channels=(int)2, "
                "format=(string)%1", GST_AUDIO_NE(F32));

    auto caps = Gst::Caps::create_from_string(capsString);

    auto sink = Gst::PadTemplate::create("sink", Gst::PAD_SINK, Gst::PAD_ALWAYS, caps);
    auto src  = Gst::PadTemplate::create("src", Gst::PAD_SRC, Gst::PAD_ALWAYS, caps);
    klass->add_pad_template(sink);
    klass->add_pad_template(src);
}

void Loudness::setLevel(uint8_t phon)
{
    // Filter 1> t: pk, f: 35.5, q: 0.56, g: <phon>/40 * 12db
    // Filter 2> t: pk, f: 100,  q: 0.25, g: <phon>/40 * 9db
    // Filter 3> t: hs, f: 1000, q: 0.8,  g: <phon>/40 * 9db
    m_pk1.setFilter({ FilterType::Peak,         35.5f, phon*0.3f,   0.56f });
    m_pk2.setFilter({ FilterType::Peak,        100.0f, phon*0.225f, 0.25f });
    m_hs.setFilter( { FilterType::HighShelf, 10000.0f, phon*0.225f, 0.80f });

    // Headroom generator: <phon> * -0,425 (actually 0,475).
    m_headroom = pow(10, (phon*-0.425)/20.0);
}

void Loudness::setVolume(float volume)
{
    m_volume = volume;
}

bool Loudness::setup_vfunc(const Gst::AudioInfo& info)
{
    if (m_audioInfo.is_equal(info)) return true;

    m_mutex.lock();
    m_audioInfo = info;

    m_pk1.setRate(m_audioInfo.get_rate());
    m_pk2.setRate(m_audioInfo.get_rate());
    m_hs.setRate(m_audioInfo.get_rate());

    m_mutex.unlock();

    std::cerr << "bytes per frame: " << info.get_bpf() << ", rate: " << info.get_rate() << ", channels: " << info.get_channels() << std::endl;

    return true;
}

GstFlowReturn Loudness::transform_ip(GstBaseTransform* self, GstBuffer* buf)
{
    Glib::ObjectBase *const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)self));
    Loudness *const obj = dynamic_cast<Loudness* const>(obj_base);
    if (obj) // This can be NULL during destruction.
    {
        // Call the virtual member method, which derived classes might override.
        obj->process(buf);
    }

    return GST_FLOW_OK;
}

void Loudness::process(GstBuffer* buf)
{
    // Loudness generates headroom, check which one is lower
    float volume = std::min(m_volume, m_headroom);

    // No volume, no headroom -> no processing.
    if (volume == 1.0f) {
        return;
    }

    GstMapInfo map;
    gst_buffer_map(buf, &map, (GstMapFlags)GST_MAP_READWRITE);

    float* data = (float*)(map.data);
    uint   frameCount = map.size/m_audioInfo.get_bpf();

    // Apply volume
    for (uint i = 0; i < frameCount*m_audioInfo.get_channels(); ++i) {
        data[i] *= volume;
    }

    // If there is no headroom, we do not have loudness set.
    if (m_headroom == 1.0f) {
        goto end;
    }

    m_mutex.lock();

    m_pk1.process(data, data, frameCount, m_audioInfo.get_channels(), m_audioInfo.get_channels());
    m_pk2.process(data, data, frameCount, m_audioInfo.get_channels(), m_audioInfo.get_channels());
    m_hs.process(data, data, frameCount, m_audioInfo.get_channels(), m_audioInfo.get_channels());

    m_mutex.unlock();

end:
    gst_buffer_unmap(buf, &map);
}

} // namespace GstDsp
