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

#include <cryptalgo/cryptalgo.h>

namespace WPEFramework {
namespace Plugin {

// Following implementation is for demonstration purposes only, not intended for a production.
struct CookieJarCrypto
{
    static constexpr uint32_t kDefaultVersion = 1;
    static constexpr uint32_t kBlockSize = 16;
    static constexpr uint32_t kKeyLen  = 32;
    const uint8_t kIV[16] = { 0x0 };
    const uint8_t kKey[kKeyLen] = { 0x0 };

    // @brief Encrypt using AES 256 CBC cipher
    // @param in Data to encrypt
    // @param version Version of the key used for encryption
    // @param out Encrypted data
    uint32_t Encrypt(std::vector<uint8_t> in, unsigned int& version /* @out */, std::vector<uint8_t>& out /* @out */)
    {
        uint32_t rc;
        size_t out_size = ((in.size() + kBlockSize - 1) / kBlockSize) * kBlockSize;

        version = kDefaultVersion;
        out.resize(out_size);
        in.resize(out_size, out_size - in.size());         // Add PKCS padding

        Crypto::AESEncryption encryptor(Crypto::AES_CBC);
        encryptor.InitialVector(kIV);
        rc = encryptor.Key(kKeyLen, kKey);
        if (rc == Core::ERROR_NONE)
        {
            rc = encryptor.Encrypt(in.size(), in.data(), out.data());
        }

        return rc;
    }

    // @brief Decrypt using AES 256 CBC cipher
    // @param in Data to decrypt
    // @param version Version of the key used for encryption
    // @param out Clear data
    uint32_t Decrypt(std::vector<uint8_t> in, unsigned int version, std::vector<uint8_t>& out)
    {
        uint32_t rc = Core::ERROR_UNAVAILABLE;
        if (version == kDefaultVersion)
        {
            size_t out_size = ((in.size() + kBlockSize - 1) / kBlockSize) * kBlockSize;
            out.resize(out_size);

            Crypto::AESDecryption decryptor(Crypto::AES_CBC);
            decryptor.InitialVector(kIV);
            rc = decryptor.Key(kKeyLen, kKey);
            if (rc == Core::ERROR_NONE)
            {
                rc = decryptor.Decrypt(in.size(), in.data(), out.data());
            }
        }

        return rc;
    }

};

} // namespace Plugin
} // namespace WPEFramework
