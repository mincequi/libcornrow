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

bool Node::isBypassed() const
{
    return m_isBypassed;
}

void Node::setIsBypassed(bool bypass)
{
    m_isBypassed = bypass;
}

} // namespace core
} // namespace coro
