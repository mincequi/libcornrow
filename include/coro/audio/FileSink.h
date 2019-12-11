#pragma once

#include <fstream>
#include <coro/core/Sink.h>

namespace coro {
namespace audio {

class FileSink : public core::Sink
{
public:
    static constexpr std::array<AudioCaps,1> inCaps() {
        return {{ { AudioCodec::RawInt16 | AudioCodec::Ac3 } }};
    }

    FileSink();
    virtual ~FileSink();

    void start() override;
    void stop() override;

    void setFileName(const std::string& fileName);

    AudioConf process(const AudioConf& conf, AudioBuffer& buffer) override;

private:
    std::string m_fileName;
    std::ofstream m_file;
};

} // namespace audio
} // namespace coro
