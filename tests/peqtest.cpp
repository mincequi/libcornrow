#include <assert.h>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Peq.h>

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    assert(mainloop);
    auto pipeline = Gst::Pipeline::create();
    assert(pipeline);

    // Create elements
    auto src = Gst::AudioTestSrc::create();
    assert(src);
    src->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_WHITE_NOISE);
    src->property_num_buffers().set_value(10);
    src->property_samplesperbuffer().set_value(44100);
    Glib::RefPtr<GstDsp::Peq> peq = Glib::RefPtr<GstDsp::Peq>::cast_dynamic(Gst::ElementFactory::create_element("peq"));
    assert(peq);
    //peq->biquads().at(0)->filter().set_value( { GstDsp::FilterType::Peak, 1000.0, -12.0, 0.707 } );
    peq->biquad(0).setFilter( { GstDsp::FilterType::Peak, 1000.0, -12.0, 0.707 } );
    auto enc = Gst::ElementFactory::create_element("wavenc");
    assert(enc);
    auto sink = Gst::FileSink::create();
    assert(sink);
    sink->property_location().set_value("test.wav");

    // Link elements
    pipeline->add(src)->add(peq)->add(enc)->add(sink);
    src->link(peq)->link(enc)->link(sink);

    // Handle messages posted on bus
    pipeline->get_bus()->add_watch([mainloop, pipeline] (const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message) {
        switch (message->get_message_type()) {
        case Gst::MESSAGE_EOS:
        case Gst::MESSAGE_ERROR:
            pipeline->set_state(Gst::STATE_NULL);
            mainloop->quit();
            break;
        default:
            break;
        }
        return true;
    });

    // Start
    pipeline->set_state(Gst::STATE_PLAYING);
    mainloop->run();

    return 0;
}
