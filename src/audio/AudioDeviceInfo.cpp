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

#include <coro/audio/AudioDeviceInfo.h>

#include <iostream>

namespace coro {
namespace audio {

std::ostream& operator<<(std::ostream& out, AudioDeviceType t) {
	switch (t) {
	case AudioDeviceType::Default:
		return out << "Default";
	case AudioDeviceType::Hdmi:
		return out << "HDMI";
	case AudioDeviceType::Spdif:
		return out << "SPDIF";
	case AudioDeviceType::Invalid:
		return out << "INVALID";
	}

	return out;
}

AudioDeviceInfo::AudioDeviceInfo(const std::string& _name, const std::string& _desc)
	: name(_name),
	  desc(_desc) {
	if (name.substr(0, 4) == "hdmi") {
		type = AudioDeviceType::Hdmi;
	} else if (name.substr(0, 6) == "iec958" || name.substr(0, 5) == "spdif") {
		type = AudioDeviceType::Spdif;
	} else if (name.substr(0, 7) == "default") {
		type = AudioDeviceType::Default;
	} else {
		type = AudioDeviceType::Invalid;
	}
}

std::ostream& operator<<(std::ostream& out, const AudioDeviceInfo& info) {
	return out << "type: " << info.type << ", name: " << info.name << ", desc: " << info.desc << ", maxOutputChannels: " << info.maxChannels;
}

} // namespace audio
} // namespace coro
