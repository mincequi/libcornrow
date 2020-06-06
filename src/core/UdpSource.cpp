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

#include <coro/core/UdpSource.h>
#include <coro/audio/Source.h>

#include <boost/bind.hpp>

namespace coro {
namespace core {

using namespace asio;
namespace ph = asio::placeholders;
using namespace coro::audio;

UdpSource::UdpSource() :
    UdpSource(Config())
{
}

UdpSource::UdpSource(const Config& config) :
    m_config(config),
    m_socket(m_ioService),
    m_localEndpoint(ip::address::from_string("0.0.0.0"), config.port),
    m_timeout(m_ioService, asio::chrono::seconds(1)),
    m_buffer(config.prePadding + m_config.mtu * 7)
{
    m_socket.open(m_localEndpoint.protocol());
    m_socket.set_option(ip::udp::socket::reuse_address(true));
    m_socket.bind(m_localEndpoint);
    asio::error_code ec;
    m_socket.set_option(ip::multicast::join_group(ip::address::from_string("239.255.77.77")), ec);
    if (ec) {
        LOG_F(ERROR, "Unable to join multicast group");
        return;
    }
    doReceive();
    startTimer();
    _start();
}

UdpSource::~UdpSource()
{
    Source::stop();
    m_ioService.stop();
    m_thread.join();
}

uint16_t UdpSource::port() const
{
    return m_localEndpoint.port();
}

void UdpSource::_start()
{
    m_thread = std::thread([this]() {
        m_ioService.run();
    });
}

void UdpSource::startTimer()
{
    m_timeout.expires_at(m_timeout.expiry() + chrono::seconds(1));
    m_timeout.async_wait(std::bind(&UdpSource::onTimeout, this));
}

void UdpSource::doReceive()
{
    if (m_isReceiving) {
        return;
    }
    m_isReceiving = true;

    m_buffer.acquire(m_config.prePadding + m_config.mtu);
    m_buffer.commit(m_config.prePadding + m_config.mtu);
    m_socket.async_receive_from(
                asio::buffer(m_buffer.data()+m_config.prePadding, m_config.mtu),
                m_remoteEndpoint,
                boost::bind(&UdpSource::onReceive, this, ph::bytes_transferred));
}

void UdpSource::onReceive(std::size_t bytesTransferred)
{
    if (m_previousBytesTransferred != bytesTransferred) {
        LOG_F(INFO, "Transfer size changed: %lu", bytesTransferred);
        m_previousBytesTransferred = bytesTransferred;
    }
    m_isReceiving = false;

    if (!Source::isStarted()) {
        Source::setReady(true);
        if (!Source::isStarted()) {
            LOG_F(1, "%s not started. Will drop buffer.", name());
            m_buffer.clear();
            return;
        }
        LOG_F(INFO, "%s restarted", name());
    }

    ++m_bufferCount;
    m_buffer.trimFront(m_config.prePadding);
    m_buffer.shrink(bytesTransferred);

    process( AudioConf { audio::AudioCodec::Unknown, SampleRate::RateUnknown, ChannelFlags::Any }, m_buffer);

    doReceive();
}

void UdpSource::onTimeout()
{
    startTimer();
    doReceive();

    if (!Source::isStarted()) {
        return;
    }

    LOG_F(1, "buffer count: %d", m_bufferCount);
    // 5 time outs without buffer reception, stop.
    if (m_bufferCount <= -5) {
        LOG_F(INFO, "%s timed out. No buffers received for a while.", name());
        m_bufferCount = 0;
        Source::stop();
        return;
    }

    // No buffers received since last timeout.
    if (m_bufferCount <= 0) {
        --m_bufferCount;
    } else {
        m_bufferCount = 0;
    }
}

} // namespace core
} // namespace coro
