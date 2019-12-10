#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/audio/AudioTypes.h>
#include <coro/core/Node.h>

namespace coro {
namespace audio {

class Node : public core::Node
{
public:
    Node();
    virtual ~Node();

    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr InCaps intersect(const InCaps& in, const OutCaps& out);

    virtual void start(const AudioConf& conf) {}
    void stop() override {}

protected:
    friend class core::Node;
};

template<class InCaps, class OutCaps>
constexpr InCaps Node::intersect(const InCaps& in, const OutCaps& out)
{
    InCaps intersection;
    uint8_t index = 0;
    for (const auto& i : in) {
        for (const auto& o : out) {
            auto caps = AudioCaps::intersect(i, o);
            if (caps) {
                intersection.at(index++) = caps;
            }
        }
    }
    return intersection;
}

} // namespace audio
} // namespace coro
