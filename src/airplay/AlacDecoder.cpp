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

#include "AlacDecoder.h"

#include <cstring>
#include <regex>

#include <openssl/pem.h>

#include "core/Util.h"

namespace coro {
namespace airplay {

AlacDecoder::AlacDecoder()
{
    m_alac = alac_create(16, 2);
}

AlacDecoder::~AlacDecoder()
{
    alac_free(m_alac);
}

void AlacDecoder::init(const std::string& fmtp)
{
    std::regex rx("(\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (\\d+)");
    std::smatch match;
    if (std::regex_search(fmtp, match, rx) && match.size() == 13) {
        m_alac->setinfo_max_samples_per_frame   = stoi(match[2].str());
        m_alac->setinfo_7a                      = stoi(match[3].str());
        m_alac->setinfo_sample_size             = stoi(match[4].str());
        m_alac->setinfo_rice_historymult        = stoi(match[5].str());
        m_alac->setinfo_rice_initialhistory     = stoi(match[6].str());
        m_alac->setinfo_rice_kmodifier          = stoi(match[7].str());
        m_alac->setinfo_7f                      = stoi(match[8].str());
        m_alac->setinfo_80                      = stoi(match[9].str());
        m_alac->setinfo_82                      = stoi(match[10].str());
        m_alac->setinfo_86                      = stoi(match[11].str());
        m_alac->setinfo_8a_rate                 = stoi(match[12].str());
        alac_allocate_buffers(m_alac);
    }
}

audio::AudioConf AlacDecoder::onProcess(const audio::AudioConf&, core::Buffer& buffer)
{
    int outputSize;
    alac_decode_frame(m_alac,
                      reinterpret_cast<unsigned char*>(buffer.data()),
                      buffer.acquire(2048),
                      &outputSize);
    buffer.commit(outputSize);

    return { audio::AudioCodec::RawInt16, audio::SampleRate::Rate44100, audio::Channels::Stereo };
}

} // namespace airplay
} // namespace core

