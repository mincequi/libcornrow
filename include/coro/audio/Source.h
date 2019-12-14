#pragma once

#include <coro/audio/AudioCaps.h>
#include <coro/audio/Node.h>

#include <atomic>
#include <functional>
#include <mutex>

namespace coro {
namespace audio {

class AudioBuffer;
class AudioConf;

class Source : public Node
{
public:
    static constexpr std::array<AudioCaps,0> outCaps() { return {}; }

    Source();
    virtual ~Source();

    Source(const Source&) = delete;

    virtual void start();
    virtual void stop();
    virtual bool isStarted() const;
    virtual bool wantsToStart() const;
    virtual void setWantsToStart(bool wts);

    using WantsToStartCallback = std::function<void(Source* const, bool)>;
    void setWantsToStartCallback(WantsToStartCallback callback);

    void pushBuffer(const AudioConf& conf, AudioBuffer& buffer);

protected:
    std::mutex  m_mutex;
    WantsToStartCallback m_wantsToStartCallback;

private:
    bool m_isStarted = false;
    bool m_wantsToStart = false;

    std::atomic_uint m_bufferCount = 0;
};

} // namespace audio
} // namespace coro
