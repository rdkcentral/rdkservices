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
#include <interfaces/IStoreInspector.h>
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
                , Path()
                , Key()
                , MaxSize(0)
                , MaxValue(0)
            {
                Add(_T("path"), &Path);
                Add(_T("legacypath"), &LegacyPath);
                Add(_T("key"), &Key);
                Add(_T("maxsize"), &MaxSize);
                Add(_T("maxvalue"), &MaxValue);
            }

        public:
            Core::JSON::String Path;
            Core::JSON::String LegacyPath;
            Core::JSON::String Key;
            Core::JSON::DecUInt64 MaxSize;
            Core::JSON::DecUInt64 MaxValue;
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
            ~Store2Notification() = default;

        public:
            // IStore2::INotification methods

            virtual void ValueChanged(const string& ns, const string& key, const string& value, const Exchange::IStore2::ScopeType scope) override
            {
                JsonData::PersistentStore::SetValueParamsData params;
                params.Namespace = ns;
                params.Key = key;
                params.Value = value;
                params.Scope = JsonData::PersistentStore::ScopeType(scope);

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
                ~Store2Notification() = default;

            public:
                // IStore2::INotification methods

                virtual void ValueChanged(const string& ns, const string& key, const string& value, const Exchange::IStore2::ScopeType) override
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
            virtual ~Store()
            {
                _store2->Unregister(&_store2Sink);
                _store2->Release();
            }

        public:
            // IStore methods

            virtual uint32_t Register(Exchange::IStore::INotification* notification) override
            {
                Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

                ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

                notification->AddRef();
                _clients.push_back(notification);

                return Core::ERROR_NONE;
            }
            virtual uint32_t Unregister(Exchange::IStore::INotification* notification) override
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
            virtual uint32_t SetValue(const string& ns, const string& key, const string& value) override
            {
                return _store2->SetValue(ns, key, value, Exchange::IStore2::ScopeType::DEVICE, 0);
            }
            virtual uint32_t GetValue(const string& ns, const string& key, string& value) override
            {
                uint32_t ttl;
                return _store2->GetValue(ns, key, Exchange::IStore2::ScopeType::DEVICE, value, ttl);
            }
            virtual uint32_t DeleteKey(const string& ns, const string& key) override
            {
                return _store2->DeleteKey(ns, key, Exchange::IStore2::ScopeType::DEVICE);
            }
            virtual uint32_t DeleteNamespace(const string& ns) override
            {
                return _store2->DeleteNamespace(ns, Exchange::IStore2::ScopeType::DEVICE);
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

            typedef std::map<Exchange::IStore2::ScopeType, Exchange::IStore2*> ScopeMapType;

        public:
            Store2()
            {
            }
            virtual ~Store2()
            {
                for (auto const& x : ScopeMap) {
                    x.second->Release();
                }
            }

        public:
            // IStore2 methods

            virtual uint32_t Register(Exchange::IStore2::INotification* notification) override
            {
                for (auto const& x : ScopeMap) {
                    x.second->Register(notification);
                }
                return Core::ERROR_NONE;
            }
            virtual uint32_t Unregister(Exchange::IStore2::INotification* notification) override
            {
                for (auto const& x : ScopeMap) {
                    x.second->Unregister(notification);
                }
                return Core::ERROR_NONE;
            }
            virtual uint32_t SetValue(const string& ns, const string& key, const string& value, const ScopeType scope, const uint32_t ttl) override
            {
                return ScopeMap.at(scope)->SetValue(ns, key, value, scope, ttl);
            }
            virtual uint32_t GetValue(const string& ns, const string& key, const ScopeType scope, string& value, uint32_t& ttl) override
            {
                return ScopeMap.at(scope)->GetValue(ns, key, scope, value, ttl);
            }
            virtual uint32_t DeleteKey(const string& ns, const string& key, const ScopeType scope) override
            {
                return ScopeMap.at(scope)->DeleteKey(ns, key, scope);
            }
            virtual uint32_t DeleteNamespace(const string& ns, const ScopeType scope) override
            {
                return ScopeMap.at(scope)->DeleteNamespace(ns, scope);
            }

            BEGIN_INTERFACE_MAP(Store2)
            INTERFACE_ENTRY(Exchange::IStore2)
            END_INTERFACE_MAP

        public:
            ScopeMapType ScopeMap;
        };

        class StoreInspector : public Exchange::IStoreInspector {
        private:
            StoreInspector(const StoreInspector&) = delete;
            StoreInspector& operator=(const StoreInspector&) = delete;

            typedef std::map<ScopeType, Exchange::IStoreInspector*> ScopeMapType;

        public:
            StoreInspector()
            {
            }
            virtual ~StoreInspector()
            {
                for (auto const& x : ScopeMap) {
                    x.second->Release();
                }
            }

        public:
            // IStoreInspector methods

            virtual uint32_t SetNamespaceLimit(const string& ns, const uint32_t size, const ScopeType scope) override
            {
                return ScopeMap.at(scope)->SetNamespaceLimit(ns, size, scope);
            }
            virtual uint32_t GetNamespaceLimit(const string& ns, const ScopeType scope, uint32_t& size) override
            {
                return ScopeMap.at(scope)->GetNamespaceLimit(ns, scope, size);
            }
            virtual uint32_t GetKeys(const string& ns, const ScopeType scope, RPC::IStringIterator*& keys) override
            {
                return ScopeMap.at(scope)->GetKeys(ns, scope, keys);
            }
            virtual uint32_t GetNamespaces(const ScopeType scope, RPC::IStringIterator*& namespaces) override
            {
                return ScopeMap.at(scope)->GetNamespaces(scope, namespaces);
            }
            virtual uint32_t GetNamespaceSizes(const ScopeType scope, INamespaceSizeIterator*& namespaceSizes) override
            {
                return ScopeMap.at(scope)->GetNamespaceSizes(scope, namespaceSizes);
            }

            BEGIN_INTERFACE_MAP(StoreInspector)
            INTERFACE_ENTRY(Exchange::IStoreInspector)
            END_INTERFACE_MAP

        public:
            ScopeMapType ScopeMap;
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
            , _store2Sink(this)
        {
            RegisterAll();
        }
        virtual ~PersistentStore()
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
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

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
        Core::Sink<Store2Notification> _store2Sink;
    };

} // namespace Plugin
} // namespace WPEFramework
