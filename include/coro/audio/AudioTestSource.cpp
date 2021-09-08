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

#include <coro/audio/AudioTestSource.h>
#include <coro/core/Mainloop.h>

namespace coro {
namespace audio {

template <AudioCodec AC, SampleRate SR, Channels C>
AudioTestSource<AC, SR, C>::AudioTestSource(const Config& config)
    : m_config(config),
      m_buffer(core::Buffer::create(m_config.numFramesPerBuffer *
                                    toInt(m_audioConfig.channels) *
                                    size(m_audioConfig.codec))) {
    m_buffer->audioConf() = m_audioConfig;
}

template <AudioCodec AC, SampleRate SR, Channels C>
AudioTestSource<AC, SR, C>::~AudioTestSource() {
}

template <AudioCodec AC, SampleRate SR, Channels C>
const char* AudioTestSource<AC, SR, C>::name() const {
    return "AudioTestSource";
}

template <AudioCodec AC, SampleRate SR, Channels C>
void AudioTestSource<AC, SR, C>::onStart() {
    for (uint32_t i = 0; i < m_config.numBuffers; ++i) {
        process(m_buffer);
    }
    stop();
}

template <AudioCodec AC, SampleRate SR, Channels C>
void AudioTestSource<AC, SR, C>::onStop() {
    core::Mainloop::instance().stop();
}

template <AudioCodec AC, SampleRate SR, Channels C>
void AudioTestSource<AC, SR, C>::onProcess(core::BufferPtr& buffer) {
    // Fill buffer with whatever
    auto bufSize = m_config.numFramesPerBuffer *
            toInt(m_audioConfig.channels) *
            size(m_audioConfig.codec);
    auto data = buffer->acquire(bufSize);
    for (size_t i = 0; i < buffer->capacity(); ++i)
        data[i] = rand() % 256;

    buffer->commit(bufSize);
}

} // namespace audio
} // namespace coro
