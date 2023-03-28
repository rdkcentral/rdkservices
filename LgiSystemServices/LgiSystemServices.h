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

#ifndef LGISYSTEMSERVICES_H
#define LGISYSTEMSERVICES_H

#include <map>

#include "Module.h"
#include "tracing/Logging.h"

namespace WPEFramework {
    namespace Plugin {

        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.

        class LgiSystemServices : public PluginHost::IPlugin, public PluginHost::JSONRPC {

        public:
            LgiSystemServices();
            virtual ~LgiSystemServices();

            // We do not allow this plugin to be copied !!
            LgiSystemServices(const LgiSystemServices&) = delete;
            LgiSystemServices& operator=(const LgiSystemServices&) = delete;

            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(LgiSystemServices)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            uint32_t getDeviceInfo(const JsonObject& parameters, JsonObject& response);
            uint32_t getTimeZoneDST(const JsonObject& parameters, JsonObject& response);

        private:
            std::map<std::string, std::string> m_deviceInfo;

        }; /* end of system service class */
    } /* end of plugin */
} /* end of wpeframework */

#endif //LGISYSTEMSERVICES_H
