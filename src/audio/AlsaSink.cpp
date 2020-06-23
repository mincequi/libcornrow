/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
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

#include "audio/AlsaSink.h"

#include "audio/SpdifTypes.h"

#include <cstdint>
#include <cstring>
#include <alsa/asoundlib.h>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

static void doAc3Payload(core::Buffer& buffer);

int snd_pcm_set_params2(snd_pcm_t *pcm,
                        snd_pcm_format_t format,
                        snd_pcm_access_t access,
                        unsigned int channels,
                        unsigned int rate,
                        snd_pcm_uframes_t period_size = 256) {
    snd_pcm_hw_params_t   *params;
    snd_pcm_hw_params_alloca(&params);
    //snd_pcm_hw_params_t params = {0};
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    //snd_pcm_sw_params_t swparams = {0};
    const char *s = snd_pcm_stream_name(snd_pcm_stream(pcm));
    int err;
    snd_pcm_uframes_t delay_size = rate / 10;
    snd_pcm_uframes_t buffer_size = rate / 2;

    assert(pcm);
    {
        /* choose all parameters */
        err = snd_pcm_hw_params_any(pcm, params);
        if (err < 0) {
            SNDERR("Broken configuration for %s: no configurations available", s);
            return err;
        }
        /* set the selected read/write format */
        err = snd_pcm_hw_params_set_access(pcm, params, access);
        if (err < 0) {
            SNDERR("Access type not available for %s: %s", s, snd_strerror(err));
            return err;
        }
        /* set the sample format */
        err = snd_pcm_hw_params_set_format(pcm, params, format);
        if (err < 0) {
            SNDERR("Sample format not available for %s: %s", s, snd_strerror(err));
            return err;
        }
        /* set the count of channels */
        err = snd_pcm_hw_params_set_channels(pcm, params, channels);
        if (err < 0) {
            SNDERR("Channels count (%i) not available for %s: %s", channels, s, snd_strerror(err));
            return err;
        }
        /* set the stream rate */
        unsigned int rrate = rate;
        err = snd_pcm_hw_params_set_rate_near(pcm, params, &rrate, 0);
        if (err < 0) {
            SNDERR("Rate %iHz not available for playback: %s", rate, snd_strerror(err));
            return err;
        }
        if (rrate != rate) {
            SNDERR("Rate doesn't match (requested %iHz, get %iHz)", rate, rrate);
            return -EINVAL;
        }
        /* set the period size */
        err = snd_pcm_hw_params_set_period_size_near(pcm, params, &period_size, NULL);
        if (err < 0) {
            SNDERR("Unable to set period size %i for %s: %s", period_size, s, snd_strerror(err));
            return err;
        }

        // set the buffer size
        buffer_size = std::max(period_size * 16, buffer_size);
        err = snd_pcm_hw_params_set_buffer_size_near(pcm, params, &buffer_size);
        if (err < 0) {
            SNDERR("Unable to set buffer size %lu %s: %s", buffer_size, s, snd_strerror(err));
            return err;
        }
    }

    /* write the parameters to device */
    err = snd_pcm_hw_params(pcm, params);
    if (err < 0) {
        SNDERR("Unable to set hw params for %s: %s", s, snd_strerror(err));
        return err;
    }

    /* get the current swparams */
    err = snd_pcm_sw_params_current(pcm, swparams);
    if (err < 0) {
        SNDERR("Unable to determine current swparams for %s: %s", s, snd_strerror(err));
        return err;
    }

    // start the transfer when the buffer with delay.
    delay_size = std::max(period_size * 4, delay_size);
    err = snd_pcm_sw_params_set_start_threshold(pcm, swparams, delay_size);
    if (err < 0) {
        SNDERR("Unable to set start threshold mode for %s: %s", s, snd_strerror(err));
        return err;
    }
    /*
     * allow the transfer when at least period_size samples can be processed
     */
    err = snd_pcm_sw_params_set_avail_min(pcm, swparams, period_size);
    if (err < 0) {
        SNDERR("Unable to set avail min for %s: %s", s, snd_strerror(err));
        return err;
    }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(pcm, swparams);
    if (err < 0) {
        SNDERR("Unable to set sw params for %s: %s", s, snd_strerror(err));
        return err;
    }

    return 0;
}

AlsaSink::AlsaSink()
{
}

AlsaSink::~AlsaSink()
{
}

void AlsaSink::start(const AudioConf& conf)
{
    //open(conf);
    openSimple(conf);

    // Get current params
    snd_pcm_sw_params_t* params;
    snd_pcm_sw_params_alloca(&params);
    snd_pcm_hw_params_t* hwparams;
    snd_pcm_hw_params_alloca(&hwparams);
    int err = snd_pcm_sw_params_current(m_pcm, params);
    if (err < 0) {
        LOG_F(ERROR, "snd_pcm_sw_params_current() failed.\n");
        return;
    }
    err = snd_pcm_hw_params_current(m_pcm, hwparams);
    if (err < 0) {
        LOG_F(ERROR, "snd_pcm_hw_params_current() failed.\n");
        return;
    }

    snd_pcm_uframes_t frames = 0;
    snd_pcm_sw_params_get_start_threshold(params, &frames);
    snd_pcm_uframes_t min = 0;
    snd_pcm_sw_params_get_avail_min(params, &min);
    snd_pcm_uframes_t buffer_size;
    err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    if (err < 0) {
        LOG_F(WARNING, "snd_pcm_hw_params_get_buffer_time() failed.\n");
    }
    LOG_F(INFO, "Device opened. delay: %u ms, buffer: %u ms",
          (uint)frames * 1000 / toInt(conf.rate),
          (uint)buffer_size * 1000 / toInt(conf.rate));

    snd_pcm_prepare(m_pcm);
}

void AlsaSink::setDevice(const std::string& device)
{
    if (device == m_device) {
        return;
    }

    m_device = device;
    onStop();
    start(m_conf);
}

AudioConf AlsaSink::onProcess(const AudioConf& conf, core::Buffer& buffer)
{
    if (m_conf != conf) {
        onStop();
        start(conf);
        m_conf = conf;
    }

    if (!m_pcm) {
        start(conf);
    }

    if (conf.codec == AudioCodec::Ac3) {
        doAc3Payload(buffer);
    }

    /*
    if (!write(buffer.data(), buffer.size())) {
        stop();
        start(conf);
    }
    */

    //
    //snd_pcm_sframes_t delay = 0;
    //snd_pcm_delay(m_pcm, &delay);
    //LOG_F(1, "Device delay: %zu ms", delay * 1000 / toInt(m_conf.rate));

    writeSimple(buffer.data(), buffer.size());

    buffer.clear();

    return conf;
}

void AlsaSink::onStop()
{
    if (m_pcm) {
        snd_pcm_drain(m_pcm);
        snd_pcm_close(m_pcm);
        m_pcm = nullptr;
    }
    LOG_F(INFO, "Device stopped");
}

bool AlsaSink::openSimple(const AudioConf& conf)
{
    if (m_pcm) {
        LOG_F(WARNING, "Device already opened");
        return false;
    }

    int err = snd_pcm_open(&m_pcm, m_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (err) {
        LOG_F(ERROR, "Unable to open ALSA device '%s'\n", m_device.c_str());
        return false;
    }

    unsigned int rate = toInt(conf.rate);
    err = snd_pcm_set_params2(m_pcm,
                              SND_PCM_FORMAT_S16,
                              SND_PCM_ACCESS_RW_INTERLEAVED,
                              2,
                              rate);
    if (err) {
        LOG_F(WARNING, "snd_pcm_set_params2() failed.");
        return false;
    }

    return true;
}

void AlsaSink::writeSimple(const char* samples, uint32_t bytesCount)
{
    snd_pcm_sframes_t frameCount = bytesCount / 4;
    char* ptr = (char*)samples;

    while (frameCount > 0) {
        auto ret = snd_pcm_writei(m_pcm, ptr, frameCount);
        // If all ok
        if (ret == 0) {
            return;
        }
        // If failed, try to recover
        if (ret < 0) {
            LOG_F(WARNING, "Write failed: %s", snd_strerror(ret));
            ret = snd_pcm_recover(m_pcm, ret, 0);
        }
        // If recover failed
        if (ret < 0) {
            LOG_F(WARNING, "Recovery failed: %s", snd_strerror(ret));
            break;
        }
        // If less frames written
        if (ret > 0 && ret < frameCount) {
            LOG_F(WARNING, "Frames written: %zd. expected: %zd.", ret, frameCount);
        }

        frameCount -= ret;
        ptr += ret * 4;
    }
}

bool AlsaSink::recover(int err)
{
    // underrun
    if (err == -EPIPE) {
        LOG_F(WARNING, "AlsaSink underrun");
        err = snd_pcm_prepare(m_pcm);
        if (err < 0) {
            LOG_F(ERROR, "AlsaSink cannot be recovered from underrun");
            // Cannot recover from underrun
            return false;
        }
        return true;
    } else if (err == -ESTRPIPE) {
        LOG_F(WARNING, "AlsaSink suspended");
        // wait until suspend flag clears
        while ((err = snd_pcm_resume(m_pcm)) == -EAGAIN)
            sleep(1);

        if (err < 0) {
            err = snd_pcm_prepare(m_pcm);
            if (err < 0) {
                LOG_F(ERROR, "AlsaSink cannot be recovered from suspend");
                // Cannot recover from suspend
                return false;
            }
        }
        return true;
    }

    LOG_F(WARNING, "AlsaSink unrecoverable");
    // unrecoverable
    return false;
}

void doAc3Payload(core::Buffer& buffer)
{
    if (buffer.size() > (spdif::ac3FrameSize - spdif::SpdifAc3Header::size())) {
        LOG_F(WARNING, "Frame too big, droppping it.");
        buffer.clear();
        return;
    }

    spdif::SpdifAc3Header ac3Header(buffer.data(), buffer.size());
    buffer.prepend((char*)&ac3Header, 8);

#if __BYTE_ORDER == __LITTLE_ENDIAN
    auto ac3Data = buffer.data() + ac3Header.size();
    const auto size = buffer.size() - ac3Header.size();

    for (uint32_t i = 0; i < size; i += 2) {
        *(uint16_t*)(ac3Data+i) = __bswap_16(*(uint16_t*)(ac3Data+i));
    }
#endif

    buffer.grow(spdif::ac3FrameSize);
}

} // namespace audio
} // namespace coro
