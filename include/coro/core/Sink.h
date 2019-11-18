#pragma once

#include <coro/audio/Node.h>

namespace coro {
namespace core {

class Sink : public Node
{
public:
    static constexpr std::array<Caps,0> inCaps() { return {}; }

    Sink();
    virtual ~Sink();

    virtual void start() {}
    virtual void stop() {}
};

} // namespace core
} // namespace coro
