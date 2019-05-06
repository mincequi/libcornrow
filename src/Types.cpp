#include "Types.h"

namespace GstDsp
{

DeviceInfo::DeviceInfo(const std::string& _name)
    : name(_name)
{
    if (name.substr(0, 4) == "hdmi") {
        type = DeviceType::Hdmi;
    } else if (name.substr(0, 6) == "iec958" || name.substr(0, 5) == "spdif") {
        type = DeviceType::Spdif;
    } else {
        type = DeviceType::Pcm;
    }
}

} // namespace GstDsp
