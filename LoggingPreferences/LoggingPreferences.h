/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "Module.h"
#include "libIBus.h"

namespace WPEFramework {

    namespace Plugin {

        class LoggingPreferences : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            LoggingPreferences(const LoggingPreferences&) = delete;
            LoggingPreferences& operator=(const LoggingPreferences&) = delete;

            //Begin methods
            uint32_t isKeystrokeMaskEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t setKeystrokeMaskEnabled(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onKeystrokeMaskEnabledChange(bool enabled);
            //End events

        public:
            LoggingPreferences();
            virtual ~LoggingPreferences();

            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(LoggingPreferences)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            void InitializeIARM();
            void DeinitializeIARM();

        public:
            static LoggingPreferences* _instance;

        };
    } // namespace Plugin
} // namespace WPEFramework
