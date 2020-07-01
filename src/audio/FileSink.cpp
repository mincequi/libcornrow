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

void FileSink::setFileName(const std::string& fileName)
{
    if (m_fileName == fileName) {
        return;
    }

    m_fileName = fileName;

    // If it had been opened previously, reopen with new filename.
    if (m_file.is_open()) {
        onStart();
    }
}

const char* FileSink::name() const
{
    return "FileSink";
}

void FileSink::onStart()
{
    // Close previously opened file
    onStop();
    m_file.open(m_fileName, std::ios::out | std::ios::binary);
}

void FileSink::onStop()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

AudioConf FileSink::onProcess(const AudioConf& conf, core::Buffer& buffer)
{
    if (!m_file.is_open()) {
        onStart();
    }

    m_file.write(buffer.data(), buffer.size());
    buffer.clear();
    return conf;
}

} // namespace audio
} // namespace coro
