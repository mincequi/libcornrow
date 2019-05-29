#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Crossover.h>
#include <Peq.h>

class Pipeline
{
public:
    Pipeline()
    {
        m_pipeline = Gst::Pipeline::create();

        // Create source
        m_src = Gst::AudioTestSrc::create();
        m_src->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_PINK_NOISE);
        m_src->property_num_buffers().set_value(100);
        m_src->property_samplesperbuffer().set_value(44100);

        // Create peq
        m_peq = Glib::RefPtr<GstDsp::Peq>::cast_dynamic(Gst::ElementFactory::create_element("peq"));
        m_peq->biquad(0).setFilter( { GstDsp::FilterType::Peak, 1000.0, -12.0, 0.707 } );
        m_peq->biquad(1).setFilter( { GstDsp::FilterType::LowPass, 5000.0, 0.0, 1.41 } );

        // Create crossover
        m_crossover = Glib::RefPtr<GstDsp::Crossover>::cast_dynamic(Gst::ElementFactory::create_element("crossover"));
        m_crossover->setFrequency(2000.0);
        m_crossover->setLfe(false);

        // Create encoder
        m_enc = Gst::ElementFactory::create_element("avenc_ac3");
        m_enc->set_property("bitrate", 640000);

        // Create sink
        m_sinkConverter = Gst::AudioConvert::create();
        m_alsaSink = Gst::AlsaSink::create();
        m_alsaSink->set_property("device", Glib::ustring("iec958:CARD=sndrpihifiberry,DEV=0"));
        m_alsaPassthroughSink = Gst::ElementFactory::create_element("alsapassthroughsink");
        m_alsaPassthroughSink->set_property("device", Glib::ustring("iec958:CARD=sndrpihifiberry,DEV=0"));

        // Add elements
        m_pipeline->add(m_src)->add(m_peq);
        m_src->link(m_peq);
    }

    void start()
    {
        // Add timeout handler
        Glib::signal_timeout().connect(sigc::mem_fun(this, &Pipeline::changePipeline), 5000);

        // Start
        changePipeline();
        m_pipeline->set_state(Gst::STATE_PLAYING);
    }

    ~Pipeline() {}

private:
    enum class Type {
        Normal,
        Crossover
    };

    /*
    bool constructPipeline(Type type, bool force = false)
    {
    }
    */

    bool changePipeline()
    {
        m_pipeline->set_state(Gst::STATE_NULL);

        static bool useRegularSink = true;
        useRegularSink ? construct({m_crossover, m_enc, m_alsaPassthroughSink}, {m_sinkConverter, m_alsaSink}) :
                         construct({m_sinkConverter, m_alsaSink}, {m_crossover, m_enc, m_alsaPassthroughSink});
        useRegularSink = !useRegularSink;

        m_pipeline->set_state(Gst::STATE_PLAYING);

        return true;
    }

    void construct(const std::vector<Glib::RefPtr<Gst::Element>>& oldElements, const std::vector<Glib::RefPtr<Gst::Element>>& newElements)
    {
        for (auto element : oldElements) {
            if (element->has_as_ancestor(m_pipeline)) {
                m_pipeline->remove(element);
            }
        }

        for (auto it = newElements.begin(); it != newElements.end(); ++it) {
            m_pipeline->add(*it);
            if (it == newElements.begin()) {
                m_peq->link(*it);
            } else {
                (*(std::prev(it)))->link(*it);
            }
        }
    }

    Type m_currentType = Type::Normal;

    Glib::RefPtr<Gst::AudioConvert> m_sinkConverter;
    Glib::RefPtr<Gst::AlsaSink>     m_alsaSink;
    Glib::RefPtr<Gst::AudioTestSrc> m_src;
    Glib::RefPtr<GstDsp::Crossover> m_crossover;
    Glib::RefPtr<GstDsp::Peq>       m_peq;
    Glib::RefPtr<Gst::Element>      m_enc;
    Glib::RefPtr<Gst::Element>      m_alsaPassthroughSink;

    /*
    Glib::RefPtr<Gst::Element>      m_bluetoothSource;
    Glib::RefPtr<Gst::Element>      m_alsaSink;
    Glib::RefPtr<Gst::Element>      m_alsaPassthroughSink;
    Glib::RefPtr<GstDsp::Crossover> m_crossover;
    Glib::RefPtr<GstDsp::Peq>       m_peq;
    */

    Glib::RefPtr<Gst::Pipeline>     m_pipeline;

    std::map<Type, std::vector<Glib::RefPtr<Gst::Element>>> m_elements;
};

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();

    Pipeline pipeline;
    pipeline.start();

    mainloop->run();

    return 0;
}
