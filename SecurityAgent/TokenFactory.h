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

#ifdef ENABLE_SECAPI
#include "SecapiToken.h"
#endif

#include <memory>

class JWTFactory {
private:
  JWTFactory(const JWTFactory &) = delete;
  JWTFactory &operator=(const JWTFactory &) = delete;
  JWTFactory() {
    #ifndef ENABLE_SECAPI
    for (uint8_t index = 0; index < sizeof(_secretKey); index++) {
      WPEFramework::Crypto::Random(_secretKey[index]);
    }
    #endif
  }

public:
  static JWTFactory &Instance() {
    static JWTFactory _instance;
    return (_instance);
  }

  #ifdef ENABLE_SECAPI
  typedef JWTSecApi ELEMENT;
  #else
  typedef WPEFramework::Web::JSONWebToken ELEMENT;
  #endif

  inline std::unique_ptr<ELEMENT> Element() {
    return std::unique_ptr<ELEMENT>(new ELEMENT
        #ifndef ENABLE_SECAPI
        (WPEFramework::Web::JSONWebToken::SHA256, sizeof(_secretKey), _secretKey)
        #endif
        );
  }

private:
  #ifndef ENABLE_SECAPI
  uint8_t _secretKey[WPEFramework::Crypto::SHA256::Length];
  #endif
};
