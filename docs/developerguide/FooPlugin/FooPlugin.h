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

#pragma once

#include "Module.h"

namespace WPEFramework
{
    namespace Plugin
    {
        class FooPlugin : public PluginHost::IPlugin, public PluginHost::JSONRPC
        {
        private:
            FooPlugin(const FooPlugin &) = delete;
            FooPlugin &operator=(const FooPlugin &) = delete;

        public:
            FooPlugin();
            virtual ~FooPlugin();

            // Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(FooPlugin)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            //   IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            virtual const string Initialize(PluginHost::IShell *service) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual string Information() const override;

        protected:
            void RegisterAll();
            void UnregisterAll();

            uint32_t endpoint_setValue(const JsonObject &parameters, JsonObject &response);

            void event_onValueChanged(const string &key, const string &value);
        };

    } // namespace Plugin
} // namespace WPEFramework
