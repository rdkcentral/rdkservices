/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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

#include "InterPluginRPC.h"
#include "ILinchpinPluginEvents.h"

namespace WPEFramework {
    namespace Plugin {

        class LinchpinPluginRPC : protected InterPluginRPC
        {
            bool                    m_isConnected {false};
            ILinchpinPluginEvents*  m_handlerLinchpinEvents {nullptr};
            string                  m_connectionID;
            int64_t                 m_publishType {0};
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> m_linchpinConnection;
            bool Disconnect();
            bool InternalSubscriptionManagement(const char* method, string& topic);
        public:
            LinchpinPluginRPC();
            LinchpinPluginRPC(ILinchpinPluginEvents* handler);
            LinchpinPluginRPC(PluginHost::IShell* service);
            ~LinchpinPluginRPC();
            bool Connect(const string& endpoint, const string& client, int64_t type, const string& token);
            bool Attach(PluginHost::IShell* service);
            bool Subscribe(string& topic);
            bool Unsubscribe(string& topic);
            bool FetchMessage(string& topic, string& pbevent, JsonObject& parameters);
            bool PublishMessage(const string& topic, const string& pbevent, const JsonObject& parameters);
            void onNotifyMessageReceived(const JsonObject& parameters);
            void onConnectionClosed(const JsonObject& parameters);
        };
    }
}
