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

#include <string>

// Forward declaration
typedef int IARM_EventId_t;

namespace WPEFramework {
    namespace Plugin {
        /**
         * The internal implementation of the functionality in WifiManager relating to scanning for access points.
         *
         */
        class WifiManagerScan {
        public:
            WifiManagerScan() = default;
            virtual ~WifiManagerScan() = default;
            WifiManagerScan(WifiManagerScan const&) = delete;
            WifiManagerScan& operator=(WifiManagerScan const&) = delete;

            std::string Initialize(PluginHost::IShell* service);
            void Deinitialize(PluginHost::IShell* service);

            uint32_t startScan(const JsonObject& parameters, JsonObject& response) const;
            uint32_t stopScan(const JsonObject& parameters, JsonObject& response);

        private:
            uint32_t getAvailableSSIDsAsync(const JsonObject& parameters, JsonObject& response) const;
            uint32_t getAvailableSSIDsAsyncIncr(const JsonObject& parameters, JsonObject& response) const;

            static void iarmEventHandler(char const* owner, IARM_EventId_t eventId, void* data, size_t len);

            struct Filter {
                bool onSsid = false;
                std::string ssid;
                bool onFrequency = false;
                std::string frequency;
            };

            static void applyFilter(JsonArray &object, const WifiManagerScan::Filter &filter);
            static Filter filter;
        };
    } // namespace Plugin
} // namespace WPEFramework
