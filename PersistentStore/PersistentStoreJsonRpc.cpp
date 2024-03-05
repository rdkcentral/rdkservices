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

#include "PersistentStore.h"

namespace WPEFramework {
namespace Plugin {

    using namespace JsonData::PersistentStore;

    void PersistentStore::RegisterAll()
    {
        Register<SetValueParamsData, DeleteKeyResultInfo>(_T("setValue"), &PersistentStore::endpoint_setValue, this);
        Register<DeleteKeyParamsInfo, GetValueResultData>(_T("getValue"), &PersistentStore::endpoint_getValue, this);
        Register<DeleteKeyParamsInfo, DeleteKeyResultInfo>(_T("deleteKey"), &PersistentStore::endpoint_deleteKey, this);
        Register<DeleteNamespaceParamsInfo, DeleteKeyResultInfo>(_T("deleteNamespace"), &PersistentStore::endpoint_deleteNamespace, this);
        Register<DeleteNamespaceParamsInfo, GetKeysResultData>(_T("getKeys"), &PersistentStore::endpoint_getKeys, this);
        Register<GetNamespacesParamsInfo, GetNamespacesResultData>(_T("getNamespaces"), &PersistentStore::endpoint_getNamespaces, this);
        Register<GetNamespacesParamsInfo, JsonObject>(_T("getStorageSize"), &PersistentStore::endpoint_getStorageSize, this); // Deprecated
        Register<GetNamespacesParamsInfo, GetStorageSizesResultData>(_T("getStorageSizes"), &PersistentStore::endpoint_getStorageSizes, this);
        Register<void, DeleteKeyResultInfo>(_T("flushCache"), &PersistentStore::endpoint_flushCache, this);
        Register<DeleteNamespaceParamsInfo, GetNamespaceStorageLimitResultData>(_T("getNamespaceStorageLimit"), &PersistentStore::endpoint_getNamespaceStorageLimit, this);
        Register<SetNamespaceStorageLimitParamsData, void>(_T("setNamespaceStorageLimit"), &PersistentStore::endpoint_setNamespaceStorageLimit, this);
    }

    void PersistentStore::UnregisterAll()
    {
        Unregister(_T("setValue"));
        Unregister(_T("getValue"));
        Unregister(_T("deleteKey"));
        Unregister(_T("deleteNamespace"));
        Unregister(_T("getKeys"));
        Unregister(_T("getNamespaces"));
        Unregister(_T("getStorageSize"));
        Unregister(_T("getStorageSizes"));
        Unregister(_T("flushCache"));
        Unregister(_T("getNamespaceStorageLimit"));
        Unregister(_T("setNamespaceStorageLimit"));
    }

    uint32_t PersistentStore::endpoint_setValue(const SetValueParamsData& params, DeleteKeyResultInfo& response)
    {
        auto result = _store2->SetValue(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            params.Key.Value(),
            params.Value.Value(),
            params.Ttl.Value());
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_getValue(const DeleteKeyParamsInfo& params, GetValueResultData& response)
    {
        string value;
        uint32_t ttl;
        auto result = _store2->GetValue(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            params.Key.Value(),
            value,
            ttl);
        if (result == Core::ERROR_NONE) {
            response.Value = value;
            if (ttl > 0) {
                response.Ttl = ttl;
            }
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_deleteKey(const DeleteKeyParamsInfo& params, DeleteKeyResultInfo& response)
    {
        auto result = _store2->DeleteKey(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            params.Key.Value());
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_deleteNamespace(const DeleteNamespaceParamsInfo& params, DeleteKeyResultInfo& response)
    {
        auto result = _store2->DeleteNamespace(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value());
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_getKeys(const DeleteNamespaceParamsInfo& params, GetKeysResultData& response)
    {
        RPC::IStringIterator* it;
        auto result = _storeInspector->GetKeys(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            it);
        if (result == Core::ERROR_NONE) {
            string element;
            while (it->Next(element) == true) {
                response.Keys.Add() = element;
            }
            it->Release();
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_getNamespaces(const GetNamespacesParamsInfo& params, GetNamespacesResultData& response)
    {
        RPC::IStringIterator* it;
        auto result = _storeInspector->GetNamespaces(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            it);
        if (result == Core::ERROR_NONE) {
            string element;
            while (it->Next(element) == true) {
                response.Namespaces.Add() = element;
            }
            it->Release();
            response.Success = true;
        }

        return result;
    }

    // Deprecated
    uint32_t PersistentStore::endpoint_getStorageSize(const GetNamespacesParamsInfo& params, JsonObject& response)
    {
        Exchange::IStoreInspector::INamespaceSizeIterator* it;
        auto result = _storeInspector->GetStorageSizes(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            it);
        if (result == Core::ERROR_NONE) {
            JsonObject jsonObject;
            Exchange::IStoreInspector::NamespaceSize element;
            while (it->Next(element) == true) {
                jsonObject[element.ns.c_str()] = element.size;
            }
            it->Release();
            response["namespaceSizes"] = jsonObject;
            response["success"] = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_getStorageSizes(const GetNamespacesParamsInfo& params, GetStorageSizesResultData& response)
    {
        Exchange::IStoreInspector::INamespaceSizeIterator* it;
        auto result = _storeInspector->GetStorageSizes(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            it);
        if (result == Core::ERROR_NONE) {
            Exchange::IStoreInspector::NamespaceSize element;
            while (it->Next(element) == true) {
                auto& item = response.StorageList.Add();
                item.Namespace = element.ns;
                item.Size = element.size;
            }
            it->Release();
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_flushCache(DeleteKeyResultInfo& response)
    {
        auto result = _storeCache->FlushCache();
        if (result == Core::ERROR_NONE) {
            response.Success = true;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_getNamespaceStorageLimit(const DeleteNamespaceParamsInfo& params, GetNamespaceStorageLimitResultData& response)
    {
        uint32_t size;
        auto result = _storeLimit->GetNamespaceStorageLimit(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            size);
        if (result == Core::ERROR_NONE) {
            response.StorageLimit = size;
        }

        return result;
    }

    uint32_t PersistentStore::endpoint_setNamespaceStorageLimit(const SetNamespaceStorageLimitParamsData& params)
    {
        return _storeLimit->SetNamespaceStorageLimit(
            Exchange::IStore2::ScopeType(params.Scope.Value()),
            params.Namespace.Value(),
            params.StorageLimit.Value());
    }

} // namespace Plugin
} // namespace WPEFramework
