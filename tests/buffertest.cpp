#include <assert.h>
#include <iostream>
#include <queue>
#include <vector>

#include <coro/audio/AudioBuffer.h>

using namespace std;
using namespace coro::audio;

class Encoder
{
public:
    coro::audio::AudioBuffer process(coro::audio::AudioBuffer&& buffer)
    {
        m_queue.push(std::move(buffer));
        coro::audio::AudioBuffer newBuffer("1234", 4);
        return newBuffer;
    }

    std::queue<coro::audio::AudioBuffer> m_queue;
};

int main()
{
    coro::audio::AudioBuffer buffer("5678", 4);
    std::cout << "buffer before: " << buffer.data() << std::endl;

    Encoder encoder;
    //buffer = encoder.process(buffer);
    //buffer = std::move(AudioBuffer("1234", 4));

    std::cout << "buffer after: " << buffer.data() << std::endl;

    return 0;
}
