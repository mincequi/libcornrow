/*
 * Copyright (C) 2021 Manuel Weichselbaumer <mincequi@web.de>
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

#include <coro/core/TcpClientSink.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <loguru/loguru.hpp>

#include <coro/core/Mainloop.h>

#include "core/MainloopPrivate.h"

namespace coro {
namespace core {

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;

class TcpClientSinkPrivate {
public:
    TcpClientSinkPrivate(const TcpClientSink::Config& config) :
        config(config),
        mainloop(Mainloop::instance()),
        socket(MainloopPrivate::instance().ioContext),
        timer(MainloopPrivate::instance().ioContext) {

        // No deadline is required until the first socket operation is started. We
        // set the deadline to positive infinity so that the actor takes no action
        // until a specific deadline is set.
        timer.expires_at(boost::posix_time::pos_infin);

        // Start the persistent actor that checks for deadline expiry.
        checkTimer();
    }

    bool isConnected() {
        return socket.is_open();
    }

    bool connectAsync(boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(8)) {
        boost::system::error_code ec;

        // Resolve the host name and service to a list of endpoints.
        tcp::resolver::results_type endpoints = tcp::resolver(MainloopPrivate::instance().ioContext).resolve(config.host, std::to_string(config.port), ec);
        if (ec) {
            LOG_S(WARNING) << ec.message() << " when connecting host: " << config.host;
            return false;
        }

        // Set a deadline for the asynchronous operation. As a host name may
        // resolve to multiple endpoints, this function uses the composed operation
        // async_connect. The deadline applies to the entire operation, rather than
        // individual connection attempts.
        timer.expires_from_now(timeout);

        // Set up the variable that receives the result of the asynchronous
        // operation. The error code is set to would_block to signal that the
        // operation is incomplete. Asio guarantees that its asynchronous
        // operations will never fail with would_block, so any other value in
        // ec indicates completion.
        ec = boost::asio::error::would_block;

        // Start the asynchronous operation itself. The boost::lambda function
        // object is used as a callback and will update the ec variable when the
        // operation completes. The blocking_udp_client.cpp example shows how you
        // can use boost::bind rather than boost::lambda.
        boost::asio::async_connect(socket, endpoints, var(ec) = _1);

        // Block until the asynchronous operation has completed.
        do mainloop.runOne(); while (ec == boost::asio::error::would_block);

        // Determine whether a connection was successfully established. The
        // deadline actor may have had a chance to run and close our socket, even
        // though the connect operation notionally succeeded. Therefore we must
        // check whether the socket is still open before deciding if we succeeded
        // or failed.
        if (ec || !socket.is_open()) {
            socket.close();
            LOG_S(WARNING) << ec.message() << " when connecting host: " << config.host;
            return false;
        }

        return true;
    }

    void connect() {
        // Resolve the host name and service to a list of endpoints.
        auto results = tcp::resolver(MainloopPrivate::instance().ioContext).resolve(config.host, std::to_string(config.port));
        if (results.begin() == results.end()) {
            LOG_S(WARNING) << "Host not resolvable: " << config.host;
            return;
        }

        boost::asio::async_connect(socket, results, [](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            if (ec) {
                LOG_S(WARNING) << "Error: " << ec.message() << " when connecting endpoint: " << endpoint;
            }
        });
    }

    bool write(const core::BufferPtr& buffer,
               boost::posix_time::time_duration timeout = boost::posix_time::seconds(1)) {
        // Set a deadline for the asynchronous operation. Since this function uses
        // a composed operation (async_write), the deadline applies to the entire
        // operation, rather than individual writes to the socket.
        timer.expires_from_now(timeout);

        // Set up the variable that receives the result of the asynchronous
        // operation. The error code is set to would_block to signal that the
        // operation is incomplete. Asio guarantees that its asynchronous
        // operations will never fail with would_block, so any other value in
        // ec indicates completion.
        boost::system::error_code ec = boost::asio::error::would_block;

        // Start the asynchronous operation itself. The boost::lambda function
        // object is used as a callback and will update the ec variable when the
        // operation completes. The blocking_udp_client.cpp example shows how you
        // can use boost::bind rather than boost::lambda.
        boost::asio::async_write(socket, boost::asio::buffer(buffer->data(), buffer->size()), var(ec) = _1);

        // Block until the asynchronous operation has completed.
        do mainloop.runOne(); while (ec == boost::asio::error::would_block);

        if (ec) {
            return false;
        }

        return true;
    }

    void checkTimer() {
        // Check whether the deadline has passed. We compare the deadline against
        // the current time since a new asynchronous operation may have moved the
        // deadline before this actor had a chance to run.
        if (timer.expires_at() <= deadline_timer::traits_type::now()) {
            // The deadline has passed. The socket is closed so that any outstanding
            // asynchronous operations are cancelled. This allows the blocked
            // connect() or write() functions to return.
            boost::system::error_code ec;
            socket.close(ec);

            // There is no longer an active deadline. The expiry is set to positive
            // infinity so that the actor takes no action until a new deadline is set.
            timer.expires_at(boost::posix_time::pos_infin);
        }

        // Put the actor back to sleep.
        timer.async_wait(bind(&TcpClientSinkPrivate::checkTimer, this));
    }

    const TcpClientSink::Config& config;
    Mainloop& mainloop;
    tcp::socket socket;
    deadline_timer timer;
    core::BufferPtr buffer;
};

TcpClientSink::TcpClientSink(const TcpClientSink::Config& config) :
    d(new TcpClientSinkPrivate(config)) {
}

TcpClientSink::~TcpClientSink() {
    delete d;
}

bool TcpClientSink::isStarted() const {
    return d->isConnected();
}

const char* TcpClientSink::name() const {
    return "TcpClientSink";
}

void TcpClientSink::onStart() {
    LOG_F(INFO, "%s started", name());
    d->connect();
}

void TcpClientSink::onStop() {
    LOG_F(INFO, "%s stopped", name());
    d->socket.close();
}

void TcpClientSink::onProcess(core::BufferPtr& buffer) {
    if (!d->isConnected()) {
        if (!d->connectAsync()) {
            return;
        }
    }

    boost::system::error_code ec;
    boost::asio::write(d->socket, boost::asio::buffer(buffer->data(), buffer->size()), ec);
    if (ec) {
        LOG_S(WARNING) << ec.message();
        d->socket.close();
    }
}

} // namespace core
} // namespace coro
