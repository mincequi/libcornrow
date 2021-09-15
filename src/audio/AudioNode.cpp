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

#include "audio/AudioNode.h"

#include <core/Caps.h>

namespace coro {
namespace audio {

AudioNode::AudioNode() {
}

AudioNode::~AudioNode() {
}

void AudioNode::onProcess(core::BufferPtr&) {
}

audio::AudioConf AudioNode::onProcess(const audio::AudioConf& conf, core::Buffer& /*buffer*/) {
    return conf;
}

} // namespace audio
} // namespace coro
