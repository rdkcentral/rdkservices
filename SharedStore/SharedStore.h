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
#include "UtilsLogging.h"
#include <mutex>
#include <interfaces/IStore2.h>
#include <interfaces/IStoreCache.h>
#include <interfaces/json/JsonData_PersistentStore.h>

namespace WPEFramework {
namespace Plugin {

    class SharedStore : public PluginHost::IPlugin, public PluginHost::JSONRPC {
    private:
        class Store2Notification : public Exchange::IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(SharedStore& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                LOGINFO("Scope: %d, ns: %s, key: %s, Val: %s", static_cast<int>(scope), ns.c_str(), key.c_str(), value.c_str());
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
            SharedStore& _parent;
        };

        class RemoteConnectionNotification : public RPC::IRemoteConnection::INotification {
        private:
            RemoteConnectionNotification() = delete;
            RemoteConnectionNotification(const RemoteConnectionNotification&) = delete;
            RemoteConnectionNotification& operator=(const RemoteConnectionNotification&) = delete;

        public:
            explicit RemoteConnectionNotification(SharedStore& parent)
                : _parent(parent)
            {
            }
            ~RemoteConnectionNotification() override = default;

            BEGIN_INTERFACE_MAP(RemoteConnectionNotification)
            INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

            void Activated(RPC::IRemoteConnection*) override
            {
                LOGINFO("SharedStore Notification Activated");
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                LOGINFO("SharedStore Notification Deactivated");
            }

        private:
            SharedStore& _parent;
        };

    private:
        SharedStore(const SharedStore&) = delete;
        SharedStore& operator=(const SharedStore&) = delete;

    public:
        SharedStore();
        ~SharedStore() override;

        BEGIN_INTERFACE_MAP(SharedStore)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
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
        Exchange::IStore2* getRemoteStoreObject(JsonData::PersistentStore::ScopeType eScope);

    private:
        PluginHost::IShell* _service{};
        Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> _psEngine;
        Core::ProxyType<RPC::CommunicatorClient> _psCommunicatorClient;
        Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> _csEngine;
        Core::ProxyType<RPC::CommunicatorClient> _csCommunicatorClient;
        PluginHost::IShell *_psController;
        PluginHost::IShell *_csController;
        Exchange::IStore2* _psObject;
        Exchange::IStoreCache* _psCache;
        Exchange::IStoreInspector* _psInspector;
        Exchange::IStoreLimit* _psLimit;
        Exchange::IStore2* _csObject;
        Core::Sink<Store2Notification> _storeNotification;
        Core::Sink<RemoteConnectionNotification> _notification;
        mutable Core::CriticalSection _adminLock;
        std::mutex storeObjMutex;
    };

} // namespace Plugin
} // namespace WPEFramework
