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

#pragma once

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>

#include <coro/core/Source.h>

namespace coro {
namespace core {

class UdpSource : public core::Source {
public:
    static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
        return {{{ { NoCap {} }, // in
                   { AnyCap {} } // out
               }}};
    }

    struct Config {
        uint16_t port = 0;
        uint8_t prePadding = 0;
        uint16_t mtu = 1492;
        uint32_t multicastGroup = 0;
    };

    UdpSource();
    explicit UdpSource(const Config& config);
    ~UdpSource();

    uint16_t port() const;

private:
    const char* name() const override;

    void startTimer();
    void doReceive();
    void onReceived(const boost::system::error_code& ec, std::size_t bytesTransferred);
    void onTimeout();

    Config m_config;

    class UdpSourcePrivate* const d;

    // @TODO(mawe): move these to UdpSourcePrivate
    boost::asio::ip::udp::socket     m_socket;
    boost::asio::ip::udp::endpoint   m_localEndpoint;
    boost::asio::steady_timer  m_timeout;
    int             m_bufferCount = 0;
    bool            m_isReceiving = false;
    core::BufferPtr m_buffer;
    float           m_previousBytesTransferred = 0.0f;
};

} // namespace core
} // namespace coro
