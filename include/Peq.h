#pragma once

#include <gstreamermm.h>
#include <gstreamermm/private/audiofilter_p.h>

#include "Biquad.h"

namespace GstDsp
{

class Peq : public Gst::AudioFilter
{
public:
    typedef ::Gst::AudioFilter::BaseClassType BaseClassType;
    typedef ::Gst::AudioFilter::BaseObjectType BaseObjectType;
    typedef ::Gst::AudioFilter::CppClassType CppClassType;

    explicit Peq(GstAudioFilter *obj);

    static GType get_base_type() G_GNUC_CONST;
    static void class_init(Gst::ElementClass<Peq> *klass);

    Biquad& biquad(std::uint8_t idx);

private:
    virtual bool setup_vfunc(const Gst::AudioInfo& info) override;
    static GstFlowReturn transform_ip(GstBaseTransform* self, GstBuffer* buf);
    void process(GstBuffer* buf);

    Gst::AudioInfo           m_audioInfo;
    std::deque<Biquad>       m_biquads;

    std::mutex m_mutex;
};

} // namespace GstDsp
