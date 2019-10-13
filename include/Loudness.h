#pragma once

#include <gstreamermm.h>
#include <gstreamermm/private/audiofilter_p.h>

#include "Biquad.h"

namespace GstDsp
{

class Loudness : public Gst::AudioFilter
{
public:
    explicit Loudness(GstAudioFilter *obj);

    static void class_init(Gst::ElementClass<Loudness> *klass);

    void setLevel(uint8_t phon);

    void setVolume(float volume);

private:
    virtual bool setup_vfunc(const Gst::AudioInfo& info) override;
    static GstFlowReturn transform_ip(GstBaseTransform* self, GstBuffer* buf);
    void process(GstBuffer* buf);

    Gst::AudioInfo  m_audioInfo;
    float   m_headroom = 1.0;
    float   m_volume = 1.0;
    Biquad          m_pk1;
    Biquad          m_pk2;
    Biquad          m_hs;

    std::mutex m_mutex;
};

} // namespace GstDsp
