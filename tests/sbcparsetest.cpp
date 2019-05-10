#include <assert.h>
#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm-dsp.h>
#include <gst/gstbuffer.h>
#include <gst/base/gstbaseparse.h>

int main(int argc, char** argv)
{
    assert(GstDsp::init());

    GstBuffer * buffer = gst_buffer_new ();
    GstBaseParseFrame *	frame = gst_base_parse_frame_new (buffer, GST_BASE_PARSE_FRAME_FLAG_NONE, 0);
    gint skipssize = 0;

    // parse
    auto parse = Gst::ElementFactory::create_element("sbcparse");
    GST_BASE_PARSE_GET_CLASS(parse->gobj())->handle_frame(GST_BASE_PARSE(parse->gobj()), frame, &skipssize);

    return 0;
}
