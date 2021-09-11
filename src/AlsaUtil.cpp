/*
 * Copyright (C) 2021 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio/AlsaUtil.h"

#include <alsa/asoundlib.h>

namespace coro {

AlsaUtil::AlsaUtil() {
}

std::list<AudioDeviceInfo> AlsaUtil::outputDevices() {
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
            m_outputDevices.push_back(coro::AudioDeviceInfo(name));
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

} // namespace coro
