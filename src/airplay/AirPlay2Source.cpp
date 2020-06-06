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

#include <coro/airplay/AirPlay2Source.h>

namespace coro {
namespace airplay {

class AirPlay2SourcePrivate
{

};

AirPlay2Source::AirPlay2Source()
    : d(new AirPlay2SourcePrivate())
{
}

AirPlay2Source::~AirPlay2Source()
{
    delete d;
}

/*
AudioConf AirPlay2Source::doProcess(const AudioConf&, AudioBuffer& buffer)
{
    return { audio::AudioCodec::RawInt16,
                SampleRate::Rate44100,
                audio::Channels::Stereo };
}
*/

const char* AirPlay2Source::name() const
{
    return "AirPlay2Source";
}

void AirPlay2Source::doPoll()
{
}

} // namespace airplay
} // namespace coro
