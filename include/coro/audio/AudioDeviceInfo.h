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

#include <cstdint>
#include <list>
#include <string>

namespace coro {
namespace audio {

enum class AudioDeviceType : uint8_t {
	Invalid,
	Default,
	Spdif,
	Hdmi
};
std::ostream& operator<<(std::ostream& out, AudioDeviceType t);

struct AudioDeviceInfo {
	explicit AudioDeviceInfo(const std::string& name, const std::string& desc = std::string());

	std::string name;
	uint16_t	maxChannels = 0;
	std::string desc;
	AudioDeviceType type;
};
std::ostream& operator<<(std::ostream& out, const AudioDeviceInfo& info);
using AudioDeviceInfoList = std::list<AudioDeviceInfo>;

}
}
