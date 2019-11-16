#include "audio/Node.h"

#include <core/Caps.h>

namespace coro {
namespace audio {

Node::Node()
{

}

Node::~Node()
{

}

Node* Node::next() const
{
    return m_next;
}

} // namespace audio
} // namespace coro

