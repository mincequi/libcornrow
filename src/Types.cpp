#include "Types.h"

namespace GstDsp
{

AudioDeviceInfo::AudioDeviceInfo(const std::string& _name)
    : name(_name)
{
    if (name.substr(0, 4) == "hdmi") {
        type = AudioDeviceType::Hdmi;
    } else if (name.substr(0, 6) == "iec958" || name.substr(0, 5) == "spdif") {
        type = AudioDeviceType::Spdif;
    } else {
        type = AudioDeviceType::Pcm;
    }
}

} // namespace GstDsp
