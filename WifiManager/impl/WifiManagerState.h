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

#include "../Module.h"
#include "../WifiManagerDefines.h"

namespace WPEFramework {
    namespace Plugin {
        class WifiManagerState {
        public:
            WifiManagerState();
            virtual ~WifiManagerState();
            WifiManagerState(const WifiManagerState&) = delete;
            WifiManagerState& operator=(const WifiManagerState&) = delete;

            uint32_t getCurrentState(const JsonObject& parameters, JsonObject& response);
            bool getConnectedSSID(const JsonObject& parameters, JsonObject& response);
            uint32_t setEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedSecurityModes(const JsonObject& parameters, JsonObject& response);
            void setWifiStateCache(bool value,WifiState state);
            void resetWifiStateConnectedCache(bool value);

            std::atomic<bool> m_useWifiStateCache;
            WifiState m_wifiStateCache;
            std::atomic<bool> m_useWifiConnectedCache;
            std::string m_ConnectedSSIDCache;
            std::string m_ConnectedBSSIDCache;
            int m_ConnectedSecurityModeCache;

	private:
	    std::map<std::string, std::string> retrieveValues(const char *, char *, size_t );
        };
    }
}
