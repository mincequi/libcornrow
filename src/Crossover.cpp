#include "Crossover.h"

#include <cmath>
#include <iostream>
#include <gstreamermm/private/basetransform_p.h>

#include "Util.h"

namespace GstDsp
{

Crossover::Crossover(GstBaseTransform* obj)
    : Glib::ObjectBase(typeid(Crossover)),
      Gst::BaseTransform(obj),
      m_frequency(*this,    "frequency",    3000.0),
      m_lfe(*this,          "lfe",          false),
      m_lp(2, 2),
      m_hp(2, 2),
      m_lfeLp(1, 2),
      m_lfeHp(2, 2)
{
    m_frequency.get_proxy().signal_changed().connect(sigc::mem_fun(*this, &Crossover::updateCrossover));
}

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

Gst::FlowReturn Crossover::transform_vfunc(const Glib::RefPtr<Gst::Buffer>& inbuf, const Glib::RefPtr<Gst::Buffer>& outbuf)
{
    Gst::MapInfo inInfo;
    Gst::MapInfo outInfo;
    bool success = inbuf->map(inInfo, Gst::MAP_READ);
    success = outbuf->map(outInfo, Gst::MAP_WRITE);

    float* const inData = (float*)(inInfo.get_data());
    float* const outData = (float*)(outInfo.get_data());

    uint   frameCount = inInfo.get_size()/m_info.get_bpf();

    uint outChannelCount = 0;
    bool frequencyValid = (m_frequency >= 90.0 && m_frequency <= 18000.0);
    if (frequencyValid && !m_lfe) {   // Only crossover: L+R+RL+RR
        outChannelCount = 4;
    } else if (!frequencyValid && m_lfe) { // Only LFE: L+R+LFE
        outChannelCount = 3;
    } else if (frequencyValid && m_lfe) { // Crossover + LFE: L+R+LFE+RL+RR
        outChannelCount = 5;
    } else {
        return Gst::FlowReturn::FLOW_NOT_SUPPORTED;
    }

    // LFE
    if (m_lfe) {
        // mixdown stereo input to mono
        std::uint32_t frameCount = inInfo.get_size()/sizeof(float)/2.0;
        StereoFrame<float>* inFrames = (StereoFrame<float>*)(inInfo.get_data());
        QuadLfeFrame<float>* outFrames = (QuadLfeFrame<float>*)(outInfo.get_data());
        processLfe(inFrames, outFrames, frameCount);

        // low pass LFE channel
        m_lfeLp.process(outData+2, outData+2, frameCount, outChannelCount, outChannelCount);
        // high pass front channels
        m_lfeHp.process(inData, outData, frameCount, 2, outChannelCount);
    }

    if (frequencyValid && m_lfe) {
        // First process rear channels
        m_hp.process(outData, outData+outChannelCount-2, frameCount, outChannelCount, outChannelCount);
        // Front channels
        m_lp.process(outData, outData, frameCount, outChannelCount, outChannelCount);
    } else if (frequencyValid) {
        // Front channels
        m_lp.process(inData, outData, frameCount, 2, outChannelCount);
        // Rear channels
        m_hp.process(outData, outData+outChannelCount-2, frameCount, 2, outChannelCount);
    }

    inbuf->unmap(inInfo);
    outbuf->unmap(outInfo);

    return Gst::FlowReturn::FLOW_OK;
}

Glib::RefPtr<Gst::Caps> Crossover::transform_caps_vfunc(Gst::PadDirection direction, const Glib::RefPtr<Gst::Caps>& caps, const Glib::RefPtr<Gst::Caps>& filter)
{
    //std::cerr << "transform_caps_vfunc> " << "direction: " << direction << ", inCaps: " << caps->to_string() << std::endl;

    // The input caps are guaranteed to be a simple caps with just one structure. So, take first one.
    auto outStructure = caps->get_structure(0);

    switch (direction) {
    // The out pad (source pad)
    case Gst::PadDirection::PAD_SRC:
        // Regardless what the out caps are, we only accept stereo for input.
        gst_structure_set(outStructure.gobj(), "channels", G_TYPE_INT, 2, NULL);
        // We also set a channel mask for stereo (passthrough). Otherwise comparison of pads will fail.
        gst_structure_set(outStructure.gobj(), "channel-mask", GST_TYPE_BITMASK, 0x3, NULL);
        break;
    case Gst::PadDirection::PAD_SINK: {
        bool frequencyValid = (m_frequency >= 90.0 && m_frequency <= 18000.0);
        if (frequencyValid && !m_lfe) {   // Only crossover: L+R+RL+RR
            gst_structure_set(outStructure.gobj(), "channels", G_TYPE_INT, 4, NULL);
            gst_structure_set(outStructure.gobj(), "channel-mask", GST_TYPE_BITMASK, 0x33, NULL);
        } else if (!frequencyValid && m_lfe) { // Only LFE: L+R+LFE
            gst_structure_set(outStructure.gobj(), "channels", G_TYPE_INT, 3, NULL);
            gst_structure_set(outStructure.gobj(), "channel-mask", GST_TYPE_BITMASK, 0xb, NULL);
        } else if (frequencyValid && m_lfe) { // Crossover + LFE: L+R+LFE+RL+RR
            gst_structure_set(outStructure.gobj(), "channels", G_TYPE_INT, 5, NULL);
            gst_structure_set(outStructure.gobj(), "channel-mask", GST_TYPE_BITMASK, 0x3b, NULL);
        } else {
            gst_structure_set(outStructure.gobj(), "channels", G_TYPE_INT, 2, NULL);
            // We also set a channel mask for stereo (passthrough). Otherwise comparison of pads will fail.
            gst_structure_set(outStructure.gobj(), "channel-mask", GST_TYPE_BITMASK, 0x3, NULL);
        }
        break;
    }
    default:
        std::cerr << "Unhandled case for pad direction: " << direction << std::endl;
        break;
    }

    return Gst::Caps::create(outStructure);
}

//Glib::RefPtr<Gst::Caps> Crossover::fixate_caps_vfunc(Gst::PadDirection direction, const Glib::RefPtr<Gst::Caps>& caps, const Glib::RefPtr<Gst::Caps>& othercaps)
//{
//    std::cerr << "fixate_caps_vfunc> " << "direction: " << direction << ", caps: " << caps->to_string() << ", otherCaps: " << othercaps->to_string() << std::endl;
//
//    return othercaps;
//}

bool Crossover::set_caps_vfunc(const Glib::RefPtr<Gst::Caps>& incaps, const Glib::RefPtr<Gst::Caps>& outcaps)
{
    m_info.from_caps(incaps);
    //std::cerr << "set_caps_vfunc> " << "incaps: " << incaps->to_string() << ", outcaps: " << outcaps->to_string() << std::endl;
    m_lp.setRate(m_info.get_rate());
    m_hp.setRate(m_info.get_rate());
    m_lfeLp.setRate(m_info.get_rate());
    m_lfeHp.setRate(m_info.get_rate());
    updateLfe();

    return true;
}

bool Crossover::get_unit_size_vfunc(const Glib::RefPtr<Gst::Caps>& caps, gsize& size) const
{
    Gst::AudioInfo info;
    if (!info.from_caps(caps)) {
        return false;
    }
    size = info.get_bpf();
    return info.get_bpf() > 0;
}

void Crossover::updateCrossover()
{
    m_lp.setFilter({ FilterType::LowPass, m_frequency, 0.0, M_SQRT1_2 });
    m_hp.setFilter({ FilterType::HighPass, m_frequency, 0.0, M_SQRT1_2 });
}

void Crossover::updateLfe()
{
    m_lfeLp.setFilter({ FilterType::LowPass,  180.0, 0.0, M_SQRT1_2 });
    m_lfeHp.setFilter({ FilterType::HighPass,  180.0, 0.0, M_SQRT1_2 });
}

} // namespace GstDsp