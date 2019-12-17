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

    void start();
    void stop();
    virtual bool isStarted() const;
    virtual bool isReady() const;
    virtual void setReady(bool wts);

    using ReadyCallback = std::function<void(Source* const, bool)>;
    void setReadyCallback(ReadyCallback callback);

    void pushBuffer(const AudioConf& conf, AudioBuffer& buffer);

protected:
    virtual void doStart();
    virtual void doStop();

    std::mutex  m_mutex;
    ReadyCallback m_isReadyCallback;

private:
    std::atomic_bool m_isStarted = false;
    std::atomic_bool m_isReady = false;
    std::atomic_uint m_bufferCount = 0;
};

} // namespace audio
} // namespace coro
