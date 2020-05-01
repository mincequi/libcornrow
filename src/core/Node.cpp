#include "core/Node.h"

namespace coro {
namespace core {

/*
Node::Node()
{

}

Node::~Node()
{
}
*/

Node* Node::next() const
{
    return m_next;
}

audio::AudioConf Node::process(const audio::AudioConf& _conf, audio::AudioBuffer& buffer)
{
    if (_conf.codec == audio::AudioCodec::Invalid || !buffer.size()) {
        buffer.clear();
        return {};
    }

    if (isBypassed() && next()) {
        return next()->process(_conf, buffer);
    } else if (isBypassed() && !next()) {
        buffer.clear();
        return {};
    }

    auto conf = doProcess(_conf, buffer);

    if (!next()) {
        buffer.clear();
        return {};
    }

    return next()->process(conf, buffer);
}

bool Node::isBypassed() const
{
    return m_isBypassed;
}

void Node::setIsBypassed(bool bypass)
{
    m_isBypassed = bypass;
}

audio::AudioConf Node::doProcess(const audio::AudioConf& conf, audio::AudioBuffer&)
{
    return conf;
}

} // namespace core
} // namespace coro
