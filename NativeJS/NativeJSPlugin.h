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

#include "Module.h"
#include "UtilsLogging.h"
#include "UtilsJsonRpc.h"
#include <interfaces/json/JsonData_NativeJS.h>
#include <interfaces/json/JNativeJS.h>
#include <interfaces/INativeJS.h>

namespace WPEFramework {

    namespace Plugin {

        class NativeJS : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:
            class Config : public Core::JSON::Container
            {
                private:
                    Config(const Config&) = delete;
                    Config& operator=(const Config&) = delete;

                public:
                    Config()
                        : Core::JSON::Container()
                          , Display(_T(""))
                    {
                        Add(_T("embedthunderjs"), &EmbedThunderJS);
                        Add(_T("clientidentifier"), &Display);
                    }
                    ~Config()
                    {
                    }

                public:
                    Core::JSON::Boolean EmbedThunderJS;
                    Core::JSON::String Display;
            };

            BEGIN_INTERFACE_MAP(NativeJSPlugin)
            INTERFACE_AGGREGATE(Exchange::INativeJS, mNativeJS)
	    INTERFACE_ENTRY(PluginHost::IPlugin)
	    INTERFACE_ENTRY(PluginHost::IDispatcher)
	    END_INTERFACE_MAP

        public:
            NativeJS();
            virtual ~NativeJS();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

        public/*members*/:
            static NativeJS* _instance;

        public /*constants*/:
            static const string SERVICE_NAME;

        private/*registered methods (wrappers)*/:

            NativeJS(const NativeJS&) = delete;
            NativeJS& operator=(const NativeJS&) = delete;

            PluginHost::IShell* mService;
            uint32_t mConnectionId;
            Exchange::INativeJS* mNativeJS;
            Config mConfig;
        };
    } // namespace Plugin
} // namespace WPEFramework
