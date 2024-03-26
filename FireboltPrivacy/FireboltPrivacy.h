/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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

#ifndef PLUGIN_FIREBOLTPRIVACY_H
#define PLUGIN_FIREBOLTPRIVACY_H

#include "Module.h"

#include <interfaces/IFireboltPrivacy.h>
#include <interfaces/json/JFireboltPrivacy.h>
#include <interfaces/json/JsonData_FireboltPrivacy.h>
#include <syslog.h>

namespace WPEFramework {
namespace Plugin {

    class FireboltPrivacy : public PluginHost::IPlugin,
//                           public PluginHost::IWeb,
                           public PluginHost::JSONRPC {
    private:
        class ConnectionNotification : public RPC::IRemoteConnection::INotification {
        public:
            ConnectionNotification() = delete;
            ConnectionNotification(const ConnectionNotification&) = delete;
            ConnectionNotification& operator=(const ConnectionNotification&) = delete;

            explicit ConnectionNotification(FireboltPrivacy* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }
            ~ConnectionNotification() override = default;

        public:
            void Activated(RPC::IRemoteConnection*  /* connection */) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }
            void Terminated (RPC::IRemoteConnection* /* connection */) override
            {
            }

            BEGIN_INTERFACE_MAP(ConnectionNotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

        private:
            FireboltPrivacy& _parent;
        };

        class FireboltPrivacyNotification : public Exchange::IFireboltPrivacy::INotification {
        public:
            explicit FireboltPrivacyNotification(FireboltPrivacy* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }

            ~FireboltPrivacyNotification() override
            {
            }

            FireboltPrivacyNotification() = delete;
            FireboltPrivacyNotification(const FireboltPrivacyNotification&) = delete;
            FireboltPrivacyNotification& operator=(const FireboltPrivacyNotification&) = delete;

            void OnAllowResumePointsChanged(const bool allowResumePoint) override
            {
                Exchange::JFireboltPrivacy::Event::OnAllowResumePointsChanged(_parent, allowResumePoint);
            }

            BEGIN_INTERFACE_MAP(FireboltPrivacyNotification)
                INTERFACE_ENTRY(Exchange::IFireboltPrivacy::INotification)
            END_INTERFACE_MAP

        private:
            FireboltPrivacy& _parent;
        };

    public:
        FireboltPrivacy(const FireboltPrivacy&) = delete;
        FireboltPrivacy& operator=(const FireboltPrivacy&) = delete;

        FireboltPrivacy();
        ~FireboltPrivacy() override = default;

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
        BEGIN_INTERFACE_MAP(FireboltPrivacy)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            //INTERFACE_ENTRY(PluginHost::IWeb)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IFireboltPrivacy, _fireboltPrivacy)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;


    private:
        void Deactivated(RPC::IRemoteConnection* connection);

    private:
        uint32_t _connectionId;
        PluginHost::IShell* _service;
        Exchange::IFireboltPrivacy* _fireboltPrivacy;
        Core::Sink<ConnectionNotification> _connectionNotification;
        Core::Sink<FireboltPrivacyNotification> _fireboltPrivacyNotification;
        
    };

} // namespace Plugin
} // namespace WPEFramework

#endif // PLUGIN_FIREBOLTPRIVACY_H
