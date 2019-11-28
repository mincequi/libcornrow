#pragma once

#include <gstreamermm.h>
#include <gstreamermm/private/audiofilter_p.h>

#include <coro/audio/Node.h>

#include "Biquad.h"
#include "TBiquad.h"

namespace coro
{
namespace audio
{

class Loudness : public Gst::AudioFilter,
                 public audio::Node

{
public:
    Loudness();
    explicit Loudness(GstAudioFilter *obj);

    static void class_init(Gst::ElementClass<Loudness> *klass);

    static constexpr std::array<audio::AudioCaps,1> inCaps() {
        return {{ { audio::Codec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::Codec::RawFloat32,
                    audio::SampleRate::Rate44100 | audio::SampleRate::Rate48000,
                    audio::Channels::Stereo } }};
    }

    void setLevel(uint8_t phon);

    void setVolume(float volume);

private:
    virtual bool setup_vfunc(const Gst::AudioInfo& info) override;
    static GstFlowReturn transform_ip(GstBaseTransform* self, GstBuffer* buf);
    void process(GstBuffer* buf);
    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer) override;

    Gst::AudioInfo  m_audioInfo;
    float   m_headroom = 1.0;
    float   m_volume = 1.0;
    Biquad          m_pk1;
    Biquad          m_pk2;
    Biquad          m_hs;

    TBiquad<float, float>   m_tpk1;
    TBiquad<float, float>   m_tpk2;
    TBiquad<float, float>   m_ths;

    std::mutex m_mutex;
};

} // namespace audio
} // namespace coro
