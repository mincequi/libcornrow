#include <iostream>

#include <gstdsp/AlsaUtil.h>

int main(void)
{
    GstDsp::AlsaUtil alsaUtil;
    auto outputDevices = alsaUtil.outputDevices();

    for (const auto& dev : outputDevices) {
        std::cout << dev.name << std::endl;
    }

    return 0;
}
