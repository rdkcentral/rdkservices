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

        // Deprecated
        class Store : public Exchange::IStore,
                      public Exchange::IStore2::INotification {
        private:
            Store(const Store&) = delete;
            Store& operator=(const Store&) = delete;

        public:
            Store(PersistentStore& parent)
                : _parent(parent)
            {
            }
            ~Store() override = default;

        public:
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
                if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->SetValue(Exchange::IStore2::ScopeType::DEVICE, ns, key, value, 0);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetValue(const string& ns, const string& key, string& value) override
            {
                if (_parent._deviceStore2 != nullptr) {
                    uint32_t ttl;
                    return _parent._deviceStore2->GetValue(Exchange::IStore2::ScopeType::DEVICE, ns, key, value, ttl);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t DeleteKey(const string& ns, const string& key) override
            {
                if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->DeleteKey(Exchange::IStore2::ScopeType::DEVICE, ns, key);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t DeleteNamespace(const string& ns) override
            {
                if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->DeleteNamespace(Exchange::IStore2::ScopeType::DEVICE, ns);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                ASSERT(scope == Exchange::IStore2::ScopeType::DEVICE);

                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                std::list<Exchange::IStore::INotification*>::iterator
                    index(_clients.begin());

                while (index != _clients.end()) {
                    (*index)->ValueChanged(ns, key, value);
                    index++;
                }
            }

            BEGIN_INTERFACE_MAP(Store)
            INTERFACE_ENTRY(Exchange::IStore)
            INTERFACE_ENTRY(Exchange::IStore2::INotification)
            END_INTERFACE_MAP

        private:
            PersistentStore& _parent;
            std::list<Exchange::IStore::INotification*> _clients;
            Core::CriticalSection _clientLock;
        };

        class Store2 : public Exchange::IStore2,
                       public Exchange::IStoreCache,
                       public Exchange::IStoreInspector,
                       public Exchange::IStoreLimit {
        private:
            Store2(const Store2&) = delete;
            Store2& operator=(const Store2&) = delete;

        public:
            Store2(PersistentStore& parent)
                : _parent(parent)
            {
            }
            ~Store2() override = default;

        public:
            uint32_t Register(INotification* notification) override
            {
                if (_parent._deviceStore2 != nullptr) {
                    _parent._deviceStore2->Register(notification);
                }
                if (_parent._accountStore2 != nullptr) {
                    _parent._accountStore2->Register(notification);
                }
                return Core::ERROR_NONE;
            }
            uint32_t Unregister(INotification* notification) override
            {
                if (_parent._deviceStore2 != nullptr) {
                    _parent._deviceStore2->Unregister(notification);
                }
                if (_parent._accountStore2 != nullptr) {
                    _parent._accountStore2->Unregister(notification);
                }
                return Core::ERROR_NONE;
            }
            uint32_t SetValue(const IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
            {
                if (scope == IStore2::ScopeType::ACCOUNT) {
                    if (_parent._accountStore2 != nullptr) {
                        return _parent._accountStore2->SetValue(scope, ns, key, value, ttl);
                    }
                } else if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->SetValue(scope, ns, key, value, ttl);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetValue(const IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
            {
                if (scope == IStore2::ScopeType::ACCOUNT) {
                    if (_parent._accountStore2 != nullptr) {
                        return _parent._accountStore2->GetValue(scope, ns, key, value, ttl);
                    }
                } else if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->GetValue(scope, ns, key, value, ttl);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t DeleteKey(const IStore2::ScopeType scope, const string& ns, const string& key) override
            {
                if (scope == IStore2::ScopeType::ACCOUNT) {
                    if (_parent._accountStore2 != nullptr) {
                        return _parent._accountStore2->DeleteKey(scope, ns, key);
                    }
                } else if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->DeleteKey(scope, ns, key);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t DeleteNamespace(const IStore2::ScopeType scope, const string& ns) override
            {
                if (scope == IStore2::ScopeType::ACCOUNT) {
                    if (_parent._accountStore2 != nullptr) {
                        return _parent._accountStore2->DeleteNamespace(scope, ns);
                    }
                } else if (_parent._deviceStore2 != nullptr) {
                    return _parent._deviceStore2->DeleteNamespace(scope, ns);
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t FlushCache() override
            {
                if (_parent._deviceStoreCache != nullptr) {
                    return _parent._deviceStoreCache->FlushCache();
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetKeys(const IStoreInspector::ScopeType scope, const string& ns, RPC::IStringIterator*& keys) override
            {
                if (scope == IStoreInspector::ScopeType::DEVICE) {
                    if (_parent._deviceStoreInspector != nullptr) {
                        return _parent._deviceStoreInspector->GetKeys(scope, ns, keys);
                    }
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetNamespaces(const IStoreInspector::ScopeType scope, RPC::IStringIterator*& namespaces) override
            {
                if (scope == IStoreInspector::ScopeType::DEVICE) {
                    if (_parent._deviceStoreInspector != nullptr) {
                        return _parent._deviceStoreInspector->GetNamespaces(scope, namespaces);
                    }
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetStorageSizes(const IStoreInspector::ScopeType scope, INamespaceSizeIterator*& storageList) override
            {
                if (scope == IStoreInspector::ScopeType::DEVICE) {
                    if (_parent._deviceStoreInspector != nullptr) {
                        return _parent._deviceStoreInspector->GetStorageSizes(scope, storageList);
                    }
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t SetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, const uint32_t size) override
            {
                if (scope == IStoreLimit::ScopeType::DEVICE) {
                    if (_parent._deviceStoreLimit != nullptr) {
                        return _parent._deviceStoreLimit->SetNamespaceStorageLimit(scope, ns, size);
                    }
                }
                return Core::ERROR_NOT_SUPPORTED;
            }
            uint32_t GetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, uint32_t& size) override
            {
                if (scope == IStoreLimit::ScopeType::DEVICE) {
                    if (_parent._deviceStoreLimit != nullptr) {
                        return _parent._deviceStoreLimit->GetNamespaceStorageLimit(scope, ns, size);
                    }
                }
                return Core::ERROR_NOT_SUPPORTED;
            }

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(IStore2)
            INTERFACE_ENTRY(IStoreCache)
            INTERFACE_ENTRY(IStoreInspector)
            INTERFACE_ENTRY(IStoreLimit)
            END_INTERFACE_MAP

        private:
            PersistentStore& _parent;
        };

    private:
        PersistentStore(const PersistentStore&) = delete;
        PersistentStore& operator=(const PersistentStore&) = delete;

    public:
        PersistentStore()
            : PluginHost::JSONRPC()
            , _deviceStore2(nullptr)
            , _deviceStoreCache(nullptr)
            , _deviceStoreInspector(nullptr)
            , _deviceStoreLimit(nullptr)
            , _accountStore2(nullptr)
            , _store(Core::Service<Store>::Create<Store>(*this))
            , _store2(Core::Service<Store2>::Create<Store2>(*this))
            , _store2Sink(*this)
        {
            RegisterAll();
        }
        ~PersistentStore() override
        {
            UnregisterAll();

            if (_store != nullptr) {
                _store->Release();
                _store = nullptr;
            }
            if (_store2 != nullptr) {
                _store2->Release();
                _store2 = nullptr;
            }
        }

        BEGIN_INTERFACE_MAP(PersistentStore)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        INTERFACE_AGGREGATE(Exchange::IStore, _store)
        INTERFACE_AGGREGATE(Exchange::IStore2, _store2)
        INTERFACE_AGGREGATE(Exchange::IStoreCache, _store2)
        INTERFACE_AGGREGATE(Exchange::IStoreInspector, _store2)
        INTERFACE_AGGREGATE(Exchange::IStoreLimit, _store2)
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
        Exchange::IStore2* _deviceStore2;
        Exchange::IStoreCache* _deviceStoreCache;
        Exchange::IStoreInspector* _deviceStoreInspector;
        Exchange::IStoreLimit* _deviceStoreLimit;
        Exchange::IStore2* _accountStore2;
        Store* _store; // Deprecated
        Store2* _store2;
        Core::Sink<Store2Notification> _store2Sink;
    };

} // namespace Plugin
} // namespace WPEFramework
