#include <AlsaPassthroughSink.h>

#include <iostream>

int main(int argc, char** argv)
{
    AlsaPassthroughSink sink;
    auto devices = sink.enumerateDevices();

    for (const auto& device : devices) {
        std::cout << device << std::endl;
    }

    return 0;
}
