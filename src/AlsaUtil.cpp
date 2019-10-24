#include "gstdsp/AlsaUtil.h"

#include <alsa/asoundlib.h>

namespace GstDsp
{

AlsaUtil::AlsaUtil()
{
}

std::list<AudioDeviceInfo> AlsaUtil::outputDevices()
{
    m_outputDevices.clear();

    void** hints;
    void** n;
    char*  name;
    char*  desc;
    char*  ioid;

    // Enumerate sound devices
    if (snd_device_name_hint(-1, "pcm", &hints)) {
        return {};
    }

    n = hints;
    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        desc = snd_device_name_get_hint(*n, "DESC");
        ioid = snd_device_name_get_hint(*n, "IOID");

        if (ioid && strcmp(ioid, "Output")) {
            goto end;
        }

        if (!strncmp(name, "default", 7) ||
                !strncmp(name, "iec958", 6) ||
                !strncmp(name, "spdif", 5)) {
            m_outputDevices.push_back(GstDsp::AudioDeviceInfo(name));
        }

        end:
        if (name) free(name);
        if (desc) free(desc);
        if (ioid) free(ioid);
        ++n;
    }

    // Free hint buffer
    snd_device_name_free_hint(hints);

    return m_outputDevices;
}

} // namespace GstDsp
