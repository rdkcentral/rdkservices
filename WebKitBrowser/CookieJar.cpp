/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CookieJar.h"

#include <sstream>
#include <iterator>
#include <algorithm>

#include <glib.h>
#include <zlib.h>

#if defined(COOKIE_JAR_CRYPTO_IMPLEMENTATION)
#include COOKIE_JAR_CRYPTO_IMPLEMENTATION
#else
#error "Please define COOKIE_JAR_CRYPTO_IMPLEMENTATION"
#endif

namespace WPEFramework {
namespace Plugin {

namespace {

static std::string serialize(const std::vector<std::string>& cookies)
{
    std::ostringstream os;
    std::copy(cookies.begin(), cookies.end(), std::ostream_iterator<std::string>(os, "\n"));
    return os.str();
}

static void deserialize(const std::string& cookies, std::vector<std::string>& result)
{
    std::string cookie;
    std::stringstream ss(cookies);
    while(ss.good())
    {
        getline(ss, cookie, '\n');
        if (!cookie.empty())
            result.push_back(cookie);
    }
}

static std::string toBase64(const std::vector<uint8_t>& in)
{
    gchar* encoded = g_base64_encode(in.data(), in.size());
    std::string result;
    if (encoded) {
        result.assign(encoded);
        g_free(encoded);
    }
    return result;
}

static std::vector<uint8_t> fromBase64(const std::string& str)
{
    gsize outlen;
    guint8* decoded = g_base64_decode(str.c_str(), &outlen);
    std::vector<uint8_t> result;
    if (decoded) {
        result.assign(decoded, decoded + outlen);
        g_free(decoded);
    }
    return result;
}

static std::vector<uint8_t> compress(const std::string& str)
{
    std::vector<uint8_t> result;
    size_t nbytes = str.size();
    if (nbytes == 0)
    {
        result.resize(4, '\0');
        return result;
    }
    const int compressionLevel = 1;
    unsigned long len = nbytes + nbytes / 100 + 13;
    int status;
    do
    {
        result.resize(len + 4);
        status = ::compress2((unsigned char*)result.data() + 4, &len,
            (const unsigned char*)str.c_str(), nbytes, compressionLevel);
        switch (status)
        {
        case Z_OK:
            result.resize(len + 4);
            result[0] = (nbytes & 0xff000000) >> 24;
            result[1] = (nbytes & 0x00ff0000) >> 16;
            result[2] = (nbytes & 0x0000ff00) >> 8;
            result[3] = (nbytes & 0x000000ff);
            break;
        case Z_MEM_ERROR:
            TRACE_GLOBAL(Trace::Error,(_T("Z_MEM_ERROR: Not enough memory")));
            result.resize(0);
            break;
        case Z_BUF_ERROR:
            len *= 2;
            break;
        }
    }
    while (status == Z_BUF_ERROR);
    return result;
}

static std::string uncompress(const std::vector<uint8_t>& in)
{
    std::string result;
    size_t nbytes = in.size();
    if (nbytes <= 4)
    {
        if (nbytes < 4 || std::any_of(in.cbegin(), in.cend(), [](int v) {return v != 0;}))
        {
            TRACE_GLOBAL(Trace::Error,(_T("Input data is corrupted")));
        }
        return result;
    }
    const unsigned char* data = (const unsigned char*) in.data();
    unsigned long expectedSize = (unsigned long)(
        (data[0] << 24) | (data[1] << 16) |
        (data[2] <<  8) | (data[3]));
    unsigned long len = std::max(expectedSize, 1ul);
    int status;
    do
    {
        result.resize(len);
        status = ::uncompress((unsigned char*)result.data(), &len, data + 4, nbytes - 4);
        switch (status)
        {
        case Z_BUF_ERROR:
            len *= 2;
            break;
        case Z_MEM_ERROR:
            TRACE_GLOBAL(Trace::Error,(_T("Z_MEM_ERROR: Not enough memory")));
            result.resize(0);
            break;
        case Z_DATA_ERROR:
            TRACE_GLOBAL(Trace::Error,(_T("Z_DATA_ERROR: Input data is corrupted")));
            result.resize(0);
            break;
        }
    }
    while (status == Z_BUF_ERROR);
    return result;
}

// CRC-16 for compatability with rdkbrowser / rdkbrwoser2
static uint32_t crc_checksum(const std::string& str)
{
    static const unsigned short crc_tbl[16] = {
        0x0000, 0x1081, 0x2102, 0x3183,
        0x4204, 0x5285, 0x6306, 0x7387,
        0x8408, 0x9489, 0xa50a, 0xb58b,
        0xc60c, 0xd68d, 0xe70e, 0xf78f
    };
    unsigned short crc = 0xffff;
    unsigned char c = 0;
    const unsigned char *p = (const unsigned char*) str.data();
    size_t len = str.size();
    while (len--)
    {
        c = *p++;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
        c >>= 4;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return ~crc & 0xffff;
}

} // namespace

struct CookieJar::CookieJarPrivate
{
    CookieJarCrypto _cookieJarCrypto;

    uint32_t Pack(const std::vector<std::string> &cookies, uint32_t& version, uint32_t& checksum, string& payload)
    {
        uint32_t rc;
        std::string serialized;
        std::vector<uint8_t> encrypted;

        serialized = serialize(cookies);
        checksum = crc_checksum(serialized);

        rc = _cookieJarCrypto.Encrypt(compress(serialized), version, encrypted);

        if (rc != Core::ERROR_NONE)
        {
            TRACE_GLOBAL(Trace::Error,(_T("Encryption failed, rc = %u"), rc));
        }
        else
        {
            payload = toBase64(encrypted);
        }

        return rc;
    }

    uint32_t Unpack(const uint32_t version, const uint32_t checksum, const string& payload, std::vector<std::string>& cookies)
    {
        uint32_t rc = Core::ERROR_GENERAL;
        std::vector<uint8_t> decrypted;

        rc = _cookieJarCrypto.Decrypt(fromBase64(payload), version, decrypted);

        if (rc != Core::ERROR_NONE)
        {
            TRACE_GLOBAL(Trace::Error,(_T("Decryption failed, rc = %u"), rc));
        }
        else
        {
            std::string serialized;
            int actualChecksum;

            serialized = uncompress(decrypted);
            actualChecksum = crc_checksum(serialized);
            if (actualChecksum != checksum)
            {
                rc = Core::ERROR_GENERAL;
                TRACE_GLOBAL(Trace::Error,(_T("Checksum does not match: actual=%d expected=%d"), actualChecksum, checksum));
            }
            else
            {
                deserialize(serialized, cookies);
            }
        }

        return rc;
    }
};

CookieJar::CookieJar()
    : _priv(new CookieJarPrivate)
{
}

CookieJar::~CookieJar() = default;

uint32_t CookieJar::Pack(uint32_t& version, uint32_t& checksum, string& payload) const
{
    return _priv->Pack(_cookies, version, checksum, payload);
}

uint32_t CookieJar::Unpack(const uint32_t version, const uint32_t checksum, const string& payload)
{
    uint32_t rc;
    std::vector<std::string> cookies;

    rc = _priv->Unpack(version, checksum, payload, cookies);

    if (rc == WPEFramework::Core::ERROR_NONE) {
        _cookies = std::move(cookies);
        _refreshed.SetState( false );
    }

    return rc;
}

void CookieJar::SetCookies(std::vector<std::string> && cookies)
{
    _cookies = std::move(cookies);
    _refreshed.SetState( true );
}

std::vector<std::string> CookieJar::GetCookies() const
{
    return _cookies;
}

} // namespace Plugin
} // namespace WPEFramework
