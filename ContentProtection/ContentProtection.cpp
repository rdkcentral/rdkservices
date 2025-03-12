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

#include "ContentProtection.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {
namespace Plugin {

    namespace {
        static Metadata<ContentProtection> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {});
    }

    SERVICE_REGISTRATION(ContentProtection, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    const string ContentProtection::Initialize(PluginHost::IShell* service)
    {
        string result;

        ASSERT(service != nullptr);

        _implementation = Core::Service<Implementation>::Create<
            Exchange::IContentProtection>(*this);
        Exchange::JContentProtection::Register(*this, _implementation);

        string token;
        auto security = service->QueryInterfaceByCallsign<
            PluginHost::IAuthenticate>("SecurityAgent");
        if (security != nullptr) {
            string payload = "http://localhost";
            auto ret = security->CreateToken(
                static_cast<uint16_t>(payload.length()),
                reinterpret_cast<const uint8_t*>(payload.c_str()),
                token);
            if (ret != Core::ERROR_NONE) {
                SYSLOG(Logging::Startup,
                    (_T("Couldn't create token: %d"), ret));
            }
            security->Release();
        }

        Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"),
            (_T("127.0.0.1:9998")));
        _secManager = Core::ProxyType<JSONRPCLink>::Create(
            _T("org.rdk.SecManager"), _T(""), "token=" + token);
        _watermark = Core::ProxyType<JSONRPCLink>::Create(
            _T("org.rdk.Watermark"), _T(""), "token=" + token);
        Subscribe();

        return result;
    }

    void ContentProtection::Deinitialize(PluginHost::IShell*)
    {
        Unsubscribe();
        Exchange::JContentProtection::Unregister(*this);
        if (_implementation != nullptr) {
            _implementation->Release();
        }
    }

    string ContentProtection::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
