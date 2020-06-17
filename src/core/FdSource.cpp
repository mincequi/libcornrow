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

    void doRead() {
        // Make room for our data.
        buffer.acquire(blockSize, &p);
        buffer.commit(blockSize);

        streamDescriptor.async_read_some(boost::asio::buffer(buffer.data(), buffer.size()),
                                         std::bind(&FdSourcePrivate::onRead, this, _1, _2));
    }

    void onRead(const boost::system::error_code& ec, size_t bytesRead) {
        if (ec) {
            LOG_F(WARNING, "%s", ec.message().c_str());
            p.stop();
            return;
        }

        // Push buffer into pipeline.
        buffer.shrink(bytesRead);
        p.pushBuffer(audio::AudioConf { audio::AudioCodec::Unknown,
                                        audio::SampleRate::RateUnknown,
                                        audio::ChannelFlags::Any }, buffer);

        doRead();
    }

    FdSource& p;
    io_context& ioContext;
    posix::stream_descriptor streamDescriptor;
    uint16_t    blockSize = 0;

    int                 bufferCount = 0;
    audio::AudioBuffer  buffer;
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

    // If invalid FD, stop pipeline.
    if (fd < 0) {
        stop();
        return;
    }

    // Assign params
    d->streamDescriptor.assign(fd);
    d->blockSize = blockSize;

    // Start reception
    d->doRead();

    // If our io_context ran out of work, we have to restart it.
    d->ioContext.restart();
}

const char* FdSource::name() const
{
    return "FdSource";
}

} // namespace core
} // namespace coro
