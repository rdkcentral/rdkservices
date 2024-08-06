/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#pragma once

#include "Module.h"
#include <interfaces/json/JStore2.h>

namespace WPEFramework {
namespace Plugin {

    class CloudStore : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config()
                : Core::JSON::Container()
            {
                Add(_T("uri"), &Uri);
            }

        public:
            Core::JSON::String Uri;
        };

        class Store2Notification : public Exchange::IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(CloudStore& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                Exchange::JStore2::Event::ValueChanged(_parent, scope, ns, key, value);
            }

            BEGIN_INTERFACE_MAP(Store2Notification)
            INTERFACE_ENTRY(Exchange::IStore2::INotification)
            END_INTERFACE_MAP

        private:
            CloudStore& _parent;
        };

        class RemoteConnectionNotification : public RPC::IRemoteConnection::INotification {
        private:
            RemoteConnectionNotification() = delete;
            RemoteConnectionNotification(const RemoteConnectionNotification&) = delete;
            RemoteConnectionNotification& operator=(const RemoteConnectionNotification&) = delete;

        public:
            explicit RemoteConnectionNotification(CloudStore& parent)
                : _parent(parent)
            {
            }
            ~RemoteConnectionNotification() override = default;

            BEGIN_INTERFACE_MAP(RemoteConnectionNotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

            void Activated(RPC::IRemoteConnection*) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                if (connection->Id() == _parent._connectionId) {
                    ASSERT(_parent._service != nullptr);
                    Core::IWorkerPool::Instance().Schedule(
                        Core::Time::Now(),
                        PluginHost::IShell::Job::Create(
                            _parent._service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
                }
            }

        private:
            CloudStore& _parent;
        };

    private:
        CloudStore(const CloudStore&) = delete;
        CloudStore& operator=(const CloudStore&) = delete;

    public:
        CloudStore()
            : PluginHost::JSONRPC()
            , _service(nullptr)
            , _connectionId(0)
            , _store2(nullptr)
            , _store2Sink(*this)
            , _notification(*this)
        {
        }
        ~CloudStore() override = default;

        BEGIN_INTERFACE_MAP(CloudStore)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IStore2, _store2)
        END_INTERFACE_MAP

    public:
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        Config _config;
        PluginHost::IShell* _service;
        uint32_t _connectionId;
        Exchange::IStore2* _store2;
        Core::Sink<Store2Notification> _store2Sink;
        Core::Sink<RemoteConnectionNotification> _notification;
    };

} // namespace Plugin
} // namespace WPEFramework
