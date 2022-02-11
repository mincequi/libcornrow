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

#include "airplay/AirplaySource.h"
#include "airplay/AirplaySourcePrivate.h"

namespace coro {
namespace airplay {

using namespace std::placeholders;

AirplaySource::AirplaySource(const AirplaySource::Config& config)
    : d(new AirplaySourcePrivate(*this, config)) {
	d->appSink.setStartCallback([this]() {
		pushConfig( { audio::AudioCodec::RawInt16, audio::SampleRate::Rate44100, audio::Channels::Stereo } );
		start();
	});
    d->appSink.setProcessCallback(std::bind(&AirplaySource::pushBuffer, this, _1));
    d->appSink.setStopCallback(std::bind(&AirplaySource::stop, this));
}

AirplaySource::~AirplaySource() {
    delete d;
}

const char* AirplaySource::name() const {
    return "AirplaySource";
}

void AirplaySource::setReadyCallback(ReadyCallback callback) {
    d->audioReceiver.setReadyCallback(callback);
}

} // namespace airplay
} // namespace coro
