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
#include <interfaces/json/JsonData_AppManager.h>
#include <interfaces/json/JAppManager.h>
#include <interfaces/IAppManager.h>
#include "UtilsLogging.h"
#include "tracing/Logging.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {

    class AppManager: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {
     private:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IAppManager::INotification
        {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
            explicit Notification(AppManager* parent)
                : _parent(*parent)
                {
                    ASSERT(parent != nullptr);
                }

                virtual ~Notification()
                {
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IAppManager::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

                void Activated(RPC::IRemoteConnection*) override
                {
                    LOGINFO("AppManager Notification Activated");
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                   LOGINFO("AppManager Notification Deactivated");
                   _parent.Deactivated(connection);
                }

                void onAppStateChanged(const string& appId, const uint32_t appInstanceId, const Exchange::IAppManager::LifecycleState newState, const Exchange::IAppManager::LifecycleState oldState) override
                {
                    LOGINFO("AppManager on appstate changed: %s\n", appId.c_str());
                    Exchange::JAppManager::Event::OnAppStateChanged(_parent, appId, appInstanceId, newState, oldState);
                }

            private:
                AppManager& _parent;
        };

        public:
            // We do not allow this plugin to be copied !!
            AppManager(const AppManager&) = delete;
            AppManager& operator=(const AppManager&) = delete;

            AppManager();
            virtual ~AppManager();

            BEGIN_INTERFACE_MAP(AppManager)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IAppManager, mAppManager)
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
            Exchange::IAppManager* mAppManager{};
            Core::Sink<Notification> mAppManagerNotification;
    };

} // namespace Plugin
} // namespace WPEFramework
