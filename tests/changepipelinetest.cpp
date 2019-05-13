#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Crossover.h>
#include <Peq.h>

Glib::RefPtr<Gst::Pipeline>     pipeline;
Glib::RefPtr<Gst::AudioConvert> sinkConverter;
Glib::RefPtr<GstDsp::Crossover> crossover;
Glib::RefPtr<GstDsp::Peq>       peq;

bool changePipeline()
{
    pipeline->set_state(Gst::STATE_NULL);
    static bool useRegularSink = true;
    if (useRegularSink) {
        peq->unlink(crossover);
        peq->link(sinkConverter);
    } else {
        peq->unlink(sinkConverter);
        peq->link(crossover);
    }
    pipeline->set_state(Gst::STATE_PLAYING);

    useRegularSink = !useRegularSink;

    return true;
}

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    assert(mainloop);
    pipeline = Gst::Pipeline::create();
    assert(pipeline);

    // Create source
    auto src = Gst::AudioTestSrc::create();
    assert(src);
    src->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_WHITE_NOISE);
    src->property_num_buffers().set_value(100);
    src->property_samplesperbuffer().set_value(44100);

    // Create peq
    peq = Glib::RefPtr<GstDsp::Peq>::cast_dynamic(Gst::ElementFactory::create_element("peq"));
    assert(peq);
    peq->biquad(0).setFilter( { GstDsp::FilterType::Peak, 1000.0, -12.0, 0.707 } );
    peq->biquad(1).setFilter( { GstDsp::FilterType::LowPass, 5000.0, 0.0, 1.41 } );
    peq->biquad(2).setFilter( { GstDsp::FilterType::HighPass, 40.0, 0.0, 0.707 } );
    peq->biquad(3).setFilter( { GstDsp::FilterType::Peak, 3000.0, -6.0, 10.0 } );
    peq->biquad(4).setFilter( { GstDsp::FilterType::Peak, 300.0, -2.0, 1.707 } );

    // Create crossover
    crossover = Glib::RefPtr<GstDsp::Crossover>::cast_dynamic(Gst::ElementFactory::create_element("crossover"));
    assert(crossover);
    crossover->setFrequency(3000.0);
    crossover->setLfe(false);

    // Create encoder
    auto enc = Gst::ElementFactory::create_element("avenc_ac3");
    assert(enc);
    enc->set_property("bitrate", 640000);

    // Create sink
    auto sinkConverter = Gst::AudioConvert::create();
    auto sink = Gst::AlsaSink::create();
    auto passthroughSink = Gst::ElementFactory::create_element("alsapassthroughsink");
    sink->set_property("device", Glib::ustring("iec958:CARD=MID,DEV=0"));

    // Link elements
    pipeline->add(src)->add(peq)->add(crossover)->add(enc)->add(sinkConverter)->add(sink)->add(passthroughSink);
    src->link(peq);
    sinkConverter->link(sink);
    crossover->link(enc)->link(passthroughSink);

    // Handle messages posted on bus
    pipeline->get_bus()->add_watch([&](const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message) {
        switch (message->get_message_type()) {
        case Gst::MESSAGE_ERROR:
        case Gst::MESSAGE_EOS:
            pipeline->set_state(Gst::STATE_NULL);
            mainloop->quit();
            break;
        default:
            break;
        }
        return true;
    });

    // Add timeout handler
    Glib::signal_timeout().connect(sigc::ptr_fun(&changePipeline), 5000);

    // Start
    changePipeline();
    pipeline->set_state(Gst::STATE_PLAYING);
    mainloop->run();

    return 0;
}
