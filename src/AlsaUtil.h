#pragma once

#include <DeviceInfo.h>

namespace GstDsp
{

class AlsaUtil
{
public:
    AlsaUtil();

    void enumerateDevices();

private:
    AudioDeviceInfos m_devices;
};

} // namespace GstDsp
