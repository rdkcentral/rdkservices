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

#ifndef __RIALTO_SERVER_MANAGER_H
#define __RIALTO_SERVER_MANAGER_H

#include "Module.h"
#include "RialtoServerManagerConfig.h"
#include <interfaces/IConfiguration.h>

#include <vector>

namespace WPEFramework {
namespace Plugin {

    class RialtoServerManager : public PluginHost::IPlugin {
    private:
        RialtoServerManager(const RialtoServerManager&) = delete;
        RialtoServerManager& operator=(const RialtoServerManager&) = delete;

        class Notification : public RPC::IRemoteConnection::INotification {
        private:
            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            explicit Notification(RialtoServerManager* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~Notification()
            {
            }

        public:
            virtual void Activated(RPC::IRemoteConnection*)
            {
            }
            virtual void Deactivated(RPC::IRemoteConnection* connection)
            {
                _parent.Deactivated(connection);
            }

            BEGIN_INTERFACE_MAP(Notification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            RialtoServerManager& _parent;
        };


    public:
        RialtoServerManager()
            : _notification(this) {}
        virtual ~RialtoServerManager() {}

    public:
        BEGIN_INTERFACE_MAP(OCDM)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        END_INTERFACE_MAP

    public:
        //  IPlugin methods
        virtual const string Initialize(PluginHost::IShell* service);
        virtual void Deinitialize(PluginHost::IShell* service);
        virtual string Information() const;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);

        bool setupRialtoEnvironment();
        void restoreEnvironment();
        bool prepareGstPlugins();

        void cleanUp(bool releaseObject);
        uint32_t _connectionId {};
        PluginHost::IShell* _service {nullptr};
        Exchange::IConfiguration* _object {nullptr};
        Core::Sink<Notification> _notification;

        RialtoServerManagerConfig::Config _config;
        std::vector<std::pair<string, Core::OptionalType<string>>> _environment;
        string _rialtoHome;
    };
} //namespace Plugin
} //namespace WPEFramework

#endif // __RIALTO_SERVER_MANAGER_H
