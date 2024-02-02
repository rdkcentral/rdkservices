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

namespace WPEFramework {
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
            explicit Store2Notification(PersistentStore* parent)
                : _parent(*parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            // IStore2::INotification methods

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

        // Deprecated
        class Store : public Exchange::IStore {
        private:
            Store(const Store&) = delete;
            Store& operator=(const Store&) = delete;

        private:
            class Store2Notification : public Exchange::IStore2::INotification {
            private:
                Store2Notification(const Store2Notification&) = delete;
                Store2Notification& operator=(const Store2Notification&) = delete;

            public:
                explicit Store2Notification(Store* parent)
                    : _parent(*parent)
                {
                }
                ~Store2Notification() override = default;

            public:
                // IStore2::INotification methods

                void ValueChanged(const Exchange::IStore2::ScopeType, const string& ns, const string& key, const string& value) override
                {
                    Core::SafeSyncType<Core::CriticalSection> lock(_parent._clientLock);

                    std::list<Exchange::IStore::INotification*>::iterator
                        index(_parent._clients.begin());

                    while (index != _parent._clients.end()) {
                        (*index)->ValueChanged(ns, key, value);
                        index++;
                    }
                }

                BEGIN_INTERFACE_MAP(Store2Notification)
                INTERFACE_ENTRY(Exchange::IStore2::INotification)
                END_INTERFACE_MAP

            private:
                Store& _parent;
            };

        public:
            Store(Exchange::IStore2* store2)
                : _store2(store2)
                , _store2Sink(this)
            {
                ASSERT(_store2 != nullptr);
                _store2->AddRef();
                _store2->Register(&_store2Sink);
            }
            ~Store() override
            {
                _store2->Unregister(&_store2Sink);
                _store2->Release();
            }

        public:
            // IStore methods

            uint32_t Register(Exchange::IStore::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }
            uint32_t Unregister(Exchange::IStore::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<Exchange::IStore::INotification*>::iterator
                    index(std::find(_clients.begin(), _clients.end(), notification));

                ASSERT(index != _clients.end());

                if (index != _clients.end()) {
                    notification->Release();
                    _clients.erase(index);
                }

                return Core::ERROR_NONE;
            }
            uint32_t SetValue(const string& ns, const string& key, const string& value) override
            {
                return _store2->SetValue(Exchange::IStore2::ScopeType::DEVICE, ns, key, value, 0);
            }
            uint32_t GetValue(const string& ns, const string& key, string& value) override
            {
                uint32_t ttl;
                return _store2->GetValue(Exchange::IStore2::ScopeType::DEVICE, ns, key, value, ttl);
            }
            uint32_t DeleteKey(const string& ns, const string& key) override
            {
                return _store2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, ns, key);
            }
            uint32_t DeleteNamespace(const string& ns) override
            {
                return _store2->DeleteNamespace(Exchange::IStore2::ScopeType::DEVICE, ns);
            }

            BEGIN_INTERFACE_MAP(Store)
            INTERFACE_ENTRY(Exchange::IStore)
            END_INTERFACE_MAP

        private:
            Exchange::IStore2* _store2;
            std::list<Exchange::IStore::INotification*> _clients;
            Core::CriticalSection _clientLock;
            Core::Sink<Store2Notification> _store2Sink;
        };

        class Store2 : public Exchange::IStore2 {
        private:
            Store2(const Store2&) = delete;
            Store2& operator=(const Store2&) = delete;

        public:
            typedef std::map<ScopeType, Exchange::IStore2*> ScopeMapType;

            Store2(const ScopeMapType& map)
                : _scopeMap(map)
            {
                for (auto const& x : _scopeMap) {
                    x.second->AddRef();
                }
            }
            ~Store2() override
            {
                for (auto const& x : _scopeMap) {
                    x.second->Release();
                }
            }

        public:
            // IStore2 methods

            uint32_t Register(Exchange::IStore2::INotification* notification) override
            {
                for (auto const& x : _scopeMap) {
                    x.second->Register(notification);
                }
                return Core::ERROR_NONE;
            }
            uint32_t Unregister(Exchange::IStore2::INotification* notification) override
            {
                for (auto const& x : _scopeMap) {
                    x.second->Unregister(notification);
                }
                return Core::ERROR_NONE;
            }
            uint32_t SetValue(const ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                return _scopeMap.at(scope)->SetValue(scope, ns, key, value, ttl);
            }
            uint32_t GetValue(const ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                return _scopeMap.at(scope)->GetValue(scope, ns, key, value, ttl);
            }
            uint32_t DeleteKey(const ScopeType scope, const string& ns, const string& key) override
            {
                return _scopeMap.at(scope)->DeleteKey(scope, ns, key);
            }
            uint32_t DeleteNamespace(const ScopeType scope, const string& ns) override
            {
                return _scopeMap.at(scope)->DeleteNamespace(scope, ns);
            }

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(Exchange::IStore2)
            END_INTERFACE_MAP

        private:
            ScopeMapType _scopeMap;
        };

    private:
        PersistentStore(const PersistentStore&) = delete;
        PersistentStore& operator=(const PersistentStore&) = delete;

    public:
        PersistentStore()
            : PluginHost::JSONRPC()
            , _store(nullptr)
            , _store2(nullptr)
            , _storeCache(nullptr)
            , _storeInspector(nullptr)
            , _storeLimit(nullptr)
            , _store2Sink(this)
        {
            RegisterAll();
        }
        ~PersistentStore() override
        {
            UnregisterAll();
        }

        // Build QueryInterface implementation, specifying all possible interfaces to be returned.
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
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    private:
        // JSON RPC

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
        Exchange::IStore* _store; // Deprecated
        Exchange::IStore2* _store2;
        Exchange::IStoreCache* _storeCache;
        Exchange::IStoreInspector* _storeInspector;
        Exchange::IStoreLimit* _storeLimit;
        Core::Sink<Store2Notification> _store2Sink;
    };

} // namespace Plugin
} // namespace WPEFramework
