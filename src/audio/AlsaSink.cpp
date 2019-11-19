#include "audio/AlsaSink.h"

#include <alsa/asoundlib.h>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

AlsaSink::AlsaSink()
{
}

AlsaSink::~AlsaSink()
{
}

void AlsaSink::start()
{
    if (m_pcm) {
        return;
    }

    snd_pcm_hw_params_t* hw_params;

    int error = 0;
    if ((error = snd_pcm_open(&m_pcm, "default", SND_PCM_STREAM_PLAYBACK, 0 /*block*/) < 0)) {
        LOG_F(WARNING, "cannot open audio device (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        LOG_F(WARNING, "cannot allocate hardware parameter structure (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_any(m_pcm, hw_params)) < 0) {
        LOG_F(WARNING, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_access(m_pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        LOG_F(WARNING, "cannot set access type (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_format(m_pcm, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        LOG_F(WARNING, "cannot set sample format (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_rate(m_pcm, hw_params, 44100 /*toInt(m_conf.rate)*/, 0)) < 0) {
        LOG_F(WARNING, "cannot set sample rate (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params_set_channels(m_pcm, hw_params, 2 /*toInt(m_conf.channels)*/ )) < 0) {
        LOG_F(WARNING, "cannot set channel count (%s)\n", snd_strerror(error));
        return;
    }
    /*
    if ((error = snd_pcm_hw_params_set_period_size(m_pcm, hw_params, 512, 0)) < 0) {
        LOG_F(WARNING, "cannot set period size (%s)\n", snd_strerror(error));
        return;
    }
    */
    if ((error = snd_pcm_hw_params_set_buffer_size(m_pcm, hw_params, 16384)) < 0) {
        LOG_F(WARNING, "cannot set buffer size (%s)\n", snd_strerror(error));
        return;
    }
    if ((error = snd_pcm_hw_params(m_pcm, hw_params)) < 0) {
        LOG_F(WARNING, "cannot set parameters (%s)\n", snd_strerror(error));
        return;
    }

    snd_pcm_hw_params_free(hw_params);

    if ((error = snd_pcm_prepare(m_pcm)) < 0) {
        LOG_F(WARNING, "cannot prepare audio interface for use (%s)\n", snd_strerror(error));
        return;
    }
}

void AlsaSink::stop()
{
    if (m_pcm) {
        int error = 0;
        if ((error = snd_pcm_drain(m_pcm)) < 0) {
            LOG_F(WARNING, "cannot drain device (%s)\n", snd_strerror(error));
        }
        if ((error = snd_pcm_close(m_pcm)) < 0) {
            LOG_F(WARNING, "cannot close device (%s)\n", snd_strerror(error));
        }
        m_pcm = nullptr;
    }
}

AudioConf AlsaSink::process(const AudioConf& conf, AudioBuffer& buffer)
{
    if (m_conf != conf) {
        m_conf = conf;
        //stop();
        //start();
    }

    int error = snd_pcm_writei(m_pcm, buffer.data(), buffer.size()/conf.frameSize());
    if (error < 0) {
        LOG_F(WARNING, "write to audio interface failed (%s). Recovering...", snd_strerror(error));
        error = snd_pcm_recover(m_pcm, error, 1);
    }
    if (error < 0) {
        LOG_F(WARNING, "write to audio interface failed (%s)", snd_strerror(error));
        m_conf = AudioConf();
    }

    return m_conf;
}

} // namespace audio
} // namespace coro
