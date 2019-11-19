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

    static constexpr std::array<AudioCaps,0> inCaps() { return {}; }
    static constexpr std::array<AudioCaps,0> outCaps() { return {}; }

    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<
        std::is_base_of_v<Node, Node1> &&
        std::is_base_of_v<Node, Node2> &&
        canIntersect(Node1::outCaps(), Node2::inCaps()), void>
    link(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }
};

template<class InCaps, class OutCaps>
constexpr bool Node::canIntersect(const InCaps& in, const OutCaps& out)
{
    for (const auto& i : in) {
        for (const auto& o : out) {
            if (AudioCaps::canIntersect(i, o)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace audio
} // namespace coro
