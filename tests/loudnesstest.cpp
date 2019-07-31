#include <assert.h>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

#include <Loudness.h>

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    // Create mainloop and pipeline
    auto mainloop = Glib::MainLoop::create();
    assert(mainloop);
    auto pipeline = Gst::Pipeline::create();
    assert(pipeline);

    auto src = Gst::FileSrc::create();
    src->property_location().set_value(argv[1]);
    auto parse = Gst::ElementFactory::create_element("wavparse");
    auto convert = Gst::AudioConvert::create();

    auto loudness = Glib::RefPtr<GstDsp::Loudness>::cast_dynamic(Gst::ElementFactory::create_element("loudness"));
    assert(loudness);
    loudness->setLevel(std::atoi(argv[2]));
    auto enc = Gst::ElementFactory::create_element("wavenc");
    assert(enc);
    auto sink = Gst::FileSink::create();
    assert(sink);
    sink->property_location().set_value("test.wav");

    // Link elements
    pipeline->add(src)->add(parse)->add(convert)->add(loudness)->add(enc)->add(sink);
    src->link(parse)->link(convert)->link(loudness)->link(enc)->link(sink);

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
