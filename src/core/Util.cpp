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

#include "Util.h"

namespace coro {
namespace core {
namespace util {

const char* base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789"
                           "+/";

static unsigned int pos_of_char(const unsigned char chr)
{
    if      (chr >= 'A' && chr <= 'Z') return chr - 'A';
    else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A')               + 1;
    else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
    else if (chr == '+') return 62;
    else if (chr == '/') return 63;

    return 0;
}

std::string base64Decode(const std::string& in)
{
    std::string out;
    out.reserve(in.size() / 4 * 3);

    size_t pos = 0;
    while (pos < in.size()) {
        unsigned int pos_of_char_1 = pos_of_char(in[pos+1]);
        out.push_back(static_cast<std::string::value_type>(((pos_of_char(in[pos+0])) << 2) + ((pos_of_char_1 & 0x30) >> 4)));

        if (in[pos+2] != '=') {
            unsigned int pos_of_char_2 = pos_of_char(in[pos+2] );
            out.push_back(static_cast<std::string::value_type>(((pos_of_char_1 & 0x0f) << 4) + ((pos_of_char_2 & 0x3c) >> 2)));

            if (in[pos+3] != '=' && in[pos+3] != '.') {
                out.push_back(static_cast<std::string::value_type>(((pos_of_char_2 & 0x03) << 6) + pos_of_char(in[pos+3])));
            }
        }

        pos += 4;
    }

    return out;
}

std::string base64Encode(const char* data, size_t size, bool padding)
{
    std::string out;
    out.reserve((size + 2) / 3 * 4);

    size_t pos = 0;
    while (pos < size) {
        out.push_back(base64Chars[(data[pos + 0] & 0xfc) >> 2]);

        if (pos+1 < size) {
           out.push_back(base64Chars[((data[pos + 0] & 0x03) << 4) + ((data[pos + 1] & 0xf0) >> 4)]);

           if (pos+2 < size) {
              out.push_back(base64Chars[((data[pos + 1] & 0x0f) << 2) + ((data[pos + 2] & 0xc0) >> 6)]);
              out.push_back(base64Chars[  data[pos + 2] & 0x3f]);
           } else {
              out.push_back(base64Chars[(data[pos + 1] & 0x0f) << 2]);
              if (padding) {
                  out.push_back('=');
              }
           }
        } else {
            out.push_back(base64Chars[(data[pos + 0] & 0x03) << 4]);
            if (padding) {
                out.push_back('=');
                out.push_back('=');
            }
        }

        pos += 3;
    }

    return out;
}

} // namespace util
} // namespace core
} // namespace coro
