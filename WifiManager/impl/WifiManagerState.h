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
            WifiManagerState() = default;
            virtual ~WifiManagerState() = default;
            WifiManagerState(const WifiManagerState&) = delete;
            WifiManagerState& operator=(const WifiManagerState&) = delete;

            uint32_t getCurrentState(const JsonObject& parameters, JsonObject& response) const;
            uint32_t getConnectedSSID(const JsonObject& parameters, JsonObject& response) const;
            uint32_t setEnabled(const JsonObject& parameters, JsonObject& response);
        };
    }
}
