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

#include "RtspServerPrivate.h"

#include "core/MainloopPrivate.h"

#include <coro/rtsp/RtspMessage.h>
#include <coro/rtsp/RtspMessageHandler.h>

#include <functional>
#include <loguru/loguru.hpp>

using namespace std::placeholders;

namespace coro {
namespace rtsp {

RtspServerPrivate::RtspServerPrivate(RtspMessageHandler& _handler, uint16_t port) :
    handler(_handler),
    ioContext(core::MainloopPrivate::instance().ioContext),
    acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(ioContext)
{
    LOG_F(INFO, "listening on port: %u", acceptor.local_endpoint().port());

    doAccept();
}

void RtspServerPrivate::doAccept()
{
    socket.close();
    acceptor.async_accept(socket, std::bind(&RtspServerPrivate::onAccepted, this, _1));
}

void RtspServerPrivate::onAccepted(const boost::system::error_code& error)
{
    if (error) {
        LOG_F(ERROR, "error: %s", error.message().c_str());
        doAccept();
        return;
    }

    doReceive();
}

void RtspServerPrivate::doReceive()
{
    socket.async_receive(boost::asio::buffer(receiveBuffer), std::bind(&RtspServerPrivate::onReceived, this, _1, _2));
}

void RtspServerPrivate::onReceived(const boost::system::error_code& error, size_t bytes)
{
    if (error) {
        LOG_F(WARNING, "error: %s", error.message().c_str());
        doAccept();
        return;
    }

    //auto address = socket.local_endpoint().address();
    //LOG_F(INFO, "buffer received: %s, at: %s", _buffer.c_str(), address.to_string().c_str());

    // Prepare response
    std::string _buffer(receiveBuffer.data(), bytes);
    auto request = RtspMessage::deserializeRequest(_buffer);
    auto response = RtspMessage::createResponse(request.header("CSeq"));
    handler.onMessage(request, &response, socket.local_endpoint().address().to_v4().to_uint());

    doSend(response);
}

void RtspServerPrivate::doSend(const RtspMessage& response)
{
    sendBuffer = response.serialize();
    socket.async_send(boost::asio::buffer(sendBuffer), std::bind(&RtspServerPrivate::onSent, this, _1, _2));

    //LOG_F(INFO, "send buffer: %s", _buffer.c_str());
}

void RtspServerPrivate::onSent(const boost::system::error_code& error, size_t bytes)
{
    if (error) {
        LOG_F(WARNING, "error: %s", error.message().c_str());
        return;
    }

    LOG_F(INFO, "buffer sent: %zu", bytes);

    doReceive();
}

} // namespace rtsp
} // namespace coro
