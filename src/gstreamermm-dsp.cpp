#include <gstreamermm-dsp.h>

#include <gstreamermm.h>

#include "Crossover.h"
#include "Loudness.h"
#include "Peq.h"

#include "gstalsapassthroughsink.h"

namespace GstDsp
{

bool registerElements(Glib::RefPtr<Gst::Plugin> plugin)
{
    bool success = true;

    success &= Gst::ElementFactory::register_element(plugin, "loudness",  GST_RANK_NONE, Gst::register_mm_type<Loudness>("loudness"));
    success &= Gst::ElementFactory::register_element(plugin, "peq",       GST_RANK_NONE, Gst::register_mm_type<Peq>("peq"));
    success &= Gst::ElementFactory::register_element(plugin, "crossover", GST_RANK_NONE, Gst::register_mm_type<Crossover>("crossover"));

    success &= gst_element_register(plugin->gobj(), "alsapassthroughsink", GST_RANK_PRIMARY, GST_TYPE_ALSA_PASSTHROUGH_SINK);

    return success;
}

bool init()
{
    Gst::init();
    return Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
                                        "dsp", "Plugin offering DSP functionalities",
                                        sigc::ptr_fun(&registerElements),
                                        "0.1.0", "Proprietary", "", "", "");
}

} // namespace GstDsp
