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
#include "jsonrpc/Watermark.h"

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

        SYSLOG(Logging::Startup,
            (_T("ContentProtection::Initialize: PID=%u"), getpid()));

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

        JsonRpc::SecManager* secManager = nullptr;
        auto dispatcher = service->QueryInterfaceByCallsign<
            PluginHost::IDispatcher>("SecManager");
        if (dispatcher == nullptr) {
            result = (_T("Couldn't query interface"));
        } else {
            secManager = Core::Service<JsonRpc::SecManager>::Create<
                JsonRpc::SecManager>(dispatcher, token);
            dispatcher->Release();
        }

        Exchange::IWatermark* watermark = nullptr;
        dispatcher = service->QueryInterfaceByCallsign<
            PluginHost::IDispatcher>("Watermark");
        if (dispatcher == nullptr) {
            result = (_T("Couldn't query interface"));
        } else {
            watermark = Core::Service<JsonRpc::Watermark>::Create<
                Exchange::IWatermark>(dispatcher, token);
            dispatcher->Release();
        }

        if ((secManager != nullptr) && (watermark != nullptr)) {
            _contentProtection = Core::Service<Implementation>::Create<
                Exchange::IContentProtection>(secManager, watermark);
            Exchange::JContentProtection::Register(*this, _contentProtection);
        }

        if (secManager != nullptr) {
            secManager->Release();
        }

        if (watermark != nullptr) {
            watermark->Release();
        }

        return result;
    }

    void ContentProtection::Deinitialize(PluginHost::IShell* /*service*/)
    {
        SYSLOG(Logging::Shutdown, (_T("ContentProtection::Deinitialize")));

        Exchange::JContentProtection::Unregister(*this);

        if (_contentProtection != nullptr) {
            _contentProtection->Release();
        }

        SYSLOG(Logging::Shutdown, (_T("ContentProtection de-initialised")));
    }

    string ContentProtection::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
