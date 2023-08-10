/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#pragma once

#include "Module.h"

// Include the interface we created
#include <interfaces/INetworkManagerPlugin.h>
#include <interfaces/json/JsonData_NetworkManagerPlugin.h>

#include <mutex>

namespace WPEFramework
{
    namespace Plugin
    {
        using namespace JsonData::NetworkManagerPlugin;

        /**
         * NetworkManager plugin that exposes an API over both COM-RPC and JSON-RPC
         *
         */
        class NetworkManagerPlugin : public PluginHost::IPlugin, public PluginHost::JSONRPC
        {
            /**
             * Our notification handling code
             *
             * Handle both the Activate/Deactivate notifications and provide a handler
             * for notifications raised by the COM-RPC API
             */
            class Notification : public RPC::IRemoteConnection::INotification,
                                 public Exchange::INetworkManagerPlugin::INotification
            {
            private:
                Notification() = delete;
                Notification(const Notification &) = delete;
                Notification &operator=(const Notification &) = delete;

            public:
                explicit Notification(NetworkManagerPlugin *parent)
                    : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }
                virtual ~Notification() override
                {
                }

            public:
                void SomethingHappend(const Source event) override
                {
                    _parent.SomethingHappend(event);
                }

                // The activated/deactived methods are part of the RPC::IRemoteConnection::INotification
                // interface. These are triggered when Thunder detects a connection/disconnection over the
                // COM-RPC link.
                void Activated(RPC::IRemoteConnection * /* connection */) override
                {
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                    // Something's caused the remote connection to be lost - this could be a crash
                    // on the remote side so deactivate ourselves
                    _parent.Deactivated(connection);
                }

                // Build QueryInterface implementation, specifying all possible interfaces we implement
                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::INetworkManagerPlugin::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

            private:
                NetworkManagerPlugin &_parent;
            };

        public:
            NetworkManagerPlugin();
            ~NetworkManagerPlugin() override;

            // Do not allow copy/move constructors
            NetworkManagerPlugin(const NetworkManagerPlugin &) = delete;
            NetworkManagerPlugin &operator=(const NetworkManagerPlugin &) = delete;

            // Build QueryInterface implementation, specifying all possible interfaces we implement
            // This is necessary so that consumers can discover which plugin implements what interface
            BEGIN_INTERFACE_MAP(NetworkManagerPlugin)

            // Which interfaces do we implement?
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)

            // We need to tell Thunder that this plugin provides the INetworkManagerPlugin interface, but
            // since it's not actually implemented here we tell Thunder where it can
            // find the real implementation
            // This allows other components to call QueryInterface<INetworkManagerPlugin>() and
            // receive the actual implementation (which could be in-process or out-of-process)
            INTERFACE_AGGREGATE(Exchange::INetworkManagerPlugin, _networkManagerPlugin)
            END_INTERFACE_MAP

        public:
            static const short API_VERSION_NUMBER_MAJOR;
            static const short API_VERSION_NUMBER_MINOR;

        public:
            // Implement the basic IPlugin interface that all plugins must implement
            const string Initialize(PluginHost::IShell *service) override;
            void Deinitialize(PluginHost::IShell *service) override;
            string Information() const override;

        private:
            // Notification/event handlers
            // Clean up when we're told to deactivate
            void Deactivated(RPC::IRemoteConnection *connection);

            // Our custom event
            void SomethingHappend(const Exchange::INetworkManagerPlugin::INotification::Source event);

        private:
            // JSON-RPC setup
            void RegisterAllMethods();
            void UnregisterAllMethods();

            // JSON-RPC methods (take JSON in, spit JSON back out)
            uint32_t Greeter(const GreeterParamsData &params, GreeterResultData &response);
            uint32_t Echo(const EchoParamsData &params, EchoResultData &response);
            uint32_t isConnectedToInternet(const EchoParamsData &params, EchoResultData &response);
            uint32_t getIPSettings(const GetIPSettingsParamsData& parameters, GetIPSettingsResultData& response);

        private:
            uint32_t _connectionId;
            PluginHost::IShell *_service;
            Exchange::INetworkManagerPlugin *_networkManagerPlugin;
            Core::Sink<Notification> _notification;
        };
    }
}
