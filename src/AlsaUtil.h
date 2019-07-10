#pragma once

#include "Types.h"

namespace GstDsp
{

class AlsaUtil
{
public:
    AlsaUtil();

    std::list<AudioDeviceInfo> enumerateDevices();

private:
    std::list<AudioDeviceInfo> m_devices;
};

} // namespace GstDsp
