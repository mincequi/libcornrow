#include <assert.h>
#include <iostream>
#include <queue>
#include <vector>

#include <coro/core/Buffer.h>

using namespace std;

class Encoder
{
public:
    coro::core::Buffer process(coro::core::Buffer&& buffer)
    {
        m_queue.push(std::move(buffer));
        coro::core::Buffer newBuffer("1234", 4);
        return newBuffer;
    }

    std::queue<coro::core::Buffer> m_queue;
};

int main()
{
    coro::core::Buffer buffer("5678", 4);
    std::cout << "buffer before: " << buffer.data() << std::endl;

    Encoder encoder;
    //buffer = encoder.process(buffer);
    //buffer = std::move(AudioBuffer("1234", 4));

    std::cout << "buffer after: " << buffer.data() << std::endl;

    return 0;
}
