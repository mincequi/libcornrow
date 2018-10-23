#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Crossover.h>

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    assert(mainloop);
    auto pipeline = Gst::Pipeline::create();
    assert(pipeline);

    // Create elements
    Glib::RefPtr<Gst::Element> src;
    if (std::string(argv[1]) == std::string("noise")) {
        auto noise = Gst::AudioTestSrc::create();
        assert(noise);
        noise->property_wave().set_value(Gst::AudioTestSrcWave::AUDIO_TEST_SRC_WAVE_WHITE_NOISE);
        noise->property_num_buffers().set_value(10);
        noise->property_samplesperbuffer().set_value(44100);
        src = noise;
    } else {
        auto file = Gst::FileSrc::create();
        assert(file);
        file->set_property("location", Glib::ustring(argv[1]));
        src = file;
    }
    auto dec = Gst::ElementFactory::create_element("wavparse");
    assert(dec);
    auto conv = Gst::ElementFactory::create_element("audioconvert");
    assert(conv);
    Glib::RefPtr<GstDsp::Crossover> xo = Glib::RefPtr<GstDsp::Crossover>::cast_dynamic(Gst::ElementFactory::create_element("crossover"));
    assert(xo);
    xo->set_property("frequency", 2000.0);
    xo->set_property("lfe", true);
    auto enc = Gst::ElementFactory::create_element("avenc_ac3");
    assert(enc);
    enc->set_property("bitrate", 640000);
    auto sink = Gst::FileSink::create();
    assert(sink);
    sink->property_location().set_value("test.ac3");

    // Link elements
    pipeline->add(src)->add(dec)->add(conv)->add(xo)->add(enc)->add(sink);
    src->link(dec)->link(conv)->link(xo)->link(enc)->link(sink);

    // Handle messages posted on bus
    int run = 0;
    pipeline->get_bus()->add_watch([&](const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message) {
        switch (message->get_message_type()) {
        case Gst::MESSAGE_EOS:
            switch (run) {
            case 0: // 4.0
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 1000.0);
                break;
            case 1:
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 500.0);
                break;
            case 2:
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 250.0);
                break;
            case 99: // 2.0
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 0.0);
                break;
            case 4: // 4.1
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 1000.0);
                xo->set_property("lfe", true);
            case 5: // 2.1
                ++run;
                pipeline->set_state(Gst::STATE_READY);
                pipeline->set_state(Gst::STATE_PLAYING);
                xo->set_property("frequency", 89.0);
                xo->set_property("lfe", true);
            default:
                pipeline->set_state(Gst::STATE_NULL);
                mainloop->quit();
            }
            break;
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
