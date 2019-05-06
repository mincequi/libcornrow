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

std::ostream& operator<<(std::ostream& ss, const DeviceDescriptor& info)
{
    ss << "deviceName      : " << info.deviceName << '\n';
    ss << "displayName     : " << info.m_displayName << '\n';
    ss << "displayNameExtra: " << info.m_displayNameExtra << '\n';
    ss << "deviceType      : " << DeviceDescriptor::DeviceTypeToString(info.deviceType) + '\n';
    ss << "channels        : " << (std::string)info.m_channels << '\n';

    ss << "sampleRates     : ";
    for (auto it = info.m_sampleRates.begin(); it != info.m_sampleRates.end(); ++it)
    {
        if (it != info.m_sampleRates.begin())
            ss << ", ";
        ss << *it;
    }
    ss << '\n';

    ss << "dataFormats     : ";
    for (auto it = info.m_dataFormats.begin(); it != info.m_dataFormats.end(); ++it)
    {
        if (it != info.m_dataFormats.begin())
            ss << ", ";
        ss << CAEUtil::DataFormatToStr(*it);
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

std::string DeviceDescriptor::DeviceTypeToString(enum DeviceType deviceType)
{
    switch (deviceType) {
    case DeviceType::Pcm   : return "PCM"  ; break;
    case DeviceType::Spdif : return "SPDIF"; break;
    case DeviceType::Hdmi  : return "HDMI" ; break;
    }
    return "";
}
