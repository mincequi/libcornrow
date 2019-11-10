#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <glibmm/main.h>

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
    src->property_num_buffers().set_value(10);
    src->property_samplesperbuffer().set_value(44100);

    Glib::ustring srcString = Glib::ustring::compose(
                "audio/x-raw, "
                "rate=(int)44100, "
                "channels=(int)4, "
                "channel-mask=(bitmask)0x33, "
                "layout=(string)interleaved, "
                "format=(string)%1", GST_AUDIO_NE(F32));
    auto srcCaps = Gst::Caps::create_from_string(srcString);

    auto enc = Gst::ElementFactory::create_element("avenc_ac3");
    assert(enc);
    enc->set_property("bitrate", 640000);

    // filesink
    //auto sink = Gst::FileSink::create();
    //sink->property_location().set_value("test.ac3");

    // alsasink
    auto sink = Gst::ElementFactory::create_element("alsapassthroughsink");
    sink->set_property("device", Glib::ustring("iec958:CARD=MID,DEV=0"));

    // Link elements
    pipeline->add(src)->add(enc)->add(sink);
    src->link(enc, srcCaps)->link(sink);

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
