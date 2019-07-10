#pragma once

#include <string>
#include <vector>
#include "AEChannelInfo.h"
#include "AEStreamInfo.h"
#include "Types.h"

struct AudioDeviceInfo
{
    std::string deviceName;	// the driver device name
    std::string m_displayName;	// the friendly display name
    std::string m_displayNameExtra;	// additional display name info, ie, monitor name from ELD
    GstDsp::AudioDeviceType deviceType;	// the device type, PCM, SPDIF or HDMI
    AudioChannelLayout channels;		// the channels the device is capable of rendering
    std::vector<std::uint32_t> sampleRates;	// the samplerates the device is capable of rendering
    std::vector<AudioSampleFormat> sampleFormat;	// the dataformats the device is capable of rendering
    std::vector<StreamInfo::StreamType> streamTypes;

    bool m_wantsIECPassthrough;           // if sink supports passthrough encapsulation is done when set to true

    static std::string DeviceTypeToString(enum AudioDeviceType deviceType);
};
std::ostream& operator<<(std::ostream& ss, const AudioDeviceInfo& info);

using AudioDeviceInfos = std::vector<AudioDeviceInfo>;
