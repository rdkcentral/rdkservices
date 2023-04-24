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

#include <future>
#include <atomic>

#include "dbus/DBusClient.h"

#include "../Module.h"
#include "../WifiManagerDefines.h"

using namespace WifiManagerImpl;

namespace WPEFramework {
    namespace Plugin {
        class WifiManagerState {
        public:
            WifiManagerState();
            virtual ~WifiManagerState();
            WifiManagerState(const WifiManagerState&) = delete;
            WifiManagerState& operator=(const WifiManagerState&) = delete;

            void Initialize();

            uint32_t getCurrentState(const JsonObject& parameters, JsonObject& response);
            uint32_t getConnectedSSID(const JsonObject& parameters, JsonObject& response) /* dropping const, since this can update m_latest_ssid */;
            uint32_t setEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedSecurityModes(const JsonObject& parameters, JsonObject& response);
            void setWifiStateCache(bool value,WifiState state) {/* not used */}
            static const std::string& getWifiInterfaceName();

        private:
            void statusChanged(const std::string& interface, InterfaceStatus status);
            void updateWifiStatus(InterfaceStatus status);
            bool fetchSsid(std::string& out_ssid);
            bool getWifiParams(std::map<std::string, std::string> &params);
            std::string getWifiRssi();

            std::atomic<WifiState> m_wifi_state {WifiState::DISABLED};

            static std::string fetchWifiInterfaceName();

            std::string m_latest_ssid;
        };
    }
}
