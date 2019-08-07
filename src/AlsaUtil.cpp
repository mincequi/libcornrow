#include "AlsaUtil.h"

#include <alsa/asoundlib.h>

namespace GstDsp
{

AlsaUtil::AlsaUtil()
{
}

std::list<AudioDeviceInfo> AlsaUtil::enumerateDevices()
{
    void **hints, **n;
    char *name, *descr, *descr1, *io;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
        return {};
    }

    n = hints;
    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (io != NULL && strcmp(io, "Output") != 0) {
            goto __end;
        }
        printf("%s\n", name);
        if ((descr1 = descr) != NULL) {
            printf("    ");
            while (*descr1) {
                if (*descr1 == '\n')
                    printf("\n    ");
                else
                    putchar(*descr1);
                descr1++;
            }
            putchar('\n');
        }
__end:
        if (name != NULL)
            free(name);
        if (descr != NULL)
            free(descr);
        if (io != NULL)
            free(io);
        n++;
    }
    snd_device_name_free_hint(hints);

    return {};
}

} // namespace GstDsp
