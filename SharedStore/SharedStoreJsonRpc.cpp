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

#include "SharedStore.h"

namespace WPEFramework {
namespace Plugin {

    using namespace JsonData::PersistentStore;

    void SharedStore::RegisterAll()
    {
        Register<SetValueParamsData, DeleteKeyResultInfo>(_T("setValue"), &SharedStore::endpoint_setValue, this);
        Register<DeleteKeyParamsInfo, GetValueResultData>(_T("getValue"), &SharedStore::endpoint_getValue, this);
        Register<DeleteKeyParamsInfo, DeleteKeyResultInfo>(_T("deleteKey"), &SharedStore::endpoint_deleteKey, this);
        Register<DeleteNamespaceParamsInfo, DeleteKeyResultInfo>(_T("deleteNamespace"), &SharedStore::endpoint_deleteNamespace, this);
        Register<DeleteNamespaceParamsInfo, GetKeysResultData>(_T("getKeys"), &SharedStore::endpoint_getKeys, this);
        Register<GetNamespacesParamsInfo, GetNamespacesResultData>(_T("getNamespaces"), &SharedStore::endpoint_getNamespaces, this);
        Register<GetNamespacesParamsInfo, JsonObject>(_T("getStorageSize"), &SharedStore::endpoint_getStorageSize, this); // Deprecated
        Register<GetNamespacesParamsInfo, GetStorageSizesResultData>(_T("getStorageSizes"), &SharedStore::endpoint_getStorageSizes, this);
        Register<void, DeleteKeyResultInfo>(_T("flushCache"), &SharedStore::endpoint_flushCache, this);
        Register<DeleteNamespaceParamsInfo, GetNamespaceStorageLimitResultData>(_T("getNamespaceStorageLimit"), &SharedStore::endpoint_getNamespaceStorageLimit, this);
        Register<SetNamespaceStorageLimitParamsData, void>(_T("setNamespaceStorageLimit"), &SharedStore::endpoint_setNamespaceStorageLimit, this);
    }

    void SharedStore::UnregisterAll()
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

    uint32_t SharedStore::endpoint_setValue(const SetValueParamsData& params, DeleteKeyResultInfo& response)
    {
        LOGINFO("Scope: %d, ns: %s, key: %s, Val: %s", static_cast<int>(params.Scope.Value()), params.Namespace.Value().c_str(), params.Key.Value().c_str(), params.Value.Value().c_str());
        uint32_t status = Core::ERROR_NOT_SUPPORTED;

        Exchange::IStore2* _storeObject = getRemoteStoreObject(params.Scope.Value());
        ASSERT (nullptr != _storeObject);
        _adminLock.Lock();
        if (nullptr != _storeObject)
        {
            LOGINFO("inside _storeObject");
            status = _storeObject->SetValue(
                        Exchange::IStore2::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        params.Key.Value(),
                        params.Value.Value(),
                        params.Ttl.Value() );
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_getValue(const DeleteKeyParamsInfo& params, GetValueResultData& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        string value;
        uint32_t ttl = 0;

        Exchange::IStore2* _storeObject = getRemoteStoreObject(params.Scope.Value());
        ASSERT (nullptr != _storeObject);
        _adminLock.Lock();
        if (nullptr != _storeObject)
        {
            LOGINFO("inside _storeObject");
            status = _storeObject->GetValue(
                        Exchange::IStore2::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        params.Key.Value(),
                        value,
                        ttl );
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.Value = value;
                if (ttl > 0) {
                    response.Ttl = ttl;
                }
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_deleteKey(const DeleteKeyParamsInfo& params, DeleteKeyResultInfo& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        Exchange::IStore2* _storeObject = getRemoteStoreObject(params.Scope.Value());
        ASSERT (nullptr != _storeObject);
        _adminLock.Lock();
        if (nullptr != _storeObject)
        {
            LOGINFO("inside _storeObject");
            status = _storeObject->DeleteKey(
                        Exchange::IStore2::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        params.Key.Value() );
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_deleteNamespace(const DeleteNamespaceParamsInfo& params, DeleteKeyResultInfo& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        Exchange::IStore2* _storeObject = getRemoteStoreObject(params.Scope.Value());
        ASSERT (nullptr != _storeObject);
        _adminLock.Lock();
        if (nullptr != _storeObject)
        {
            LOGINFO("inside _storeObject");
            status = _storeObject->DeleteNamespace(
                        Exchange::IStore2::ScopeType(params.Scope.Value()),
                        params.Namespace.Value() );
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_getKeys(const DeleteNamespaceParamsInfo& params, GetKeysResultData& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psInspector);
        _adminLock.Lock();
        if (nullptr != _psInspector)
        {
            LOGINFO("inside _psInspector");
            RPC::IStringIterator* it;
            status = _psInspector->GetKeys(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        it);
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                string element;
                while (it->Next(element) == true) {
                    response.Keys.Add() = element;
                }
                it->Release();
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_getNamespaces(const GetNamespacesParamsInfo& params, GetNamespacesResultData& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psInspector);
        _adminLock.Lock();
        if (nullptr != _psInspector)
        {
            LOGINFO("inside _psInspector");
            RPC::IStringIterator* it;
            status = _psInspector->GetNamespaces(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        it);
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                string element;
                while (it->Next(element) == true) {
                    response.Namespaces.Add() = element;
                }
                it->Release();
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    // Deprecated
    uint32_t SharedStore::endpoint_getStorageSize(const GetNamespacesParamsInfo& params, JsonObject& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psInspector);
        _adminLock.Lock();
        if (nullptr != _psInspector)
        {
            LOGINFO("inside _psInspector");
            Exchange::IStoreInspector::INamespaceSizeIterator* it;
            status = _psInspector->GetStorageSizes(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        it);
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                JsonObject jsonObject;
                Exchange::IStoreInspector::NamespaceSize element;
                while (it->Next(element) == true) {
                    jsonObject[element.ns.c_str()] = element.size;
                }
                it->Release();
                response["namespaceSizes"] = jsonObject;
                response["success"] = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_getStorageSizes(const GetNamespacesParamsInfo& params, GetStorageSizesResultData& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psInspector);
        _adminLock.Lock();
        if (nullptr != _psInspector)
        {
            LOGINFO("inside _psInspector");
            Exchange::IStoreInspector::INamespaceSizeIterator* it;
            status = _psInspector->GetStorageSizes(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        it);
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                JsonObject jsonObject;
                Exchange::IStoreInspector::NamespaceSize element;
                while (it->Next(element) == true) {
                    auto& item = response.StorageList.Add();
                    item.Namespace = element.ns;
                    item.Size = element.size;
                }
                it->Release();
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_flushCache(DeleteKeyResultInfo& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        ASSERT (nullptr != _psCache);
        _adminLock.Lock();
        if (nullptr != _psCache)
        {
            LOGINFO("inside _psCache");
            status = _psCache->FlushCache();
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.Success = true;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_getNamespaceStorageLimit(const DeleteNamespaceParamsInfo& params, GetNamespaceStorageLimitResultData& response)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psLimit);
        _adminLock.Lock();
        if (nullptr != _psLimit)
        {
            LOGINFO("inside _psLimit");
            uint32_t size;
            status = _psLimit->GetNamespaceStorageLimit(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        size);
            if (status == Core::ERROR_NONE) {
                LOGINFO("status success");
                response.StorageLimit = size;
            }
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

    uint32_t SharedStore::endpoint_setNamespaceStorageLimit(const SetNamespaceStorageLimitParamsData& params)
    {
        uint32_t status = Core::ERROR_NOT_SUPPORTED;
        if(params.Scope.Value() != ScopeType::DEVICE)
        {
            return status;
        }
        ASSERT (nullptr != _psLimit);
        _adminLock.Lock();
        if (nullptr != _psLimit)
        {
            LOGINFO("inside _psLimit");
            status = _psLimit->SetNamespaceStorageLimit(
                        Exchange::IStoreInspector::ScopeType(params.Scope.Value()),
                        params.Namespace.Value(),
                        params.StorageLimit.Value());
        }
        _adminLock.Unlock();
        LOGINFO("status: %d", status);
        return status;
    }

} // namespace Plugin
} // namespace WPEFramework
