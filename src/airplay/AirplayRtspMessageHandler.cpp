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

#include "AirplayRtspMessageHandler.h"

#include "AlacDecoder.h"
#include "AirplayDecrypter.h"

#include <coro/audio/AudioDecoderFfmpeg.h>
#include <coro/core/UdpSource.h>
#include <coro/rtp/RtpDecoder.h>
#include <coro/rtsp/RtspMessage.h>
#include "core/MainloopPrivate.h"
#include "core/Util.h"
#include "sdp/Sdp.h"

#include <loguru/loguru.hpp>

#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <regex>
#include <sstream>

namespace coro {
namespace airplay {

using namespace coro::audio;
using namespace coro::rtsp;

static char airportRsaPrivateKey[] = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n"
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n"
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n"
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n"
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n"
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n"
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n"
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n"
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n"
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n"
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n"
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n"
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n"
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n"
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n"
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n"
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n"
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n"
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n"
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n"
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\n"
"-----END RSA PRIVATE KEY-----";

AirplayRtspMessageHandler::AirplayRtspMessageHandler(uint16_t audioPort,
                                                     uint16_t controlPort,
                                                     rtp::RtpDecoder<audio::AudioCodec::Alac>& rtpReceiver,
                                                     AirplayDecrypter& decrypter,
                                                     audio::AudioDecoderFfmpeg<audio::AudioCodec::Alac>& decoder)
    : m_audioPort(audioPort),
      m_controlPort(controlPort),
      m_rtpReceiver(rtpReceiver),
      m_decrypter(decrypter),
      m_decoder(decoder)
{
}

void AirplayRtspMessageHandler::onOptions(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    RtspMessageHandler::onOptions(request, response, ipAddress);

    //response->header("Audio-Jack-Status") = "connected; type=analog"; // not needed
    if (request.header("Apple-Challenge").size()) {
        onAppleChallenge(request, response, ipAddress);
    }
}

void AirplayRtspMessageHandler::onAnnounce(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    if (!request.sdp().media.size()) {
        return;
    }

    // @TODO(mawe): this might throw. To be fixed.
    std::string rsaaeskey = request.sdp().media.front().attributes.at("rsaaeskey");
    std::string aesiv = request.sdp().media.front().attributes.at("aesiv");
    std::string fmtp = request.sdp().media.front().attributes.at("fmtp");

    m_decrypter.init(rsaaeskey, aesiv);
    m_decoder.init(fmtp);
}

void AirplayRtspMessageHandler::onSetup(const RtspMessage& request, RtspMessage* response, uint32_t ipAddress) const
{
    if (m_udpSourceAudio) {
        delete m_udpSourceAudio;
        m_udpSourceAudio = new core::UdpSource;
    }

    std::stringstream ss;
    ss << "RTP/AVP/UDP;unicast;mode=record";
    ss << ";server_port=" << m_audioPort;       // server_port and
    ss << ";control_port=" << m_controlPort;    // control_port are mandatory.
    //ss << ";timing_port=" << m_timerPort;     // timing_port is optional.
    ss.flush();

    //response->header("Session") = "1";    // not needed
    response->header("Transport") = ss.str();
}

void AirplayRtspMessageHandler::onRecord(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const
{
    //m_rtpReceiver.flush();
}

void AirplayRtspMessageHandler::onTeardown(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const
{
    if (m_udpSourceAudio) {
        delete m_udpSourceAudio;
        m_udpSourceAudio = nullptr;
    }
    //m_rtpReceiver.flush();
}

void AirplayRtspMessageHandler::onAppleChallenge(const rtsp::RtspMessage& request, rtsp::RtspMessage* response, uint32_t ipAddress) const
{
    // base64 decode appleChallenge
    auto appleChallenge = core::util::base64Decode(request.header("Apple-Challenge"));
    // append ipv4 address in network byte order
    auto ipAddressBe = htobe32(ipAddress);
    appleChallenge.insert(appleChallenge.size(), reinterpret_cast<const char*>(&ipAddressBe), sizeof(ipAddressBe));
    // append mac address in network byte order
    uint8_t hwAddress[] = { 1, 2, 3, 4, 5, 6 };
    appleChallenge.insert(appleChallenge.size(), reinterpret_cast<const char*>(hwAddress), sizeof(hwAddress));
    // pad with 0s until 32 byte
    while (appleChallenge.size() < 32) {
        appleChallenge.push_back('\0');
    }

    // Encrypt the buffer using the RSA private key extracted in shairport.
    // https://www.openssl.org/docs/crypto/RSA_private_encrypt.html
    BIO* bio = BIO_new_mem_buf(airportRsaPrivateKey, -1);
    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);

    // need memory for signature
    std::vector<char> to;
    to.resize(RSA_size(rsa), 0);
    RSA_private_encrypt(appleChallenge.size(),
                        reinterpret_cast<const unsigned char*>(appleChallenge.data()),
                        reinterpret_cast<unsigned char*>(to.data()),
                        rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);

    // Base64 encode the ciphertext without padding
    response->header("Apple-Response") = core::util::base64Encode(to.data(), to.size(), false);
}

} // namespace airplay
} // namespace coro
