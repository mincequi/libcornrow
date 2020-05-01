#pragma once

// @TODO(mawe): Temporary use audio types
#include <coro/audio/AudioBuffer.h>
#include <coro/audio/AudioConf.h>
#include <coro/core/Caps.h>

namespace coro {
namespace core {

class Node
{
public:
    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<
        std::is_base_of_v<Node, Node1> &&
        std::is_base_of_v<Node, Node2> &&
        canIntersect(Node1::outCaps(), Node2::inCaps()), void>
        //canIntersect(Node1, Node2), void>
    link(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    // @TODO(mawe): make this pure virtual to enforce nodes to give a name.
    virtual const char* name() const { return ""; }
    Node* next() const;

    virtual void start() {}
    virtual void stop() {}

    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer);

    bool isBypassed() const;
    void setIsBypassed(bool);

protected:
    virtual audio::AudioConf doProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer);

    Node* m_next = nullptr;

private:
    bool m_isBypassed = false;
};

template<class InCaps, class OutCaps>
constexpr bool Node::canIntersect(const InCaps& in, const OutCaps& out)
{
    for (const auto& i : in) {
        for (const auto& o : out) {
            if (Caps::intersect(i, o).isValid()) {
                return true;
            }
        }
    }
    return false;
}

} // namespace core
} // namespace coro
