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
#include <interfaces/IStore.h>
#include <interfaces/IStore2.h>
#include <interfaces/IStoreCache.h>
#include <interfaces/json/JsonData_PersistentStore.h>

namespace Thunder {
namespace Plugin {

    class PersistentStore : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config()
                : Core::JSON::Container()
                , MaxSize(0)
                , MaxValue(0)
                , Limit(0)
            {
                Add(_T("uri"), &Uri);
                Add(_T("path"), &Path);
                Add(_T("legacypath"), &LegacyPath);
                Add(_T("key"), &Key);
                Add(_T("maxsize"), &MaxSize);
                Add(_T("maxvalue"), &MaxValue);
                Add(_T("limit"), &Limit);
            }

        public:
            Core::JSON::String Uri;
            Core::JSON::String Path;
            Core::JSON::String LegacyPath;
            Core::JSON::String Key;
            Core::JSON::DecUInt64 MaxSize;
            Core::JSON::DecUInt64 MaxValue;
            Core::JSON::DecUInt64 Limit;
        };

        class Store2Notification : public Exchange::IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(PersistentStore& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                JsonData::PersistentStore::SetValueParamsData params;
                params.Scope = JsonData::PersistentStore::ScopeType(scope);
                params.Namespace = ns;
                params.Key = key;
                params.Value = value;

                _parent.event_onValueChanged(params);
            }

            BEGIN_INTERFACE_MAP(Store2Notification)
            INTERFACE_ENTRY(Exchange::IStore2::INotification)
            END_INTERFACE_MAP

        private:
            PersistentStore& _parent;
        };

        class RemoteConnectionNotification : public RPC::IRemoteConnection::INotification {
        private:
            RemoteConnectionNotification() = delete;
            RemoteConnectionNotification(const RemoteConnectionNotification&) = delete;
            RemoteConnectionNotification& operator=(const RemoteConnectionNotification&) = delete;

        public:
            explicit RemoteConnectionNotification(PersistentStore& parent)
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
            PersistentStore& _parent;
        };

    private:
        PersistentStore(const PersistentStore&) = delete;
        PersistentStore& operator=(const PersistentStore&) = delete;

    public:
        PersistentStore()
            : PluginHost::JSONRPC()
            , _service(nullptr)
            , _connectionId(0)
            , _store(nullptr)
            , _store2(nullptr)
            , _storeCache(nullptr)
            , _storeInspector(nullptr)
            , _storeLimit(nullptr)
            , _store2Sink(*this)
            , _notification(*this)
        {
            RegisterAll();
        }
        ~PersistentStore() override
        {
            UnregisterAll();
        }

        BEGIN_INTERFACE_MAP(PersistentStore)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IStore, _store)
        INTERFACE_AGGREGATE(Exchange::IStore2, _store2)
        INTERFACE_AGGREGATE(Exchange::IStoreCache, _storeCache)
        INTERFACE_AGGREGATE(Exchange::IStoreInspector, _storeInspector)
        INTERFACE_AGGREGATE(Exchange::IStoreLimit, _storeLimit)
        END_INTERFACE_MAP

    public:
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        void RegisterAll();
        void UnregisterAll();

        uint32_t endpoint_setValue(const JsonData::PersistentStore::SetValueParamsData& params, JsonData::PersistentStore::DeleteKeyResultInfo& response);
        uint32_t endpoint_getValue(const JsonData::PersistentStore::DeleteKeyParamsInfo& params, JsonData::PersistentStore::GetValueResultData& response);
        uint32_t endpoint_deleteKey(const JsonData::PersistentStore::DeleteKeyParamsInfo& params, JsonData::PersistentStore::DeleteKeyResultInfo& response);
        uint32_t endpoint_deleteNamespace(const JsonData::PersistentStore::DeleteNamespaceParamsInfo& params, JsonData::PersistentStore::DeleteKeyResultInfo& response);
        uint32_t endpoint_getKeys(const JsonData::PersistentStore::DeleteNamespaceParamsInfo& params, JsonData::PersistentStore::GetKeysResultData& response);
        uint32_t endpoint_getNamespaces(const JsonData::PersistentStore::GetNamespacesParamsInfo& params, JsonData::PersistentStore::GetNamespacesResultData& response);
        uint32_t endpoint_getStorageSize(const JsonData::PersistentStore::GetNamespacesParamsInfo& params, JsonObject& response); // Deprecated
        uint32_t endpoint_getStorageSizes(const JsonData::PersistentStore::GetNamespacesParamsInfo& params, JsonData::PersistentStore::GetStorageSizesResultData& response);
        uint32_t endpoint_flushCache(JsonData::PersistentStore::DeleteKeyResultInfo& response);
        uint32_t endpoint_getNamespaceStorageLimit(const JsonData::PersistentStore::DeleteNamespaceParamsInfo& params, JsonData::PersistentStore::GetNamespaceStorageLimitResultData& response);
        uint32_t endpoint_setNamespaceStorageLimit(const JsonData::PersistentStore::SetNamespaceStorageLimitParamsData& params);

        void event_onValueChanged(const JsonData::PersistentStore::SetValueParamsData& params)
        {
            Notify(_T("onValueChanged"), params);
        }

    private:
        Config _config;
        PluginHost::IShell* _service;
        uint32_t _connectionId;
        Exchange::IStore* _store;
        Exchange::IStore2* _store2;
        Exchange::IStoreCache* _storeCache;
        Exchange::IStoreInspector* _storeInspector;
        Exchange::IStoreLimit* _storeLimit;
        Core::Sink<Store2Notification> _store2Sink;
        Core::Sink<RemoteConnectionNotification> _notification;
    };

} // namespace Plugin
} // namespace Thunder
