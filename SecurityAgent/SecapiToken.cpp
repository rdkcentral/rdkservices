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

#include "SecapiToken.h"

#define PRINT(f_, ...) \
  { printf((f_), ##__VA_ARGS__); fflush(stdout); }
#define CHECK_RESULT(x) { \
  Sec_Result result = (x); \
  ASSERT(result == SEC_RESULT_SUCCESS); \
  if (result != SEC_RESULT_SUCCESS) \
    PRINT("%s:%d %s : %d\n", __FILE__, __LINE__, #x, result); \
}

using namespace std;
using namespace WPEFramework;

const char* JWTSecApi::MacGenerator::StorageDir = nullptr;

JWTSecApi::SecApi::SecApi(const char* storage)
  : secProcHandle(nullptr) {
  CHECK_RESULT(SecProcessor_GetInstance_Directories(&secProcHandle, storage, storage));
}

JWTSecApi::SecApi::~SecApi() {
  if (secProcHandle != nullptr) {
    CHECK_RESULT(SecProcessor_Release(secProcHandle));
  }
}

SEC_OBJECTID JWTSecApi::SecApi::GenerateKey(Sec_KeyType keyType, Sec_StorageLoc location) {
  SEC_OBJECTID id = SecKey_ObtainFreeObjectId(secProcHandle,
      SEC_OBJECTID_USER_BASE, SEC_OBJECTID_USER_TOP);

  CHECK_RESULT(SecKey_Generate(secProcHandle, id, keyType, location));

  return id;
}

void JWTSecApi::SecApi::DeleteKey(SEC_OBJECTID id) {
  CHECK_RESULT(SecKey_Delete(secProcHandle, id));
}

void JWTSecApi::SecApi::Mac(SEC_OBJECTID id,
    const uint16_t sourceSize, const uint8_t *source,
    uint16_t &macSize, uint8_t *mac) {

  SEC_SIZE mac_len;
  CHECK_RESULT(SecMac_SingleInputId(secProcHandle, SEC_MACALGORITHM_HMAC_SHA256, id,
      const_cast<SEC_BYTE *>(reinterpret_cast<const SEC_BYTE *>(source)),
      static_cast<SEC_SIZE>(sourceSize),
      reinterpret_cast<SEC_BYTE *>(mac),
      &mac_len));
  macSize = static_cast<uint16_t>(mac_len);
}

JWTSecApi::MacGenerator::MacGenerator()
  : secApi(make_shared<SecApi>(StorageDir)) {
  ASSERT(secApi != nullptr);
  hmacKey = secApi->GenerateKey(SEC_KEYTYPE_HMAC_256, SEC_STORAGELOC_RAM);
}


JWTSecApi::MacGenerator::~MacGenerator() {
  ASSERT(secApi != nullptr);
  secApi->DeleteKey(hmacKey);
}

void JWTSecApi::MacGenerator::Mac(const uint16_t sourceSize, const uint8_t *source, uint16_t &macSize, uint8_t *mac) {
  ASSERT(secApi != nullptr);
  secApi->Mac(hmacKey, sourceSize, source, macSize, mac);
}

JWTSecApi::MacGenerator JWTSecApi::generator;

JWTSecApi::JWTSecApi()
  : jwt(Web::JSONWebToken::mode(-1), 0, nullptr) {
}

uint16_t JWTSecApi::Encode(string &token, const uint16_t length, const uint8_t *payload) {
  jwt.Encode(token, length, payload);

  uint16_t macSize;
  uint8_t mac[SEC_MAC_MAX_LEN];
  generator.Mac(static_cast<uint16_t>(token.length()),
      reinterpret_cast<const uint8_t*>(token.c_str()),
      macSize, mac);
  TCHAR signature[((SEC_MAC_MAX_LEN * 8) / 6) + 4];
  uint16_t convertedLength = Core::URL::Base64Encode(mac, macSize,
      signature, sizeof(signature), false);
  token += '.' + string(signature, convertedLength);

  ASSERT(token.length() < 0xFFFF);

  return (static_cast<uint16_t>(token.length()));
}

uint16_t JWTSecApi::Decode(const string &token, const uint16_t maxLength, uint8_t *payload) {
  uint16_t length = ~0;

  if (ValidSignature(token)) {
    size_t pos = token.find_first_of('.');
    size_t sig_pos = token.find_last_of('.');
    if (sig_pos > pos) {
      length = Core::URL::Base64Decode(token.substr(pos + 1).c_str(),
          static_cast<uint16_t>(sig_pos - pos) - 1, payload, maxLength, nullptr);
    }
  }

  return (length);
}

uint16_t JWTSecApi::PayloadLength(const string &token) const {
  return jwt.PayloadLength(token);
}

bool JWTSecApi::ValidSignature(const string& token) {
  bool result = false;

  size_t pos = token.find_last_of('.');
  if (pos != string::npos) {
    uint8_t signature[SEC_MAC_MAX_LEN];
    Core::URL::Base64Decode(token.substr(pos + 1).c_str(),
        static_cast<uint16_t>(token.length() - pos - 1), signature, sizeof(signature), nullptr);

    uint16_t macSize;
    uint8_t mac[SEC_MAC_MAX_LEN];
    generator.Mac(static_cast<uint16_t>(pos),
        reinterpret_cast<const uint8_t*>(token.substr(0, pos).c_str()),
        macSize, mac);

    result = (::memcmp(signature, mac, macSize) == 0);
  }

  return (result);
}
