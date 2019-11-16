#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/Buffer.h>
#include <coro/audio/Conf.h>
#include <coro/audio/Types.h>

namespace coro {
namespace audio {

class Node
{
public:
    static constexpr std::array<Caps,0> inCaps() { return {}; }
    static constexpr std::array<Caps,0> outCaps() { return {}; }

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

    virtual Conf process(const Conf& conf, Buffer& buffer) = 0;
    Node* next() const;

protected:
    Node* m_next = nullptr;
};

template<class InCaps, class OutCaps>
constexpr bool Node::canIntersect(const InCaps& in, const OutCaps& out)
{
    for (const auto& i : in) {
        for (const auto& o : out) {
            if (Caps::canIntersect(i, o)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace audio
} // namespace coro
