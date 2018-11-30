#ifndef GSTDSPCROSSOVER_H
#define GSTDSPCROSSOVER_H

#include <gstreamermm.h>

#include "Biquad.h"

namespace GstDsp
{

class Crossover : public Gst::BaseTransform
{
public:
    explicit Crossover(GstBaseTransform* obj);

    static void class_init(Gst::ElementClass<Crossover> *klass);

    Glib::PropertyProxy<float> frequency();
    Glib::PropertyProxy<bool>  lfe();

private:
    /// Processing overrides
    virtual Gst::FlowReturn transform_vfunc(const Glib::RefPtr<Gst::Buffer>& inbuf, const Glib::RefPtr<Gst::Buffer>& outbuf) override;

    /// Negotiation overrides
    virtual Glib::RefPtr<Gst::Caps> transform_caps_vfunc(Gst::PadDirection direction, const Glib::RefPtr<Gst::Caps>& caps, const Glib::RefPtr<Gst::Caps>& filter) override;
    // No need to reimplement fixate_caps. transform_caps already sets appropriate caps and default impl of fixate_caps does the rest.
    //virtual Glib::RefPtr<Gst::Caps> fixate_caps_vfunc(Gst::PadDirection direction, const Glib::RefPtr<Gst::Caps>& caps, const Glib::RefPtr<Gst::Caps>& othercaps) override;
    virtual bool set_caps_vfunc(const Glib::RefPtr<Gst::Caps>& incaps, const Glib::RefPtr<Gst::Caps>& outcaps) override;

    // Allocation overrides
    virtual bool get_unit_size_vfunc(const Glib::RefPtr<Gst::Caps>& caps, gsize& size) const override;

    bool isFrequencyValid() const;
    void updateCrossover();
    void updateLfe();

    template<typename InFrame, typename OutFrame>
    static void processLfe(InFrame* inFrames, OutFrame* outFrames, std::uint32_t frameCount)
    {
        for (std::uint32_t i = 0; i < frameCount; ++i) {
            outFrames[i].lfe = inFrames[i].left*M_SQRT1_2/*0.5*/ + inFrames[i].right*M_SQRT1_2/*0.5*/;
        }
    }

    Glib::Property<float>   m_frequency;
    Glib::Property<bool>    m_lfe;
    Gst::AudioInfo          m_info;

    Biquad  m_lp;
    Biquad  m_hp;
    Biquad  m_lfeLp;
    Biquad  m_lfeHp;
};

} // namespace GstDsp

#endif // GSTDSPCROSSOVER_H
