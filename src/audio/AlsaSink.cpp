#include "audio/AlsaSink.h"

#include "audio/SpdifTypes.h"

#include <cstring>
#include <alsa/asoundlib.h>
#include <ao/ao.h>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

AlsaSink::AlsaSink()
{
    ao_initialize();
}

AlsaSink::~AlsaSink()
{
    // This also frees options
    ao_shutdown();
}

void AlsaSink::start(const AudioConf& conf)
{
    if (m_aoDevice) {
        return;
    }

    ao_append_option(&m_aoOptions, "buffer_time", "100");
    ao_append_option(&m_aoOptions, "dev", m_device.c_str());

    ao_sample_format format;
    memset(&format, 0, sizeof(format));

    format.bits = 16;
    format.rate = conf.rate == SampleRate::Rate48000 ? 48000 : 44100;
    format.channels = 2;
    format.byte_format = AO_FMT_NATIVE;

    m_aoDevice = ao_open_live(ao_driver_id("alsa"), &format, m_aoOptions);
}

void AlsaSink::stop()
{
    if (!m_aoDevice) {
        return;
    }

    ao_free_options(m_aoOptions);
    ao_close(m_aoDevice);
    m_aoDevice = nullptr;
}

void AlsaSink::setDevice(const std::string& device)
{
    m_device = device;
}

AudioConf AlsaSink::process(const AudioConf& conf, AudioBuffer& buffer)
{
    /*
    if (m_conf != conf) {
        close();
        open(conf);
        m_conf = conf;
    }
    */

    if (conf.codec == Codec::Ac3) {
        doAc3Payload(buffer);
    }
    ao_play(m_aoDevice, (char*)buffer.data(), buffer.size());
    buffer.clear();
    return conf;
}

bool AlsaSink::open(const AudioConf& conf)
{
    if (m_pcm) {
        LOG_F(INFO, "Device already opened");
        return false;
    }

    int err = snd_pcm_open(&m_pcm, m_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (err) {
        printf("Unable to open ALSA device '%s'\n", m_device.c_str());
        return false;
    }

    /* try to set up hw params */
    if (!setHwParams(conf)) {
        printf("Unable to set HW params for device '%s'\n", m_device.c_str());
        snd_pcm_close(m_pcm);
        m_pcm = nullptr;
        return false;
    }

    /* try to set up sw params */
    if (!setSwParams()) {
        printf("Unable to set SW params for device '%s'\n", m_device.c_str());
        snd_pcm_close(m_pcm);
        m_pcm = nullptr;
        return false;
    }

    return true;
}

void AlsaSink::close()
{
    if (m_pcm) {
        snd_pcm_drain(m_pcm);
        snd_pcm_close(m_pcm);
        m_pcm = nullptr;
    }
}

bool AlsaSink::setHwParams(const AudioConf& conf)
{
    snd_pcm_hw_params_t   *params;
    snd_pcm_hw_params_alloca(&params);

    /* fetch all possible hardware parameters */
    int err = snd_pcm_hw_params_any(m_pcm, params);
    if (err < 0) {
        printf("snd_pcm_hw_params_any() failed.\n");
        return false;
    }

    /* set the access type */
    err = snd_pcm_hw_params_set_access(m_pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_access() failed.\n");
        return false;
    }

    /* set the sample bitformat */
    err = snd_pcm_hw_params_set_format(m_pcm, params, SND_PCM_FORMAT_S16);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_format() failed.\n");
        return false;
    }

    /* set the number of channels */
    err = snd_pcm_hw_params_set_channels(m_pcm, params, 2);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_channels() failed.\n");
        return false;
    }

    /* set the sample rate */
    unsigned int rate = toInt(conf.rate);
    err = snd_pcm_hw_params_set_rate_near(m_pcm, params, &rate, 0);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_rate_near() failed.\n");
        return false;
    }
    if (rate > 1.05 * toInt(conf.rate) || rate < 0.95 * toInt(conf.rate)) {
        printf("sample rate %i not supported by the hardware, using %u\n", toInt(conf.rate), rate);
    }

    /*
    // set the time per hardware sample transfer
    if (internal->period_time == 0)
        internal->period_time = internal->buffer_time/4;

    err = snd_pcm_hw_params_set_period_time_near(m_pcm, params, &(internal->period_time), 0);
    if (err < 0){
        printf("snd_pcm_hw_params_set_period_time_near() failed.\n");
        return err;
    }

    // set the length of the hardware sample buffer in microseconds
    // some plug devices have very high minimum periods; don't allow a buffer
    // size small enough that it's ~ guaranteed to skip
    if(internal->buffer_time<internal->period_time*3)
        internal->buffer_time=internal->period_time*3;
    err = snd_pcm_hw_params_set_buffer_time_near(m_pcm, params, &(internal->buffer_time), 0);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_buffer_time_near() failed.\n");
        return err;
    }
    */

    /* Set buffer size and period size manually for SPDIF */
    //if (self->passthrough) {
    snd_pcm_uframes_t buffer_size = spdif::ac3BufferSize;
    snd_pcm_uframes_t period_size = spdif::ac3PeriodSize;

    err = snd_pcm_hw_params_set_buffer_size_near(m_pcm, params, &buffer_size);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_buffer_size_near() failed.\n");
        return false;
    }
    err = snd_pcm_hw_params_set_period_size_near(m_pcm, params, &period_size, NULL);
    if (err < 0) {
        printf("snd_pcm_hw_params_set_period_size_near() failed.\n");
        return false;
    }
    //}

    /* commit the params structure to the hardware via ALSA */
    err = snd_pcm_hw_params(m_pcm, params);
    if (err < 0){
        printf("snd_pcm_hw_params() failed.\n");
        return false;
    }

    return true;
}

bool AlsaSink::setSwParams()
{
    snd_pcm_sw_params_t *params;
    snd_pcm_sw_params_alloca(&params);

    /* fetch the current software parameters */
    int err = snd_pcm_sw_params_current(m_pcm, params);
    if (err < 0) {
        printf("snd_pcm_sw_params_current() failed.\n");
        return false;
    }

    err = snd_pcm_sw_params_set_start_threshold(m_pcm, params, spdif::ac3BufferSize);
    if (err < 0) {
        printf("snd_pcm_sw_params_set_start_threshold() failed.\n");
        return false;
    }

    /* allow the transfer when at least period_size samples can be processed */
    err = snd_pcm_sw_params_set_avail_min(m_pcm, params, spdif::ac3PeriodSize);
    if (err < 0) {
        printf("snd_pcm_sw_params_set_avail_min() failed.\n");
        return false;
    }

    /* commit the params structure to ALSA */
    err = snd_pcm_sw_params(m_pcm, params);
    if (err < 0) {
        printf("snd_pcm_sw_params() failed.\n");
        return false;
    }

    return true;
}

bool AlsaSink::write(const char* samples, uint32_t bytesCount)
{
    uint_32 frameCount = bytesCount / 4; // 4 = frameSize = 2 channels * 2 byte per sample
    char* ptr = (char*)samples;

    while (frameCount > 0) {
        int ret = snd_pcm_writei(m_pcm, ptr, frameCount);
        if (ret == -EAGAIN || ret == -EINTR) {
            continue;
        }

        if (ret < 0) {
            if (!recover(ret)) {
                return false;
            }
            continue;
        }

        frameCount -= ret;
        ptr += ret * 4;
    }

    return true;
}

bool AlsaSink::recover(int err)
{
    // underrun
    if (err == -EPIPE) {
        err = snd_pcm_prepare(m_pcm);
        if (err < 0) {
            // Cannot recover from underrun
            return false;
        }
    } else if (err == -ESTRPIPE) {
        // wait until suspend flag clears
        while ((err = snd_pcm_resume(m_pcm)) == -EAGAIN)
            sleep(1);

        if (err < 0) {
            err = snd_pcm_prepare(m_pcm);
            if (err < 0) {
                // Cannot recover from suspend
                return false;
            }
        }
        return true;
    }

    // unrecoverable
    return false;
}

void AlsaSink::doAc3Payload(AudioBuffer& buffer)
{
    if (buffer.size() > (spdif::ac3FrameSize - spdif::SpdifAc3Header::size())) {
        LOG_F(WARNING, "Frame too big, droppping it.");
        buffer.clear();
        return;
    }

    auto ac3Data = buffer.acquire(spdif::ac3FrameSize);
    spdif::SpdifAc3Header ac3Header(buffer.data(), buffer.size());

    std::memcpy(ac3Data, &ac3Header, ac3Header.size());
    ac3Data += ac3Header.size();

#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (uint32_t i = 1; i < buffer.size(); i += 2) {
        ac3Data[i-1] = buffer.data()[i];
        ac3Data[i] = buffer.data()[i-1];
    }
    // Check for remaining (single) byte
    if (buffer.size() % 2) {
        ac3Data[buffer.size()-1] = 0;
        ac3Data[buffer.size()] = buffer.data()[buffer.size()-1];
        ac3Data += 1;
    }
#else
    // If we are on big endian, we can just copy
    std::memcpy(ac3Data, buffer.data(), buffer.size());
#endif

    // Zero remaining
    memset(ac3Data, 0, spdif::ac3FrameSize - buffer.size() - (buffer.size()%2));
    buffer.commit(spdif::ac3FrameSize);
}

} // namespace audio
} // namespace coro
