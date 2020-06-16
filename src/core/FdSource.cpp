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

#include <coro/core/FdSource.h>

#include "core/MainloopPrivate.h"

#include <boost/asio/posix/stream_descriptor.hpp>
#include <loguru/loguru.hpp>

namespace coro {
namespace core {

using namespace boost::asio;
using namespace std::placeholders;

class FdSourcePrivate {
public:
    FdSourcePrivate(FdSource& _p) :
        p(_p),
        ioContext(MainloopPrivate::instance().ioContext),
        streamDescriptor(ioContext) {
    }

    void doWait() {
        streamDescriptor.async_wait(posix::stream_descriptor::wait_read,
                                    std::bind(&FdSourcePrivate::onWaited, this, _1));
    }

    void onWaited(const boost::system::error_code& ec) {
        if (ec) {
            LOG_F(WARNING, "%s", ec.message().c_str());
            p.stop();
            return;
        }

        // We iteratively read bytes as long as there are bytes available.
        boost::asio::posix::stream_descriptor::bytes_readable command(true);
        size_t bytesReadable = 0;
        size_t rounds = 0;
        do {
            // Increment internal buffer counter.
            ++bufferCount;

            // Make room for our data.
            buffer.acquire(blockSize, &p);
            buffer.commit(blockSize);

            // Read bytes into buffer.
            size_t bytesRead = 0;
            try {
                bytesRead = streamDescriptor.read_some(boost::asio::buffer(buffer.data(), buffer.size()));
            } catch (...) {
                LOG_F(INFO, "Connection closed by peer.");
                p.stop();
                return;
            }

            // Push buffer into pipeline.
            buffer.shrink(bytesRead);
            p.pushBuffer(audio::AudioConf { audio::AudioCodec::Unknown,
                                            audio::SampleRate::RateUnknown,
                                            audio::ChannelFlags::Any }, buffer);

            // Check if still bytes readable
            streamDescriptor.io_control(command);
            bytesReadable = command.get();
            //LOG_F(2, "Bytes read: %zu, still readable: %zu", bytesRead, bytesReadable);
            ++rounds;
        } while(bytesReadable);

        if (rounds > 1) {
            LOG_F(2, "Sequential reads: %zu", rounds);
        }

        doWait();
    }

    FdSource& p;
    io_context& ioContext;
    posix::stream_descriptor streamDescriptor;
    uint16_t    blockSize = 0;

    int                 bufferCount = 0;
    audio::AudioBuffer  buffer;
    float               previousBytesTransferred = 0.0f;
};

FdSource::FdSource() :
    d(new FdSourcePrivate(*this))
{
}

FdSource::~FdSource()
{
    // When we close, we have to poll for pending events in queue.
    // Otherwise signal handler on deleted object will be called.
    d->streamDescriptor.close();
    d->ioContext.poll();

    delete d;
}

void FdSource::init(int fd, uint16_t blockSize)
{
    // Close previously opened FD.
    if (d->streamDescriptor.is_open()) {
        d->streamDescriptor.close();
    }

    // If invalid FD give, do nothing.
    if (fd < 0) {
        return;
    }

    // Assign params
    d->streamDescriptor.assign(fd);
    d->blockSize = blockSize;

    // Start reception
    d->doWait();

    // If our io_context ran out of work, we have to restart it.
    d->ioContext.restart();
}

const char* FdSource::name() const
{
    return "FdSource";
}

} // namespace core
} // namespace coro
