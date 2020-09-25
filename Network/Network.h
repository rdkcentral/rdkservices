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

#include <cjson/cJSON.h>
#include <string>

#include "Module.h"
#include "NetUtils.h"
#include "utils.h"
#include "upnpdiscoverymanager.h"


// Define this to use netlink calls (where there may be an alternative method but netlink could provide
// the information or perform the action required)
//#define USE_NETLINK

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
        class Network : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            Network(const Network&) = delete;
            Network& operator=(const Network&) = delete;

            //Begin methods
            uint32_t getQuirks(const JsonObject& parameters, JsonObject& response);

            // Network_API_Version_1
            uint32_t getInterfaces(const JsonObject& parameters, JsonObject& response);
            uint32_t isInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t setInterfaceEnabled(const JsonObject& parameters, JsonObject& response);
            uint32_t getDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t setDefaultInterface(const JsonObject& parameters, JsonObject& response);
            uint32_t getStbIp(const JsonObject& parameters, JsonObject& response);
            uint32_t setApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t trace(const JsonObject& parameters, JsonObject& response);
            uint32_t traceNamedEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t getNamedEndpoints(const JsonObject& parameters, JsonObject& response);
            uint32_t ping(const JsonObject& parameters, JsonObject& response);
            uint32_t pingNamedEndpoint(const JsonObject& parameters, JsonObject& response);
            uint32_t setIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response);
            uint32_t getSTBIPFamily(const JsonObject& parameters, JsonObject& response);

            void onInterfaceEnabledStatusChanged(std::string interface, bool enabled);
            void onInterfaceConnectionStatusChanged(std::string interface, bool connected);
            void onInterfaceIPAddressChanged(std::string interface, std::string ipv6Addr, std::string ipv4Addr, bool acquired);
            void onDefaultInterfaceChanged(std::string oldInterface, std::string newInterface);

            static void eventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Internal methods
            bool _getDefaultInterface(std::string& interface, std::string& gateway);

            bool _doTrace(std::string &endpoint, int packets, JsonObject& response);
            bool _doTraceNamedEndpoint(std::string &endpointName, int packets, JsonObject& response);

            JsonObject _doPing(std::string endPoint, int packets);
            JsonObject _doPingNamedEndpoint(std::string endpointName, int packets);

        public:
            Network();
            virtual ~Network();

            //Build QueryInterface implementation, specifying all possible interfaces to be returned.
            BEGIN_INTERFACE_MAP(Network)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            //IPlugin methods
            virtual const std::string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual std::string Information() const override;

        public:
            static Network *_instance;
            static Network *getInstance() {return _instance;}

        private:
            uint32_t m_apiVersionNumber;
            NetUtils m_netUtils;
        };
    } // namespace Plugin
} // namespace WPEFramework
