/*
 * Copyright (C) 2020 Manuel Weichselbaumer <mincequi@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    auto conf = onProcess(_conf, buffer);

    if (!next()) {
        buffer.clear();
        return {};
    }

    return next()->process(conf, buffer);
}

void Node::flush()
{
    if (!isBypassed()) {
        onFlush();
    }

    if (next()) {
        next()->flush();
    }
}

bool Node::isBypassed() const
{
    return m_isBypassed;
}

void Node::setIsBypassed(bool bypass)
{
    m_isBypassed = bypass;
}

audio::AudioConf Node::onProcess(const audio::AudioConf& conf, audio::AudioBuffer&)
{
    return conf;
}

void Node::onFlush()
{
}

} // namespace core
} // namespace coro
