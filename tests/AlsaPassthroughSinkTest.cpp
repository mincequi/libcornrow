#include <AlsaPassthroughSink.h>

#include <iostream>

int main(int argc, char** argv)
{
    AlsaPassthroughSink sink;
    auto devices = sink.enumerateDevices();

    for (const DeviceDescriptor& device : devices) {
        if (device.deviceType == DeviceType::Spdif && !device.streamTypes.empty()) {
            std::cout << device << std::endl;
        }
    }

    return 0;
}
