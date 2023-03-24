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

// Forward declaration
typedef int IARM_EventId_t;

namespace WPEFramework {
    namespace Plugin {
        /**
         * The internal implementation of the functionality in WifiManager relating to event generation.
         *
         */
        class WifiManagerEvents {
        public:
            WifiManagerEvents() = default;
            virtual ~WifiManagerEvents() = default;
            WifiManagerEvents(WifiManagerEvents const&) = delete;
            WifiManagerEvents& operator=(WifiManagerEvents const&) = delete;

            std::string Initialize(WPEFramework::PluginHost::IShell* service);
            void Deinitialize(WPEFramework::PluginHost::IShell* service);
        };
    } // namespace Plugin
} // namespace WPEFramework
