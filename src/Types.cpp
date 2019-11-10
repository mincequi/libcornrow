#include "Types.h"

namespace coro
{

AudioDeviceInfo::AudioDeviceInfo(const std::string& _name, const std::string& _desc)
    : name(_name),
      desc(_desc)
{
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

} // namespace GstDsp
