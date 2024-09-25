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

#include <interfaces/IAnalytics.h>

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
        class Analytics : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            // We do not allow this plugin to be copied !!
            Analytics(const Analytics&) = delete;
            Analytics& operator=(const Analytics&) = delete;

        public:
            Analytics():
                mConnectionId(0),
                mAnalytics(nullptr)
            {
                RegisterAll();
            }
            virtual ~Analytics()
            {
                UnregisterAll();
            }
            virtual const string Initialize(PluginHost::IShell* shell) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(Analytics)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IAnalytics, mAnalytics)
            END_INTERFACE_MAP

            static const string ANALYTICS_METHOD_SEND_EVENT;
            static const string ANALYTICS_METHOD_SET_SESSION_ID;
            static const string ANALYTICS_METHOD_SET_TIME_READY;

        private:
            void Deactivated(RPC::IRemoteConnection* connection);
            // JSONRPC methods
            void RegisterAll();
            void UnregisterAll();

            uint32_t SendEventWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t SetSessionIdWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t SetTimeReadyWrapper(const JsonObject& parameters, JsonObject& response);

        private:
            PluginHost::IShell* mService;
            uint32_t mConnectionId;
            Exchange::IAnalytics* mAnalytics;
        };
	} // namespace Plugin
} // namespace WPEFramework
