/*
 * Copyright (C) 2022 Manuel Weichselbaumer <mincequi@web.de>
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

#include <coro/audio/AudioDeviceInfo.h>
#include <coro/audio/AudioNode.h>

#include <list>
#include <string>

#include <portaudiocpp/BlockingStream.hxx>

typedef struct _snd_pcm snd_pcm_t;

namespace coro {
namespace audio {

class PortAudioSink : public AudioNode {
public:
	static constexpr std::array<std::pair<core::Cap, core::Cap>, 1> caps() {
		return {{{ { AudioCapRaw<int16_t> {} },
				   { core::NoCap {} }
			   }}};
	}

	PortAudioSink();
	virtual ~PortAudioSink();

	static std::list<AudioDeviceInfo> outputDevices();

private:
	const char* name() const override;
	void onStart() override;
	void onStop() override;
	void onConfig(const AudioConf& config) override;
	void onProcess(core::BufferPtr& buffer) override;

	AudioConf  m_config;
	portaudio::BlockingStream m_stream;
};

} // namespace audio
} // namespace coro
