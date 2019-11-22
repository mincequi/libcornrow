#pragma once

#include <gstreamermm.h>
#include <gstreamermm/private/audiofilter_p.h>
#include <coro/audio/Node.h>
#include "Biquad.h"
#include "TBiquad.h"

namespace coro
{

class Peq : public Gst::AudioFilter,
            public audio::Node
{
public:
    typedef ::Gst::AudioFilter::BaseClassType BaseClassType;
    typedef ::Gst::AudioFilter::BaseObjectType BaseObjectType;
    typedef ::Gst::AudioFilter::CppClassType CppClassType;

    Peq();
    explicit Peq(GstAudioFilter *obj);

    static GType get_base_type() G_GNUC_CONST;
    static void class_init(Gst::ElementClass<Peq> *klass);

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { /*audio::Codec::RawFloat32 |*/ audio::Codec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { /*audio::Codec::RawFloat32 |*/ audio::Codec::RawInt16,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    void setVolume(float volume);

    void setFilters(const std::vector<Filter> filters);
    std::vector<Filter> filters();

    // @TODO(mawe): to be removed
    Biquad& biquad(std::uint8_t idx);

private:
    virtual bool setup_vfunc(const Gst::AudioInfo& info) override;
    static GstFlowReturn transform_ip(GstBaseTransform* self, GstBuffer* buf);
    void process(GstBuffer* buf);
    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    Gst::AudioInfo      m_audioInfo;
    float               m_volume = 1.0;
    std::deque<Biquad>  m_biquads;
    std::deque<TBiquad<int16_t, float>> m_tBiquads;

    std::mutex m_mutex;
};

} // namespace coro
