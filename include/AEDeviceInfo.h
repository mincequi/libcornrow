/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>
#include <vector>
#include "AEChannelInfo.h"
#include "AEStreamInfo.h"

enum class DeviceType {
    Pcm,
    Spdif,
    Hdmi
};

struct AudioDeviceDescriptor
{
public:
    std::string deviceName;	// the driver device name
    std::string m_displayName;	// the friendly display name
    std::string m_displayNameExtra;	// additional display name info, ie, monitor name from ELD
    DeviceType deviceType;	// the device type, PCM, SPDIF or HDMI
    AudioChannelLayout channels;		// the channels the device is capable of rendering
    std::vector<std::uint32_t> sampleRates;	// the samplerates the device is capable of rendering
    std::vector<AudioSampleFormat> sampleFormat;	// the dataformats the device is capable of rendering
    std::vector<StreamInfo::StreamType> streamTypes;

    bool m_wantsIECPassthrough;           // if sink supports passthrough encapsulation is done when set to true

    friend std::ostream& operator<<(std::ostream& ss, const AudioDeviceDescriptor& info);
    static std::string DeviceTypeToString(enum DeviceType deviceType);
};

using DeviceDescriptors = std::vector<AudioDeviceDescriptor>;
