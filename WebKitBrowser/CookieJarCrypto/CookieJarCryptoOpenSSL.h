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

#pragma once

#include "../Module.h"

#include <stdio.h>
#include <glib.h>
#include <openssl/err.h>
#include <openssl/evp.h>

namespace WPEFramework {
namespace Plugin {

struct CookieJarCrypto
{
    static constexpr uint32_t kDefaultVersion = 3;
    static constexpr uint32_t kKeyLen  = 32;

    bool _DidTryInit { false };
    bool _KeyV3loaded { false };
    uint8_t _KeyV3 [kKeyLen];

    static bool loadKeyV3(unsigned char *key, unsigned int keyLen)
    {
        // command in the binary.
        const char cmd[] = "/usr/bin/GetConfigFile cookie.jar stdout";
        FILE *p = popen(cmd, "r");
        if (p) {
            char buf[64];
            std::string s;
            while(fgets(buf, sizeof(buf), p) != NULL)
                s.append(buf);
            pclose(p);

            gsize outlen;
            guint8* decoded = g_base64_decode(s.c_str(), &outlen);
            if (outlen == keyLen) {
                memcpy(key, decoded, keyLen);
                g_free(decoded);
                return true;
            } else {
                fprintf(stderr,  "Unexpected data length for config: %d instead of %d\n", outlen, keyLen);
                g_free(decoded);
            }
        }
        fprintf(stderr,  "Failed to run cfgp\n");
        return false;
    }

    bool initialize()
    {
        if (!_DidTryInit) {
            _DidTryInit = true;
            _KeyV3loaded = loadKeyV3(_KeyV3, kKeyLen);
        }
        return _KeyV3loaded;
    }

    uint32_t crypt_impl(const std::vector<uint8_t>& in, bool encrypt, unsigned int &version, std::vector<uint8_t> &result)
    {
        static unsigned char keyV2[kKeyLen] = {
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
            0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
            0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x18
        };
        static unsigned char iv[]  = {
            0x58, 0x52, 0x45, 0x4E, 0x61, 0x74, 0x69, 0x76,
            0x65, 0x52, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65
        };

        unsigned char *key = keyV2;

        initialize();

        if (3 == version) {
            if (!_KeyV3loaded) {
                if (!encrypt) {
                    SYSLOG(Logging::Error,(_T("Failed get parameters.")));
                    return Core::ERROR_GENERAL;
                }
                SYSLOG(Logging::Notification,(_T("Failed get parameters, falling back to version 2.")));
                key = keyV2;
                version = 2;
            } else {
                SYSLOG(Logging::Notification,(_T("Using cookiejar version 3.")));
                key = _KeyV3;
            }
        }

#define CHECK_EVP_STATUS(r)                                             \
        if (r == 0) {                                                   \
            SYSLOG(Logging::Error,(_T("Crypt failed")));                \
            while (ERR_peek_error() != 0) {                             \
                SYSLOG(                                                 \
                    Logging::Error,                                     \
                    (_T("Openssl %s"),                                  \
                     ERR_error_string(ERR_get_error(), nullptr)));      \
            }                                                           \
            return Core::ERROR_GENERAL;                                 \
        }

        ERR_load_crypto_strings();

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(ctx);

        const EVP_CIPHER* cipher = EVP_aes_256_cbc();

        int status = 0;
        auto init_ex = encrypt ? EVP_EncryptInit_ex : EVP_DecryptInit_ex;

        status = init_ex(ctx, cipher, 0, 0, 0);
        CHECK_EVP_STATUS(status);

        status = EVP_CIPHER_CTX_set_key_length(ctx, kKeyLen);
        CHECK_EVP_STATUS(status);

        status = init_ex(ctx, 0, 0, key, iv);
        CHECK_EVP_STATUS(status);

        status = EVP_CIPHER_CTX_set_padding(ctx, 1);
        CHECK_EVP_STATUS(status);

        int outl = 0;
        int inl = in.size();

        result.resize(inl + EVP_CIPHER_CTX_block_size(ctx));

        auto update = encrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
        status = update(ctx, (unsigned char*)result.data(), &outl, (unsigned char*) in.data(), inl);
        CHECK_EVP_STATUS(status);

        inl = outl;
        result.resize(inl + EVP_CIPHER_CTX_block_size(ctx));
        auto final_ex = encrypt ? EVP_EncryptFinal_ex : EVP_DecryptFinal_ex;
        status = final_ex(ctx, (unsigned char*)(result.data() + inl), &outl);
        CHECK_EVP_STATUS(status);

        result.resize(inl + outl);

        ERR_free_strings();
        EVP_CIPHER_CTX_cleanup(ctx);
        EVP_CIPHER_CTX_free(ctx);

        ctx = nullptr;

#undef CHECK_EVP_STATUS

        return Core::ERROR_NONE;
    }

    // @brief Encrypt using AES 256 CBC cipher
    // @param in Data to encrypt
    // @param version Version of the key used for encryption
    // @param out Encrypted data
    // @return Error code or ERROR_NONE
    uint32_t Encrypt(std::vector<uint8_t> in, unsigned int& version /* @out */, std::vector<uint8_t>& out /* @out */)
    {
        version = kDefaultVersion;
        return crypt_impl(in, true, version, out);
    }

    // @brief Decrypt using AES 256 CBC cipher
    // @param in Data to decrypt
    // @param version Version of the key used for encryption
    // @param out Clear data
    // @return Error code or ERROR_NONE
    uint32_t Decrypt(std::vector<uint8_t> in, unsigned int version, std::vector<uint8_t>& out)
    {
        return crypt_impl(in, false, version, out);
    }

};

} // namespace Plugin
} // namespace WPEFramework
