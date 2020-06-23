#include "audio/AudioNode.h"

#include <core/Caps.h>

namespace coro {
namespace audio {

AudioNode::AudioNode()
{

}

AudioNode::~AudioNode()
{

}

audio::AudioConf AudioNode::onProcess(const audio::AudioConf& conf, core::Buffer&)
{
    return conf;
}

} // namespace audio
} // namespace coro
