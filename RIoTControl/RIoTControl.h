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

#include <string>
#include <atomic>

#include "Module.h"

namespace WPEFramework
{
    namespace Plugin
    {
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
        class RIoTControl : public PluginHost::IPlugin, public PluginHost::JSONRPC
        {
        private:
            // We do not allow this plugin to be copied !!
            RIoTControl(const RIoTControl &) = delete;
            RIoTControl &operator=(const RIoTControl &) = delete;

            int m_apiVersionNumber;
            std::string remote_addr;

            // Available functions

            uint32_t getAvailableDevicesWrapper(const JsonObject &parameters, JsonObject &response);
            uint32_t getDeviceProperties(const JsonObject &parameters, JsonObject &response);
            uint32_t getDeviceProperty(const JsonObject &parameters, JsonObject &response);
            uint32_t sendCommand(const JsonObject &parameters, JsonObject &response);

            bool initializeIPC();

        public:
            RIoTControl();
            virtual ~RIoTControl();

            // Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(RIoTControl)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            // IPlugin methods
            virtual const std::string Initialize(PluginHost::IShell *service) override;
            virtual void Deinitialize(PluginHost::IShell *service) override;
            virtual std::string Information() const override;
        };
    } // namespace Plugin
} // namespace WPEFramework