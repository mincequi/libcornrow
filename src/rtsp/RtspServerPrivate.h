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

#include <functional>
#include <map>

#include <boost/array.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace coro {
namespace rtsp {

class RtspMessage;
class RtspMessageHandler;

class RtspServerPrivate
{
public:
    RtspServerPrivate(RtspMessageHandler& _handler, uint16_t port);

    void doAccept();
    void onAccepted(const boost::system::error_code& error);
    void doReceive();
    void onReceived(const boost::system::error_code& error, size_t bytes);
    void doSend(const RtspMessage& response);
    void onSent(const boost::system::error_code& error, size_t bytes);

    RtspMessageHandler& handler;
    boost::asio::io_context& ioContext;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    boost::array<char, 2048> buffer;
};

} // namespace rtsp
} // namespace coro
