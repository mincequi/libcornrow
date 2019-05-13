#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Crossover.h>
#include <Peq.h>

Glib::RefPtr<Gst::Pipeline>     pipeline;
Glib::RefPtr<Gst::AudioConvert> sinkConverter;
Glib::RefPtr<Gst::AlsaSink>     alsaSink;
Glib::RefPtr<Gst::AudioTestSrc> src;
Glib::RefPtr<GstDsp::Crossover> crossover;
Glib::RefPtr<GstDsp::Peq>       peq;
Glib::RefPtr<Gst::Element>      enc;
Glib::RefPtr<Gst::Element>      passthruSink;

void construct(const std::vector<Glib::RefPtr<Gst::Element>>& oldElements, const std::vector<Glib::RefPtr<Gst::Element>>& newElements)
{
    for (auto element : oldElements) {
        if (element->has_as_ancestor(pipeline)) {
            pipeline->remove(element);
        }
    }

    for (auto it = newElements.begin(); it != newElements.end(); ++it) {
        pipeline->add(*it);
        if (it == newElements.begin()) {
            peq->link(*it);
        } else {
            (*(std::prev(it)))->link(*it);
        }
    }
}

bool changePipeline()
{
    pipeline->set_state(Gst::STATE_NULL);

    static bool useRegularSink = true;
    useRegularSink ? construct({crossover, enc, passthruSink}, {sinkConverter, alsaSink}) :
                     construct({sinkConverter, alsaSink}, {crossover, enc, passthruSink});
    useRegularSink = !useRegularSink;

    pipeline->set_state(Gst::STATE_PLAYING);

    return true;
}

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    pipeline = Gst::Pipeline::create();

    // Create source
    src = Gst::AudioTestSrc::create();
    src->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_PINK_NOISE);
    src->property_num_buffers().set_value(100);
    src->property_samplesperbuffer().set_value(44100);

    // Create peq
    peq = Glib::RefPtr<GstDsp::Peq>::cast_dynamic(Gst::ElementFactory::create_element("peq"));
    peq->biquad(0).setFilter( { GstDsp::FilterType::Peak, 1000.0, -12.0, 0.707 } );
    peq->biquad(1).setFilter( { GstDsp::FilterType::LowPass, 5000.0, 0.0, 1.41 } );
    peq->biquad(2).setFilter( { GstDsp::FilterType::HighPass, 40.0, 0.0, 0.707 } );
    peq->biquad(3).setFilter( { GstDsp::FilterType::Peak, 3000.0, -6.0, 10.0 } );
    peq->biquad(4).setFilter( { GstDsp::FilterType::Peak, 300.0, -2.0, 1.707 } );

    // Create crossover
    crossover = Glib::RefPtr<GstDsp::Crossover>::cast_dynamic(Gst::ElementFactory::create_element("crossover"));
    crossover->setFrequency(2000.0);
    crossover->setLfe(false);

    // Create encoder
    enc = Gst::ElementFactory::create_element("avenc_ac3");
    enc->set_property("bitrate", 640000);

    // Create sink
    sinkConverter = Gst::AudioConvert::create();
    alsaSink = Gst::AlsaSink::create();
    alsaSink->set_property("device", Glib::ustring("iec958:CARD=MID,DEV=0"));
    passthruSink = Gst::ElementFactory::create_element("alsapassthroughsink");
    passthruSink->set_property("device", Glib::ustring("iec958:CARD=MID,DEV=0"));

    // Add elements
    pipeline->add(src)->add(peq);
    src->link(peq);

    // Add timeout handler
    Glib::signal_timeout().connect(sigc::ptr_fun(&changePipeline), 5000);

    // Start
    changePipeline();
    pipeline->set_state(Gst::STATE_PLAYING);
    mainloop->run();

    return 0;
}
