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

#include "AirplayDecrypter.h"

#include <core/Util.h>

#include <cstring>
#include <openssl/pem.h>

namespace coro {
namespace airplay {

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

AirplayDecrypter::AirplayDecrypter()
{
}

void AirplayDecrypter::init(const std::string& key, const std::string& iv)
{
    initKey(key);
    initIv(iv);
}

const char* AirplayDecrypter::name() const
{
    return "AirplayDecrypter";
}

audio::AudioConf AirplayDecrypter::onProcess(const audio::AudioConf&, core::Buffer& buffer)
{
    decrypt(buffer.data(), buffer.acquire(buffer.size(), this), buffer.size());
    buffer.commit(buffer.size());

    return { audio::AudioCodec::Alac, audio::SampleRate::Rate44100, audio::Channels::Stereo };
}

void AirplayDecrypter::initKey(const std::string& key)
{
    // init RSA
    BIO* bio = BIO_new_mem_buf(airportRsaPrivateKey, -1);
    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);

    auto buffer = core::util::base64Decode(key);
    std::string tmpKey(RSA_size(rsa), 0);
    auto size = RSA_private_decrypt(buffer.size(),
                                   reinterpret_cast<const unsigned char*>(buffer.data()),
                                   reinterpret_cast<unsigned char*>(tmpKey.data()),
                                   rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    tmpKey.resize(size);

    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(tmpKey.data()), 128, &m_aesKey);
}

void AirplayDecrypter::initIv(const std::string& iv)
{
    m_iv = core::util::base64Decode(iv);
}

void AirplayDecrypter::decrypt(const char *in, char *out, int length)
{
    // Not entire buffer is encrypted.
    int encryptedSize = length & ~0xf;
    // Need to copy iv, because AES_cbc_encrypt() overwrites it.
    unsigned char iv[16];
    std::memcpy(iv, m_iv.data(), m_iv.size());
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(in),
                    reinterpret_cast<unsigned char*>(out),
                    encryptedSize, &m_aesKey,
                    iv,
                    AES_DECRYPT);
    // Append unencrypted part.
    std::memcpy(out + encryptedSize, in + encryptedSize, length - encryptedSize);
}

} // namespace airplay
} // namespace core

