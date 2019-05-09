/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <sstream>
#include "AEDeviceInfo.h"
#include "AEUtil.h"

std::ostream& operator<<(std::ostream& ss, const AudioDeviceDescriptor& info)
{
    ss << "deviceName      : " << info.deviceName << '\n';
    ss << "displayName     : " << info.m_displayName << '\n';
    ss << "displayNameExtra: " << info.m_displayNameExtra << '\n';
    ss << "deviceType      : " << AudioDeviceDescriptor::DeviceTypeToString(info.deviceType) + '\n';
    ss << "channels        : " << (std::string)info.channels << '\n';

    ss << "sampleRates     : ";
    for (auto it = info.sampleRates.begin(); it != info.sampleRates.end(); ++it)
    {
        if (it != info.sampleRates.begin())
            ss << ", ";
        ss << *it;
    }
    ss << '\n';

    ss << "dataFormats     : ";
    for (auto it = info.sampleFormat.begin(); it != info.sampleFormat.end(); ++it)
    {
        if (it != info.sampleFormat.begin())
            ss << ", ";
        ss << (*it);
    }
    ss << '\n';

    ss << "streamTypes     : ";
    for (auto it = info.streamTypes.begin(); it != info.streamTypes.end(); ++it)
    {
        if (it != info.streamTypes.begin())
            ss << ", ";
        ss << CAEUtil::StreamTypeToStr(*it);
    }

    ss << '\n';
    return ss;
}

std::string AudioDeviceDescriptor::DeviceTypeToString(enum DeviceType deviceType)
{
    switch (deviceType) {
    case DeviceType::Pcm   : return "PCM"  ; break;
    case DeviceType::Spdif : return "SPDIF"; break;
    case DeviceType::Hdmi  : return "HDMI" ; break;
    }
    return "";
}
