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

#include <string>
#include <vector>
#include <memory>

namespace WPEFramework {
namespace Plugin {

class CookieJar
{
public:
    CookieJar();
    ~CookieJar();

    bool IsStale() const { return _refreshed.GetState() == false; }
    void MarkAsStale() { _refreshed.SetState( false ); }
    bool WaitForRefresh(int timeout_ms) const { return _refreshed.WaitState(true, timeout_ms); }

    // Get/Set cookies
    void SetCookies(std::vector<std::string> &&);
    std::vector<std::string> GetCookies() const;

    // Pack/unack cookies for storing in the "cloud"
    uint32_t Pack(uint32_t& version, uint32_t& checksum, string& payload) const;
    uint32_t Unpack(const uint32_t version, const uint32_t checksum, const string& payload);

private:
    Core::StateTrigger<bool> _refreshed { false };
    std::vector<std::string> _cookies;

    struct CookieJarPrivate;
    mutable std::unique_ptr<CookieJarPrivate> _priv;
};

} // namespace Plugin
} // namespace WPEFramework
