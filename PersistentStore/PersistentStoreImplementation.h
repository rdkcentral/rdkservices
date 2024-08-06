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

namespace WPEFramework {
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
            return Core::ERROR_NONE;
        }
        uint32_t Unregister(IStore2::INotification* notification) override
        {
            if (_deviceStore2 != nullptr) {
                _deviceStore2->Unregister(notification);
            }
            return Core::ERROR_NONE;
        }
        uint32_t SetValue(const IStore2::ScopeType, const string& ns, const string& key, const string& value, const uint32_t ttl) override
        {
            if (_deviceStore2 != nullptr) {
                return _deviceStore2->SetValue(IStore2::ScopeType::DEVICE, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetValue(const IStore2::ScopeType, const string& ns, const string& key, string& value, uint32_t& ttl) override
        {
            if (_deviceStore2 != nullptr) {
                return _deviceStore2->GetValue(IStore2::ScopeType::DEVICE, ns, key, value, ttl);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteKey(const IStore2::ScopeType, const string& ns, const string& key) override
        {
            if (_deviceStore2 != nullptr) {
                return _deviceStore2->DeleteKey(IStore2::ScopeType::DEVICE, ns, key);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t DeleteNamespace(const IStore2::ScopeType, const string& ns) override
        {
            if (_deviceStore2 != nullptr) {
                return _deviceStore2->DeleteNamespace(IStore2::ScopeType::DEVICE, ns);
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
        uint32_t GetKeys(const IStoreInspector::ScopeType, const string& ns, RPC::IStringIterator*& keys) override
        {
            if (_deviceStoreInspector != nullptr) {
                return _deviceStoreInspector->GetKeys(IStore2::ScopeType::DEVICE, ns, keys);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetNamespaces(const IStoreInspector::ScopeType, RPC::IStringIterator*& namespaces) override
        {
            if (_deviceStoreInspector != nullptr) {
                return _deviceStoreInspector->GetNamespaces(IStore2::ScopeType::DEVICE, namespaces);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetStorageSizes(const IStoreInspector::ScopeType, INamespaceSizeIterator*& storageList) override
        {
            if (_deviceStoreInspector != nullptr) {
                return _deviceStoreInspector->GetStorageSizes(IStore2::ScopeType::DEVICE, storageList);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t SetNamespaceStorageLimit(const IStoreLimit::ScopeType, const string& ns, const uint32_t size) override
        {
            if (_deviceStoreLimit != nullptr) {
                return _deviceStoreLimit->SetNamespaceStorageLimit(IStore2::ScopeType::DEVICE, ns, size);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }
        uint32_t GetNamespaceStorageLimit(const IStoreLimit::ScopeType, const string& ns, uint32_t& size) override
        {
            if (_deviceStoreLimit != nullptr) {
                return _deviceStoreLimit->GetNamespaceStorageLimit(IStore2::ScopeType::DEVICE, ns, size);
            }
            return Core::ERROR_NOT_SUPPORTED;
        }

    private:
        IStore2* _deviceStore2;
        IStoreCache* _deviceStoreCache;
        IStoreInspector* _deviceStoreInspector;
        IStoreLimit* _deviceStoreLimit;
        Core::Sink<Store2Notification> _store2Sink;
        std::list<IStore::INotification*> _clients;
        Core::CriticalSection _clientLock;
    };

} // namespace Plugin
} // namespace WPEFramework
