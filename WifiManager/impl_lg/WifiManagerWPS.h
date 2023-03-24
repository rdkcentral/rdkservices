/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "../WifiManagerDefines.h"

namespace WPEFramework {

    namespace Plugin {
        // Implementation of wifi protected setup (WPS) WiFiManger API
        class WifiManagerWPS {
        public:
            WifiManagerWPS();
            virtual ~WifiManagerWPS();
            WifiManagerWPS(const WifiManagerWPS&) = delete;
            WifiManagerWPS& operator=(const WifiManagerWPS&) = delete;

            uint32_t initiateWPSPairing(const JsonObject& parameters, JsonObject& response);
            uint32_t initiateWPSPairing2(const JsonObject& parameters, JsonObject& response);
            uint32_t cancelWPSPairing(const JsonObject& parameters, JsonObject& response);
            uint32_t saveSSID (const JsonObject& parameters, JsonObject& response);
            uint32_t clearSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t getPairedSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t getPairedSSIDInfo(const JsonObject& parameters, JsonObject& response);
            uint32_t isPaired(const JsonObject& parameters, JsonObject& response);
            void updateWifiWPSCache(bool value);

            std::atomic<bool> m_useCachePairedSSID;
            std::string m_cachePairedSSID;
            std::string m_cachePairedBSSID;
        };
    } // namespace Plugin
} // namespace WPEFramework
