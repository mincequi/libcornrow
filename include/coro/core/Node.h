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
    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<canIntersect(Node1::outCaps(), Node2::inCaps())>
    link(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    // Different CapsType for in and out (since different array sizes).
    template<class InCaps, class OutCaps>
    static constexpr bool canIntersect2(const InCaps& in, const OutCaps& out);

    template<class Node1, class Node2>
    static std::enable_if_t<Cap::canIntersect(Node1::outCaps(), Node2::inCaps())>
    link2(Node1& prev, Node2& next) {
        prev.m_next = &next;
    }

    // @TODO(mawe): make this pure virtual to enforce nodes to give a name.
    virtual const char* name() const;

    Node* next() const;

    virtual void start() {}
    virtual void stop() {}

    audio::AudioConf process(const audio::AudioConf& conf, audio::AudioBuffer& buffer);
    void flush();

    bool isBypassed() const;
    void setIsBypassed(bool);

    Node* m_next = nullptr;

protected:
    virtual audio::AudioConf onProcess(const audio::AudioConf& conf, audio::AudioBuffer& buffer);
    virtual void onFlush();

private:
    bool m_isBypassed = false;
};

template<class InCaps, class OutCaps>
constexpr bool Node::canIntersect(const InCaps& in, const OutCaps& out)
{
    for (const auto& i : in) {
        for (const auto& o : out) {
            if (audio::AudioCap::intersect(i, o).isValid()) {
                return true;
            }
        }
    }
    return false;
}

} // namespace core
} // namespace coro
