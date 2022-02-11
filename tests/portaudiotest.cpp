#include <iostream>

#include <coro/audio/PortAudioSink.h>

int main(void) {

	auto outputDevices = coro::audio::PortAudioSink::outputDevices();
    for (const auto& dev : outputDevices) {
		std::cout << dev << std::endl;
    }

    return 0;
}
