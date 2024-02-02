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
#include <fstream>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 5

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::PersistentStore> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {});
}

namespace Plugin {

    SERVICE_REGISTRATION(PersistentStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    const string PersistentStore::Initialize(PluginHost::IShell* service)
    {
        string result;

        ASSERT(service != nullptr);

        auto configLine = service->ConfigLine();
        _config.FromString(configLine);

        {
            Core::File file(_config.Path.Value());
            Core::File oldFile(_config.LegacyPath.Value());
            if (file.Name() != oldFile.Name()) {
                if (oldFile.Exists()) {
                    if (!file.Exists()) {
                        std::ifstream in(oldFile.Name(), std::ios::in | std::ios::binary);
                        std::ofstream out(file.Name(), std::ios::out | std::ios::binary);
                        out << in.rdbuf();
                    }

                    oldFile.Destroy();
                }
            }
        }

        Core::SystemInfo::SetEnvironment(URI_ENV, _config.Uri.Value());
        Core::SystemInfo::SetEnvironment(PATH_ENV, _config.Path.Value());
        Core::SystemInfo::SetEnvironment(MAXSIZE_ENV, std::to_string(_config.MaxSize.Value()));
        Core::SystemInfo::SetEnvironment(MAXVALUE_ENV, std::to_string(_config.MaxValue.Value()));
        Core::SystemInfo::SetEnvironment(LIMIT_ENV, std::to_string(_config.Limit.Value()));

        uint32_t connectionId;

        Store2::ScopeMapType initList1;
        auto deviceStore2 = service->Root<Exchange::IStore2>(connectionId, 2000, _T("SqliteStore2"));
        if (deviceStore2 != nullptr) {
            initList1.emplace(Exchange::IStore2::ScopeType::DEVICE, deviceStore2);
        }
        auto accountStore2 = service->Root<Exchange::IStore2>(connectionId, 2000, _T("GrpcStore2"));
        if (accountStore2 != nullptr) {
            initList1.emplace(Exchange::IStore2::ScopeType::ACCOUNT, accountStore2);
        }
        _store2 = Core::Service<Store2>::Create<Exchange::IStore2>(initList1);
        if (deviceStore2 != nullptr) {
            deviceStore2->Release();
        }
        if (accountStore2 != nullptr) {
            accountStore2->Release();
        }
        _store2->Register(&_store2Sink);
        _store = Core::Service<Store>::Create<Exchange::IStore>(_store2);
        _storeCache = service->Root<Exchange::IStoreCache>(connectionId, 2000, _T("SqliteStoreCache"));
        _storeInspector = service->Root<Exchange::IStoreInspector>(connectionId, 2000, _T("SqliteStoreInspector"));
        _storeLimit = service->Root<Exchange::IStoreLimit>(connectionId, 2000, _T("SqliteStoreLimit"));

        return result;
    }

    void PersistentStore::Deinitialize(PluginHost::IShell* /* service */)
    {
        if (_store != nullptr) {
            _store->Release();
            _store = nullptr;
        }
        if (_store2 != nullptr) {
            _store2->Unregister(&_store2Sink);
            _store2->Release();
            _store2 = nullptr;
        }
        if (_storeCache != nullptr) {
            _storeCache->Release();
            _storeCache = nullptr;
        }
        if (_storeInspector != nullptr) {
            _storeInspector->Release();
            _storeInspector = nullptr;
        }
        if (_storeLimit != nullptr) {
            _storeLimit->Release();
            _storeLimit = nullptr;
        }
    }

    string PersistentStore::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
