#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Crossover.h>

int main(int argc, char** argv)
{
    assert(coro::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    assert(mainloop);
    auto pipeline = Gst::Pipeline::create();
    assert(pipeline);

    // Create elements
    auto src = Gst::AudioTestSrc::create();
    assert(src);
    src->property_volume().set_value(1.0);
    src->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_WHITE_NOISE);
    src->property_num_buffers().set_value(100);
    src->property_samplesperbuffer().set_value(44100);

    Glib::RefPtr<coro::Crossover> xo = Glib::RefPtr<coro::Crossover>::cast_dynamic(Gst::ElementFactory::create_element("crossover"));
    assert(xo);
    xo->setFrequency(2000.0);
    xo->setLfe(true);
    auto enc = Gst::ElementFactory::create_element("avenc_ac3");
    assert(enc);
    enc->set_property("bitrate", 640000);
    auto sink = Gst::FileSink::create();
    assert(sink);
    sink->property_location().set_value("test.ac3");

    // Link elements
    pipeline->add(src)->add(xo)->add(enc)->add(sink);
    src->link(xo)->link(enc)->link(sink);

    // Handle messages posted on bus
    pipeline->get_bus()->add_watch([&](const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message) {
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
