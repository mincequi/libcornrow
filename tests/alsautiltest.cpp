#include <iostream>

#include "../src/AlsaUtil.h"

int main(void)
{
    GstDsp::AlsaUtil alsaUtil;
    auto outputDevices = alsaUtil.enumerateDevices();

    for (const auto& dev : outputDevices) {
        std::cout << dev.name << std::endl;
    }

    return 0;
}
