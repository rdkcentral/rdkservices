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

namespace Thunder {
namespace Plugin {

    class PersistentStoreImplementation : public Exchange::IStore,
                                          public Exchange::IStore2,
                                          public Exchange::IStoreCache,
                                          public Exchange::IStoreInspector,
                                          public Exchange::IStoreLimit {
    private:
        class Store2Notification : public IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(PersistentStoreImplementation& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                ASSERT(scope == IStore2::ScopeType::DEVICE);

                Core::SafeSyncType<Core::CriticalSection> lock(_parent._clientLock);

                std::list<IStore::INotification*>::iterator
                    index(_parent._clients.begin());

                while (index != _parent._clients.end()) {
                    (*index)->ValueChanged(ns, key, value);
                    index++;
                }
            }

            BEGIN_INTERFACE_MAP(Store)
            INTERFACE_ENTRY(IStore2::INotification)
            END_INTERFACE_MAP

        private:
            PersistentStoreImplementation& _parent;
        };

    private:
        PersistentStoreImplementation(const PersistentStoreImplementation&) = delete;
        PersistentStoreImplementation& operator=(const PersistentStoreImplementation&) = delete;

    public:
        PersistentStoreImplementation();
        ~PersistentStoreImplementation() override;

        BEGIN_INTERFACE_MAP(PersistentStoreImplementation)
        INTERFACE_ENTRY(IStore)
        INTERFACE_ENTRY(IStore2)
        INTERFACE_ENTRY(IStoreCache)
        INTERFACE_ENTRY(IStoreInspector)
        INTERFACE_ENTRY(IStoreLimit)
        END_INTERFACE_MAP

    private:
        uint32_t Register(IStore::INotification* notification) override
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

            ASSERT(std::find(_clients.begin(), _clients.end(), notification) == _clients.end());

            notification->AddRef();
            _clients.push_back(notification);

            return Core::ERROR_NONE;
        }
        uint32_t Unregister(IStore::INotification* notification) override
        {
            Core::SafeSyncType<Core::CriticalSection> lock(_clientLock);

            std::list<IStore::INotification*>::iterator
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
            return SetValue(IStore2::ScopeType::DEVICE, ns, key, value, 0);
        }
        uint32_t GetValue(const string& ns, const string& key, string& value) override
        {
            uint32_t ttl;
            return GetValue(IStore2::ScopeType::DEVICE, ns, key, value, ttl);
        }
        uint32_t DeleteKey(const string& ns, const string& key) override
        {
            return DeleteKey(IStore2::ScopeType::DEVICE, ns, key);
        }
        uint32_t DeleteNamespace(const string& ns) override
        {
            return DeleteNamespace(IStore2::ScopeType::DEVICE, ns);
        }
        uint32_t Register(IStore2::INotification* notification) override
        {
            if (_deviceStore2 != nullptr) {
                _deviceStore2->Register(notification);
            }
            if (_accountStore2 != nullptr) {
                _accountStore2->Register(notification);
            }
            return Core::ERROR_NONE;
        }
        uint32_t Unregister(IStore2::INotification* notification) override
        {
            if (_deviceStore2 != nullptr) {
                _deviceStore2->Unregister(notification);
            }
            if (_accountStore2 != nullptr) {
                _accountStore2->Unregister(notification);
            }
            return Core::ERROR_NONE;
        }
        uint32_t SetValue(const IStore2::ScopeType scope, const string& ns, const string& key, const string& value, const uint32_t ttl) override
        {
            if (scope == IStore2::ScopeType::ACCOUNT) {
                if (_accountStore2 != nullptr) {
                    return _accountStore2->SetValue(scope, ns, key, value, ttl);
                }
            } else if (_deviceStore2 != nullptr) {
                return _deviceStore2->SetValue(scope, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetValue(const IStore2::ScopeType scope, const string& ns, const string& key, string& value, uint32_t& ttl) override
        {
            if (scope == IStore2::ScopeType::ACCOUNT) {
                if (_accountStore2 != nullptr) {
                    return _accountStore2->GetValue(scope, ns, key, value, ttl);
                }
            } else if (_deviceStore2 != nullptr) {
                return _deviceStore2->GetValue(scope, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteKey(const IStore2::ScopeType scope, const string& ns, const string& key) override
        {
            if (scope == IStore2::ScopeType::ACCOUNT) {
                if (_accountStore2 != nullptr) {
                    return _accountStore2->DeleteKey(scope, ns, key);
                }
            } else if (_deviceStore2 != nullptr) {
                return _deviceStore2->DeleteKey(scope, ns, key);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteNamespace(const IStore2::ScopeType scope, const string& ns) override
        {
            if (scope == IStore2::ScopeType::ACCOUNT) {
                if (_accountStore2 != nullptr) {
                    return _accountStore2->DeleteNamespace(scope, ns);
                }
            } else if (_deviceStore2 != nullptr) {
                return _deviceStore2->DeleteNamespace(scope, ns);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t FlushCache() override
        {
            if (_deviceStoreCache != nullptr) {
                return _deviceStoreCache->FlushCache();
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetKeys(const IStoreInspector::ScopeType scope, const string& ns, RPC::IStringIterator*& keys) override
        {
            if (scope == IStoreInspector::ScopeType::DEVICE) {
                if (_deviceStoreInspector != nullptr) {
                    return _deviceStoreInspector->GetKeys(scope, ns, keys);
                }
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetNamespaces(const IStoreInspector::ScopeType scope, RPC::IStringIterator*& namespaces) override
        {
            if (scope == IStoreInspector::ScopeType::DEVICE) {
                if (_deviceStoreInspector != nullptr) {
                    return _deviceStoreInspector->GetNamespaces(scope, namespaces);
                }
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetStorageSizes(const IStoreInspector::ScopeType scope, INamespaceSizeIterator*& storageList) override
        {
            if (scope == IStoreInspector::ScopeType::DEVICE) {
                if (_deviceStoreInspector != nullptr) {
                    return _deviceStoreInspector->GetStorageSizes(scope, storageList);
                }
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t SetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, const uint32_t size) override
        {
            if (scope == IStoreLimit::ScopeType::DEVICE) {
                if (_deviceStoreLimit != nullptr) {
                    return _deviceStoreLimit->SetNamespaceStorageLimit(scope, ns, size);
                }
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetNamespaceStorageLimit(const IStoreLimit::ScopeType scope, const string& ns, uint32_t& size) override
        {
            if (scope == IStoreLimit::ScopeType::DEVICE) {
                if (_deviceStoreLimit != nullptr) {
                    return _deviceStoreLimit->GetNamespaceStorageLimit(scope, ns, size);
                }
            }
            return Core::ERROR_NOT_SUPPORTED;
        }

    private:
        IStore2* _deviceStore2;
        IStoreCache* _deviceStoreCache;
        IStoreInspector* _deviceStoreInspector;
        IStoreLimit* _deviceStoreLimit;
        IStore2* _accountStore2;
        Core::Sink<Store2Notification> _store2Sink;
        std::list<IStore::INotification*> _clients;
        Core::CriticalSection _clientLock;
    };

} // namespace Plugin
} // namespace Thunder
