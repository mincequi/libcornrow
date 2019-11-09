#include <gstreamermm-dsp.h>

#include <gstreamermm.h>

#include "Crossover.h"
#include "Loudness.h"
#include "Peq.h"
#include <core/AppSource.h>
#include <core/FdSource.h>
#include "rtp/RtpSbcDepay.h"

#include "gstalsapassthroughsink.h"
#include "gstavdtpsrc.h"
#include "gstfdsrc.h"
#include "gstsbcparse.h"

namespace GstDsp
{

bool registerElements(Glib::RefPtr<Gst::Plugin> plugin)
{
    bool success = true;

    success &= Gst::ElementFactory::register_element(plugin, "loudness",  GST_RANK_NONE, Gst::register_mm_type<Loudness>("loudness"));
    success &= Gst::ElementFactory::register_element(plugin, "peq",       GST_RANK_NONE, Gst::register_mm_type<Peq>("peq"));
    success &= Gst::ElementFactory::register_element(plugin, "crossover", GST_RANK_NONE, Gst::register_mm_type<Crossover>("crossover"));

    success &= gst_element_register(plugin->gobj(), "alsapassthroughsink", GST_RANK_PRIMARY, GST_TYPE_ALSA_PASSTHROUGH_SINK);
    success &= gst_element_register(plugin->gobj(), "avdtpsrc2", GST_RANK_PRIMARY, GST_TYPE_AVDTP_SRC2);
    success &= gst_element_register(plugin->gobj(), "cr_appsrc", GST_RANK_PRIMARY, CR_TYPE_APP_SOURCE);
    success &= gst_element_register(plugin->gobj(), "cr_fdsrc", GST_RANK_PRIMARY+1, CR_TYPE_FD_SOURCE);
    success &= gst_element_register(plugin->gobj(), "sbcparse", GST_RANK_PRIMARY+1, GST_TYPE_SBC_PARSE);

    success &= gst_element_register(plugin->gobj(), "cr_rtpsbcdepay", GST_RANK_SECONDARY, CR_TYPE_RTP_SBC_DEPAY);

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
