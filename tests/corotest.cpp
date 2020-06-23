//#include <coro/core/Buffer.h>

#include <algorithm>
#include <assert.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

//using namespace coro::core;

class Buffer
{
public:
    Buffer(std::string x) : txt(x) {}

    Buffer(Buffer&& other) : txt(std::move(other.txt)) {}
    Buffer& operator=(Buffer&& other) { return *this; }

    // The copy operations are implicitly deleted, but you can
    // spell that out explicitly if you want:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    std::string txt;
};

void onProcess(std::unique_ptr<Buffer>& in)
{
    in->txt = "before1";
    std::unique_ptr<Buffer> mine = std::move(in);
    //in = std::make_unique<Buffer>(Buffer("before2"));
}

void process(std::unique_ptr<Buffer>& in)
{
    onProcess(in);

    if (!in) {
        in = std::make_unique<Buffer>(Buffer("after"));
    }
}

int main()
{
    std::unique_ptr<Buffer> buffer(new Buffer("before"));

    std::cout << "buffer before: " << buffer->txt << std::endl;
    process(buffer);
    std::cout << "buffer after: " << buffer->txt << std::endl;

    std::vector<int> ints;
    for (int i = 0; i < 4096; ++i) {
        ints.push_back(i);

        static size_t cap = 0;
        if (cap != ints.capacity()) {
            std::cout << "capacity: " << ints.capacity() << std::endl;
            cap = ints.capacity();
        }

        std::vector<int> ints2;
        ints2.reserve(ints.capacity());

        assert(ints.capacity() == ints2.capacity());
    }
}
