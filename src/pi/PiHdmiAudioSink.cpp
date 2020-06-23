#include <coro/pi/PiHdmiAudioSink.h>

extern "C" {
#include <bcm_host.h>
#include <interface/vcos/vcos_types.h>
#include "audioplay.h"
}

#include <assert.h>
#include <cstdint>
#include <cstring>
#include <loguru/loguru.hpp>

namespace coro {
namespace pi {

PiHdmiAudioSink::PiHdmiAudioSink()
{
    bcm_host_init();
}

PiHdmiAudioSink::~PiHdmiAudioSink()
{
}

void PiHdmiAudioSink::stop()
{
    if (m_handle) {
        audioplay_delete(m_handle);
        m_handle = nullptr;
    }
}

audio::AudioConf PiHdmiAudioSink::onProcess(const audio::AudioConf& conf, core::Buffer& buffer)
{
    // If handle is open, but config changed, close device.
    if (m_handle && m_conf != conf) {
        LOG_F(INFO, "Configuration changed. Reopening device");
        audioplay_delete(m_handle);
        m_handle = nullptr;
        m_conf = conf;
    }

    // If handle is closes, open device.
    if (!m_handle) {
        m_conf = conf;
        if (audioplay_create(&m_handle, audio::toInt(m_conf.rate), audio::toInt(m_conf.channels), 16, 10, buffer.size())) {
            LOG_F(ERROR, "Error opening device");
            return audio::AudioConf();
        }
        if (audioplay_set_dest(m_handle, "hdmi")) {
            LOG_F(ERROR, "Error setting output to hdmi");
            return audio::AudioConf();
        }
    }

    auto data = audioplay_get_buffer(m_handle);
    std::memcpy(data, buffer.data(), buffer.size());

    // try and wait for a minimum latency time (in ms) before sending the next packet.
#define CTTW_SLEEP_TIME 10
#define MIN_LATENCY_TIME 20
    while (audioplay_get_latency(m_handle) > (audio::toInt(m_conf.rate) * (MIN_LATENCY_TIME + CTTW_SLEEP_TIME) / 1000)) {
       usleep(CTTW_SLEEP_TIME*1000);
    }

    auto ret = audioplay_play_buffer(m_handle, data, buffer.size());
    assert(ret == 0);



    return conf;
}

const char* PiHdmiAudioSink::name() const
{
    return "PiHdmiAudioSink";
}

} // namespace pi
} // namespace coro
