#ifndef GSTDSPLOUDNESS_H
#define GSTDSPLOUDNESS_H

#include <gstreamermm.h>
#include <gstreamermm/private/audiofilter_p.h>

namespace GstDsp
{

class Loudness : public Gst::AudioFilter
{
public:
    explicit Loudness(GstAudioFilter *obj);

    static void class_init(Gst::ElementClass<Loudness> *klass);
    virtual Gst::FlowReturn transform_ip_vfunc(const Glib::RefPtr<Gst::Buffer>& buf) override;

};

} // namespace GstDsp

#endif // GSTDSPLOUDNESS_H
