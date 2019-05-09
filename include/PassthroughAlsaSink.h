#ifndef PASSTHROUGHALSASINK_H
#define PASSTHROUGHALSASINK_H

#include <alsa/asoundlib.h>

#include <gstreamermm.h>
#include <gstreamermm/private/audiosink_p.h>

#include "Types.h"

namespace GstDsp
{

class PassthroughAlsaSink : public Gst::AudioSink
{
public:
    explicit PassthroughAlsaSink(GstAudioSink* obj);

    static void class_init(Gst::ElementClass<PassthroughAlsaSink> *klass);

private:
    // Open the device. No configuration needs to be done at this
    // point. This function is also used to check if the device is available.
    virtual bool open_vfunc() override;

    // Prepare the device to operate with the specified parameters.
    virtual bool prepare_audiosink_vfunc(Gst::AudioRingBufferSpec& spec) override;

    // Undo operations done in prepare.
    virtual bool unprepare_vfunc() override;

    // Close the device.
    virtual bool close_vfunc() override;

    // Write data to the device.
    virtual int write_vfunc(gpointer data, guint lenght) override;

    // Return how many samples are still in the device. This is used to drive the synchronisation.
    virtual guint get_delay_vfunc() const override;

    // Return as quickly as possible from a write and flush any pending samples from the device.
    virtual void reset_vfunc() override;

    static DeviceInfoList enumerateDevices();
    static DeviceInfoList enumerateDevice(const std::string &device);
    static bool openDevice(const std::string &name, const std::string &params, snd_pcm_t **pcmp);
    static bool tryDevice(const std::string &name, snd_pcm_t **pcmp);
    static std::string aesParams(const Gst::AudioInfo& audioInfo);

    snd_pcm_t* m_pcm = nullptr;
};

} // namespace GstDsp

#endif // PASSTHROUGHALSASINK_H
