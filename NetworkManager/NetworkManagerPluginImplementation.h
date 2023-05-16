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

#include <mutex>

#include "Module.h"

#include <interfaces/INetworkManagerPlugin.h>


namespace WPEFramework
{
    namespace Plugin
    {
        class NetworkManagerPluginImplementation : public Exchange::INetworkManagerPlugin
        {
        public:
            NetworkManagerPluginImplementation();
            ~NetworkManagerPluginImplementation() override;

            // Do not allow copy/move constructors
            NetworkManagerPluginImplementation(const NetworkManagerPluginImplementation &) = delete;
            NetworkManagerPluginImplementation &operator=(const NetworkManagerPluginImplementation &) = delete;

            BEGIN_INTERFACE_MAP(NetworkManagerPluginImplementation)
            INTERFACE_ENTRY(Exchange::INetworkManagerPlugin)
            END_INTERFACE_MAP

        public:
            // Implement the main methods from INetworkManagerPlugin
            uint32_t Greet(const string &message, string &result /* @out */) override;
            uint32_t Echo(const string &message, string &result /* @out */) override;
            uint32_t isConnectedToInternet(const string &message, string &result /* @out */);
            //uint32_t getIPSettings(const JsonObject& parameters, JsonObject& response) ;
            uint32_t getIPSettings(const string &message, string &result /* @out */) ;

        public:
            // Handle Notification registration/removal
            uint32_t Register(INetworkManagerPlugin::INotification *notification) override;
            uint32_t Unregister(INetworkManagerPlugin::INotification *notification) override;

        private:
            std::list<Exchange::INetworkManagerPlugin::INotification *> _notificationCallbacks;
            std::mutex _notificationMutex;

            const std::vector<std::string> _greetings;
        };
    }
}
