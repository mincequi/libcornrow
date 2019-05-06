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

std::ostream& operator<<(std::ostream& ss, const CAEDeviceInfo& info)
{
  ss << "m_deviceName      : " << info.m_deviceName << '\n';
  ss << "m_displayName     : " << info.m_displayName << '\n';
  ss << "m_displayNameExtra: " << info.m_displayNameExtra << '\n';
  ss << "m_deviceType      : " << CAEDeviceInfo::DeviceTypeToString(info.m_deviceType) + '\n';
  ss << "m_channels        : " << (std::string)info.m_channels << '\n';

  ss << "m_sampleRates     : ";
  for (auto itt = info.m_sampleRates.begin(); itt != info.m_sampleRates.end(); ++itt)
  {
    if (itt != info.m_sampleRates.begin())
      ss << ',';
    ss << *itt;
  }
  ss << '\n';

  ss << "m_dataFormats     : ";
  for (auto itt = info.m_dataFormats.begin(); itt != info.m_dataFormats.end(); ++itt)
  {
    if (itt != info.m_dataFormats.begin())
      ss << ',';
    ss << CAEUtil::DataFormatToStr(*itt);
  }
  ss << '\n';

  ss << "m_streamTypes     : ";
  for (auto itt = info.m_streamTypes.begin(); itt != info.m_streamTypes.end(); ++itt)
  {
    if (itt != info.m_streamTypes.begin())
      ss << ',';
    ss << CAEUtil::StreamTypeToStr(*itt);
  }
  if (info.m_streamTypes.empty())
    ss << "No passthrough capabilities";
  ss << '\n';

  return ss;
}

std::string CAEDeviceInfo::DeviceTypeToString(enum AEDeviceType deviceType)
{
  switch (deviceType)
  {
    case AE_DEVTYPE_PCM   : return "AE_DEVTYPE_PCM"   ; break;
    case AE_DEVTYPE_IEC958: return "AE_DEVTYPE_IEC958"; break;
    case AE_DEVTYPE_HDMI  : return "AE_DEVTYPE_HDMI"  ; break;
    case AE_DEVTYPE_DP    : return "AE_DEVTYPE_DP"    ; break;
  }
  return "INVALID";
}
