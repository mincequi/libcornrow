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

} // namespace core
} // namespace coro
