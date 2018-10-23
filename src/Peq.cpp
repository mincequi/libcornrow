#include "Peq.h"

#include <iostream>

namespace GstDsp
{

Peq::Peq(GstAudioFilter *obj)
    : Glib::ObjectBase(typeid(Peq)),
      Gst::AudioFilter(obj)
{
    set_in_place(true);
    GST_BASE_TRANSFORM_GET_CLASS(Gst::BaseTransform::gobj())->transform_ip = &transform_ip;
}

GType Peq::get_base_type()
{
    return Gst::AudioFilter::get_base_type();
}

void Peq::class_init(Gst::ElementClass<Peq> *klass)
{
    klass->set_metadata("Parametric Equalizer",
                        "Filter/Effect/Audio",
                        "A fully parametric equalizer",
                        "Manuel Weichselbaumer <mincequi@web.de>");

    Glib::ustring capsString = Glib::ustring::compose(
                "audio/x-raw, "
                "format=(string)%1, "
                "rate=(int){44100,48000}, "
                "channels=(int)2, "
                "layout=(string)interleaved", GST_AUDIO_NE(F32));
    auto caps = Gst::Caps::create_from_string(capsString);

    auto sink = Gst::PadTemplate::create("sink", Gst::PAD_SINK, Gst::PAD_ALWAYS, caps);
    auto src  = Gst::PadTemplate::create("src", Gst::PAD_SRC, Gst::PAD_ALWAYS, caps);
    klass->add_pad_template(sink);
    klass->add_pad_template(src);
}

Biquad& Peq::biquad(std::uint8_t idx)
{
    m_mutex.lock();
    if (idx >= m_biquads.size()) {
        // Always 2 channels
        m_biquads.resize(idx+1, Biquad(2, 1, m_audioInfo.get_rate()));
    }
    m_mutex.unlock();

    return m_biquads.at(idx);
}

bool Peq::setup_vfunc(const Gst::AudioInfo& info)
{
    if (m_audioInfo.is_equal(info)) return true;

    m_mutex.lock();
    m_audioInfo = info;
    for (auto& biquad : m_biquads) {
        biquad.setRate(m_audioInfo.get_rate());
    }
    m_mutex.unlock();

    std::cerr << "bytes per frame: " << info.get_bpf() << ", rate: " << info.get_rate() << ", channels: " << info.get_channels() << std::endl;

    return true;
}

GstFlowReturn Peq::transform_ip(GstBaseTransform* self, GstBuffer* buf)
{
    Glib::ObjectBase *const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)self));
    Peq *const obj = dynamic_cast<Peq* const>(obj_base);
    if (obj) // This can be NULL during destruction.
    {
        // Call the virtual member method, which derived classes might override.
        obj->process(buf);
    }

    return GST_FLOW_OK;
}

void Peq::process(GstBuffer* buf)
{
    GstMapInfo map;
    gst_buffer_map(buf, &map, (GstMapFlags)GST_MAP_READWRITE);

    float* data = (float*)(map.data);
    uint   frameCount = map.size/m_audioInfo.get_bpf();

    m_mutex.lock();
    for (auto& biquad : m_biquads) {
        biquad.process(data, data, frameCount, m_audioInfo.get_channels(), m_audioInfo.get_channels());
    }
    m_mutex.unlock();

    gst_buffer_unmap(buf, &map);
}

} // namespace GstDsp
