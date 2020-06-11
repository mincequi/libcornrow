#include "audio/FileSink.h"

#include <cstdint>
#include <cstring>
#include <loguru/loguru.hpp>

namespace coro {
namespace audio {

FileSink::FileSink()
{
}

FileSink::~FileSink()
{
}

void FileSink::start()
{
    m_file.open(m_fileName, std::ios::out | std::ios::binary);
}

void FileSink::stop()
{
    m_file.close();
}

void FileSink::setFileName(const std::string& fileName)
{
    m_fileName = fileName;
}

AudioConf FileSink::onProcess(const AudioConf& conf, AudioBuffer& buffer)
{
    m_file.write(buffer.data(), buffer.size());
    buffer.clear();
    return conf;
}

} // namespace audio
} // namespace coro
