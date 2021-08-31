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

#include "Module.h"

#include "sec_security.h"
#include "sec_security_datatype.h"

#include <memory>

class JWTSecApi {
private:
  class SecApi {
  public:
    SecApi(const char* storage);
    ~SecApi();

    SEC_OBJECTID GenerateKey(Sec_KeyType keyType, Sec_StorageLoc location);
    void DeleteKey(SEC_OBJECTID id);
    void Mac(SEC_OBJECTID id,
        const uint16_t sourceSize, const uint8_t source[],
        uint16_t &macSize, uint8_t mac[]);

  private:
    Sec_ProcessorHandle* secProcHandle;
  };

  class MacGenerator {
  public:
    MacGenerator();
    ~MacGenerator();
    void Mac(const uint16_t sourceSize, const uint8_t source[],
        uint16_t &macSize, uint8_t mac[]);

  private:
    static const char* StorageDir;
    std::shared_ptr<SecApi> secApi;
    SEC_OBJECTID hmacKey;
  };

private:
  JWTSecApi(const JWTSecApi &) = delete;
  JWTSecApi &operator=(const JWTSecApi &) = delete;
public:
  JWTSecApi();

public:
  uint16_t Encode(string &token, const uint16_t length, const uint8_t payload[]);
  uint16_t Decode(const string &token, const uint16_t maxLength, uint8_t payload[]);
  uint16_t PayloadLength(const string &token) const;

private:
  bool ValidSignature(const string& token);

private:
  static MacGenerator generator;
  WPEFramework::Web::JSONWebToken jwt;
};
