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

#include <thread>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <asio/steady_timer.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/multicast.hpp>
#include <asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>
#include <loguru/loguru.hpp>

#include <coro/audio/AudioBuffer.h>
#include <coro/audio/Source.h>

namespace coro {
namespace core {

class UdpSource : public audio::Source
{
public:
    static constexpr std::array<audio::AudioCaps,1> outCaps() {
        return {{ { audio::AudioCodecs::Any,
                    audio::SampleRates::Any,
                    audio::ChannelFlags::Any } }};
    }

    struct Config {
        uint16_t port = 0;
        uint8_t prePadding = 0;
        uint16_t mtu = 1492;
    };

    UdpSource();
    explicit UdpSource(const Config& config);
    ~UdpSource();

    uint16_t port() const;

private:
    void _start();
    void startTimer();
    void doReceive();
    void onReceive(std::size_t bytesTransferred);
    void onTimeout();

    Config m_config;

    asio::io_service          m_ioService;
    asio::ip::udp::socket     m_socket;
    asio::ip::udp::endpoint   m_localEndpoint;
    asio::ip::udp::endpoint   m_remoteEndpoint;
    std::thread         m_thread;
    asio::steady_timer  m_timeout;
    int                 m_bufferCount = 0;
    bool                m_isReceiving = false;
    audio::AudioBuffer  m_buffer;
    size_t              m_previousBytesTransferred = 0;
};

} // namespace core
} // namespace coro
