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

#include "Sdp.h"

#include <loguru/loguru.hpp>

#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

namespace coro {
namespace sdp {

Media::Media()
{
}

Sdp Sdp::deserialize(std::istringstream& stream)
{
    std::string line;
    Sdp sdp;
    std::optional<Media> media;
    while (std::getline(stream, line)) {
        LOG_F(ERROR, "line: %s", line.c_str());
        if (!line.rfind("m=", 0)) {
            if (media) {
                sdp.ms.push_back(*media);
                media.reset();
            }

            std::regex rx("m=(\\S+) (\\S+) (\\S+) (\\S+)");
            std::smatch match;
            if (std::regex_search(line, match, rx) && match.size() == 5) {
                media = Media();
                media->type = match[1].str();
                media->port = match[2].str();
                media->proto = match[3].str();
                media->fmt   = match[4].str();
            }
        } else if (!line.rfind("a=") && media) {
            std::regex rx("a=([\\s\\S]+)");
            std::smatch match;
            if (std::regex_search(line, match, rx) && match.size() == 2) {
                media->as.push_back(match[1].str());
            }
        }
    }

    if (media) {
        sdp.ms.push_back(*media);
    }

    return sdp;
}

Sdp::Sdp()
{
}

} // namespace sdp
} // namespace coro
