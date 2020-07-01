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

#include <coro/core/Source.h>
#include <coro/core/UdpSource.h>

#include "core/MainloopPrivate.h"

#include <loguru/loguru.hpp>

#include <functional>

namespace coro {
namespace core {

using namespace boost::asio;
using namespace boost::asio::ip;
namespace ph = std::placeholders;

class UdpSourcePrivate
{
public:
    UdpSourcePrivate()
        : ioContext(MainloopPrivate::instance().ioContext)
    {
    }

    boost::asio::io_context& ioContext;
    boost::asio::ip::udp::endpoint remoteEndpoint;
};

UdpSource::UdpSource() :
    UdpSource(Config())
{
}

UdpSource::UdpSource(const Config& config) :
    m_config(config),
    d(new UdpSourcePrivate),
    m_socket(d->ioContext),
    m_localEndpoint(ip::udp::v4(), config.port),
    m_timeout(d->ioContext, std::chrono::seconds(1)),
    m_buffer(config.prePadding + m_config.mtu)
{
    m_socket.open(m_localEndpoint.protocol());
    m_socket.set_option(ip::udp::socket::reuse_address(true));
    m_socket.bind(m_localEndpoint);

    if (config.multicastGroup) {
        boost::system::error_code ec;
        m_socket.set_option(ip::multicast::join_group(ip::address_v4(config.multicastGroup)), ec);
        if (ec) {
            LOG_F(ERROR, "Unable to join multicast group");
            return;
        }
    }

    doReceive();
    //startTimer();
}

UdpSource::~UdpSource()
{
    // When we close, we have to poll for pending events in queue.
    // Otherwise signal handler on deleted object will be called.
    m_socket.close();
    d->ioContext.poll();

    delete d;
    //Source::stop();
}

const char* UdpSource::name() const
{
    return "UdpSource";
}

uint16_t UdpSource::port() const
{
    return m_socket.local_endpoint().port();
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

    m_buffer.acquire(m_config.prePadding + m_config.mtu, this);
    m_buffer.commit(m_config.prePadding + m_config.mtu);
    m_socket.async_receive_from(
                buffer(m_buffer.data()+m_config.prePadding, m_config.mtu),
                d->remoteEndpoint,
                std::bind(&UdpSource::onReceived, this, ph::_1, ph::_2));
}

void UdpSource::onReceived(const boost::system::error_code& ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOG_F(INFO, "%s", ec.message().c_str());
        return;
    }

    if (fabs((m_previousBytesTransferred - bytesTransferred) / bytesTransferred) > 0.125f) {
        LOG_F(1, "Transfer size changed: %zu", bytesTransferred);
    }
    m_previousBytesTransferred = bytesTransferred;
    m_isReceiving = false;

    ++m_bufferCount;
    m_buffer.trimFront(m_config.prePadding);
    m_buffer.shrink(bytesTransferred);

    pushBuffer(audio::AudioConf { audio::AudioCodec::Unknown,
                                  audio::SampleRate::RateUnknown,
                                  audio::ChannelFlags::Any }, m_buffer);

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
