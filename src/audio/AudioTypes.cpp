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

#include "audio/AudioTypes.h"

namespace coro {
namespace audio {

uint8_t size(AudioCodec codec) {
    switch (codec) {
    case AudioCodec::Invalid: return 0;
    case AudioCodec::RawFloat32: return 4;
    default: return 2;
    }
    return 0;
}

uint32_t toInt(SampleRate rate) {
    switch (rate) {
    case SampleRate::Invalid: return 0;
    case SampleRate::RateUnknown: return 0;
    //case SampleRate::Rate16000: return 16000;
    case SampleRate::Rate32000: return 32000;
    case SampleRate::Rate44100: return 44100;
    case SampleRate::Rate48000: return 48000;
    //case SampleRate::Rate88200: return 88200;
    case SampleRate::Rate96000: return 96000;
    //case SampleRate::Rate176400: return 176400;
    case SampleRate::Rate192000: return 192000;
    }
    return 0;
}

uint8_t toInt(ChannelFlags channels) {
    int i = 0;
    if (channels.testFlag(Channels::Mono))      i+=1;
    if (channels.testFlag(Channels::Stereo))    i+=2;
    if (channels.testFlag(Channels::Lfe))       i+=1;
    if (channels.testFlag(Channels::RearStereo))    i+=2;

    return i;
}

} // namespace audio
} // namespace coro
