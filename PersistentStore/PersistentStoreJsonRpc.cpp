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

#include "UtilsJsonRpc.h"

namespace WPEFramework {
namespace Plugin {

void PersistentStore::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T("setValue"), &PersistentStore::endpoint_setValue, this);
    Register<JsonObject, JsonObject>(_T("getValue"), &PersistentStore::endpoint_getValue, this);
    Register<JsonObject, JsonObject>(_T("deleteKey"), &PersistentStore::endpoint_deleteKey, this);
    Register<JsonObject, JsonObject>(_T("deleteNamespace"), &PersistentStore::endpoint_deleteNamespace, this);
    Register<JsonObject, JsonObject>(_T("getKeys"), &PersistentStore::endpoint_getKeys, this);
    Register<JsonObject, JsonObject>(_T("getNamespaces"), &PersistentStore::endpoint_getNamespaces, this);
    Register<JsonObject, JsonObject>(_T("getStorageSize"), &PersistentStore::endpoint_getStorageSize, this);
    Register<JsonObject, JsonObject>(_T("flushCache"), &PersistentStore::endpoint_flushCache, this);
    Property<JsonObject>(_T("version"), &PersistentStore::get_version, nullptr, this);
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
    Unregister(_T("flushCache"));
    Unregister(_T("version"));
}

uint32_t PersistentStore::endpoint_setValue(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("namespace") ||
        !parameters.HasLabel("key") ||
        !parameters.HasLabel("value")) {
        response["error"] = "params missing";
    }
    else {
        string ns = parameters["namespace"].String();
        string key = parameters["key"].String();
        string value = parameters["value"].String();

        if (ns.empty() || key.empty()) {
            response["error"] = "params empty";
        }
        else {
            auto status = _store->SetValue(ns, key, value);
            if (status == Core::ERROR_INVALID_INPUT_LENGTH) {
                response["error"] = "params too long";
            }
            success = (status == Core::ERROR_NONE);
        }
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_getValue(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("namespace") ||
        !parameters.HasLabel("key")) {
        response["error"] = "params missing";
    }
    else {
        string ns = parameters["namespace"].String();
        string key = parameters["key"].String();
        if (ns.empty() || key.empty()) {
            response["error"] = "params empty";
        }
        else {
            string value;
            success = (_store->GetValue(ns, key, value) == Core::ERROR_NONE);
            if (success)
                response["value"] = value;
        }
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_deleteKey(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("namespace") ||
        !parameters.HasLabel("key")) {
        response["error"] = "params missing";
    }
    else {
        string ns = parameters["namespace"].String();
        string key = parameters["key"].String();
        if (ns.empty() || key.empty()) {
            response["error"] = "params empty";
        }
        else {
            success = (_store->DeleteKey(ns, key) == Core::ERROR_NONE);
        }
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_deleteNamespace(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("namespace")) {
        response["error"] = "params missing";
    }
    else {
        string ns = parameters["namespace"].String();
        if (ns.empty()) {
            response["error"] = "params empty";
        }
        else {
            success = (_store->DeleteNamespace(ns) == Core::ERROR_NONE);
        }
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_getKeys(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("namespace")) {
        response["error"] = "params missing";
    }
    else {
        string ns = parameters["namespace"].String();
        if (ns.empty())
            response["error"] = "params empty";
        else {
            std::vector<string> keys;
            success = (_storeListing->GetKeys(ns, keys) == Core::ERROR_NONE);
            if (success) {
                JsonArray jsonKeys;
                for (auto it = keys.begin(); it != keys.end(); ++it)
                    jsonKeys.Add(*it);
                response["keys"] = jsonKeys;
            }
        }
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_getNamespaces(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    std::vector<string> namespaces;
    success = (_storeListing->GetNamespaces(namespaces) == Core::ERROR_NONE);
    if (success) {
        JsonArray jsonNamespaces;
        for (auto it = namespaces.begin(); it != namespaces.end(); ++it)
            jsonNamespaces.Add(*it);
        response["namespaces"] = jsonNamespaces;
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_getStorageSize(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    std::map<string, uint64_t> namespaceSizes;
    success = (_storeListing->GetStorageSize(namespaceSizes) == Core::ERROR_NONE);
    if (success) {
        JsonObject jsonNamespaceSizes;
        for (auto it = namespaceSizes.begin(); it != namespaceSizes.end(); ++it)
            jsonNamespaceSizes[it->first.c_str()] = it->second;
        response["namespaceSizes"] = jsonNamespaceSizes;
    }

    returnResponse(success);
}

uint32_t PersistentStore::endpoint_flushCache(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = (_storeCache->FlushCache() == Core::ERROR_NONE);

    returnResponse(success);
}

uint32_t PersistentStore::get_version(JsonObject& response) const
{
    response[_T("version")] = SOLUTIONS_GENERICS_SYSTEM_PREPROCESSOR_2(PLUGIN_VERSION);

    return (Core::ERROR_NONE);
}

void PersistentStore::event_onValueChanged(const string &ns, const string &key, const string &value)
{
    JsonObject params;
    params["namespace"] = ns;
    params["key"] = key;
    params["value"] = value;

    sendNotify(_T("onValueChanged"), params);
}

void PersistentStore::event_onStorageExceeded()
{
    sendNotify(_T("onStorageExceeded"), JsonObject());
}

} // namespace Plugin
} // namespace WPEFramework