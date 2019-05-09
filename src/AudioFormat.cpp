#include <AEChannelData.h>

std::ostream& operator<<(std::ostream& os, const AudioSampleFormat& format)
{
    switch (format) {
    case AudioSampleFormat::Invalid:    os << "Invalid";    break;
    case AudioSampleFormat::S16BE:      os << "S16BE";      break;
    case AudioSampleFormat::S16LE:      os << "S16LE";      break;
    case AudioSampleFormat::S16NE:      os << "S16NE";      break;
    case AudioSampleFormat::S32BE:      os << "S32BE";      break;
    case AudioSampleFormat::S32LE:      os << "S32LE";      break;
    case AudioSampleFormat::S32NE:      os << "S32NE";      break;
    case AudioSampleFormat::Double:     os << "Double";     break;
    case AudioSampleFormat::Float:      os << "Float";      break;
    case AudioSampleFormat::Bitstream:  os << "Bitstream";  break;
    case AudioSampleFormat::Max:        os << "Invalid";    break;
    }

    return os;
}
