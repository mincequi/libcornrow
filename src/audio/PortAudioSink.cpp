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

#include "audio/PortAudioSink.h"

#include <loguru/loguru.hpp>
#include <portaudiocpp/AutoSystem.hxx>
#include <portaudiocpp/Device.hxx>
#include <portaudiocpp/Exception.hxx>
#include <portaudiocpp/StreamParameters.hxx>
#include <portaudiocpp/System.hxx>
#include <portaudiocpp/SystemDeviceIterator.hxx>

using namespace portaudio;

namespace coro {
namespace audio {

PortAudioSink::PortAudioSink() {
	try {
		System::initialize();
	}  catch (const PaException& e) {
		LOG_S(ERROR) << e.what();
	}
}

PortAudioSink::~PortAudioSink() {
	try {
		System::terminate();
	}  catch (const PaException& e) {
		LOG_S(ERROR) << e.what();
	}
}

std::list<AudioDeviceInfo> PortAudioSink::outputDevices() {
	std::list<AudioDeviceInfo> out;

	AutoSystem autoSys;
	System& sys = System::instance();
	for (auto it = sys.devicesBegin(); it != sys.devicesEnd(); ++it) {
		if (it->maxOutputChannels() == 0) continue;
		auto info = AudioDeviceInfo{ it->name() };
		info.maxChannels = it->maxOutputChannels();
		out.push_back(info);
	}

	return out;
}

const char* PortAudioSink::name() const {
	return "PortAudioSink";
}

void PortAudioSink::onStart() {
	SampleDataFormat sampleFormat = INVALID_FORMAT;
	switch (m_config.codec) {
	case AudioCodec::RawInt16:
		sampleFormat = INT16;
		break;
	case AudioCodec::RawFloat32:
		sampleFormat = FLOAT32;
		break;
	default:
		break;
	}

	System& sys = System::instance();
	DirectionSpecificStreamParameters outParams(sys.defaultOutputDevice(),
												toInt(m_config.channels),
												sampleFormat,
												true,
												sys.defaultOutputDevice().defaultLowOutputLatency(),
												NULL);
	LOG_S(INFO) << "opening device: " << sys.defaultOutputDevice().name();

	StreamParameters parameters(DirectionSpecificStreamParameters::null(), outParams, toInt(m_config.rate), 256, paClipOff);

	try {
		m_stream.open(parameters);
		m_stream.start();
	}  catch (const PaException& e) {
		LOG_S(ERROR) << e.what();
	}
}

void PortAudioSink::onStop() {
	if (!m_stream.isOpen()) {
		return;
	}

	try {
		LOG_S(INFO) << "closing device";
		m_stream.stop();
		m_stream.close();
	}  catch (const PaException& e) {
		LOG_S(ERROR) << e.what();
	}
}

void PortAudioSink::onConfig(const AudioConf& config) {
	if (m_config != config) {
		m_config = config;
		if (m_stream.isOpen()) {
			onStop();
			onStart();
		}
	}
}

void PortAudioSink::onProcess(core::BufferPtr& buffer) {
	if (!m_stream.isOpen()) {
		return;
	}

	try {
		m_stream.write(buffer->data(), buffer->size() / m_config.frameSize());
	} catch (const PaException& e) {
		LOG_S(WARNING) << e.what();
	}
}

} // namespace audio
} // namespace coro
