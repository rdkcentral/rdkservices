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

#include "CENCParser.h"

namespace WPEFramework {

namespace Plugin {

    /* static */ const uint8_t CommonEncryptionData::PSSHeader[] = { 0x70, 0x73, 0x73, 0x68 };
    /* static */ const uint8_t CommonEncryptionData::CommonEncryption[] = { 0x10, 0x77, 0xef, 0xec, 0xc0, 0xb2, 0x4d, 0x02, 0xac, 0xe3, 0x3c, 0x1e, 0x52, 0xe2, 0xfb, 0x4b };
    /* static */ const uint8_t CommonEncryptionData::PlayReady[] = { 0x9a, 0x04, 0xf0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xab, 0x92, 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95 };
    /* static */ const uint8_t CommonEncryptionData::WideVine[] = { 0xed, 0xef, 0x8b, 0xa9, 0x79, 0xd6, 0x4a, 0xce, 0xa3, 0xc8, 0x27, 0xdc, 0xd5, 0x1d, 0x21, 0xed };
    /* static */ const uint8_t CommonEncryptionData::ClearKey[] = { 0x58, 0x14, 0x7e, 0xc8, 0x04, 0x23, 0x46, 0x59, 0x92, 0xe6, 0xf5, 0x2c, 0x5c, 0xe8, 0xc3, 0xcc };
    /* static */ const    char CommonEncryptionData::JSONKeyIds[] = "{\"kids\":";
}
} // namespace WPEFramework::Plugin
