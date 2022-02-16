/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

namespace WPEFramework {
namespace Plugin {

struct IStoreListing : virtual public Core::IUnknown {
    virtual uint32_t GetKeys(const string &ns, std::vector<string> &keys /* @out */) = 0;
    virtual uint32_t GetNamespaces(std::vector<string> &namespaces /* @out */) = 0;
    virtual uint32_t GetStorageSize(std::map<string, uint64_t> &namespaceSizes /* @out */) = 0;
};

} // namespace Plugin
} // namespace WPEFramework
