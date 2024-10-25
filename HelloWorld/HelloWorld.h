/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
#include <interfaces/json/JsonData_HelloWorld.h>
#include <interfaces/json/JHelloWorld.h>
#include <interfaces/IHelloWorld.h>
#include "UtilsLogging.h"
#include "tracing/Logging.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {

    class HelloWorld: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {
     private:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IHelloWorld::INotification
        {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
            explicit Notification(HelloWorld* parent)
                : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification()
                {
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IHelloWorld::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

                void Activated(RPC::IRemoteConnection*) override
                {
                    LOGINFO("HelloWorld Notification Activated");
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                   LOGINFO("HelloWorld Notification Deactivated");
                   _parent.Deactivated(connection);
                }

                void onEcho(const string& message) override
                {
                    LOGINFO("Helloworld echo: %s\n", message.c_str());
                    Exchange::JHelloWorld::Event::OnEcho(_parent, message);
                }

            private:
                HelloWorld& _parent;
        };

        public:
            // We do not allow this plugin to be copied !!
            HelloWorld(const HelloWorld&) = delete;
            HelloWorld& operator=(const HelloWorld&) = delete;

            HelloWorld();
            virtual ~HelloWorld();

            BEGIN_INTERFACE_MAP(HelloWorld)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IHelloWorld, _helloWorld)
            END_INTERFACE_MAP

            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell* service) override;
            string Information() const override;

        private:
            void Deactivated(RPC::IRemoteConnection* connection);

        private:
            PluginHost::IShell* _service{};
            uint32_t _connectionId{};
            Exchange::IHelloWorld* _helloWorld{};
            Core::Sink<Notification> _helloWorldNotification;
    };

} // namespace Plugin
} // namespace WPEFramework
