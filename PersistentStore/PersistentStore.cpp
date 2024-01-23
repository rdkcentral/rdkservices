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

#include "sqlite/Handle.h"
#include "sqlite/Store2Type.h"
#include "sqlite/Store2WithReconnectType.h"
#if defined(WITH_CLOCK_SYNC)
#include "sqlite/Store2WithClockSyncType.h"
#endif
#include "sqlite/StoreCacheType.h"
#include "sqlite/StoreInspectorType.h"
#include "sqlite/StoreLimitType.h"

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

#if defined(WITH_CLOCK_SYNC)
    class SqliteStore2 : public Sqlite::Store2WithClockSyncType<Sqlite::Store2WithReconnectType<Sqlite::Store2Type<Sqlite::Handle>>> {
    };
#else
    class SqliteStore2 : public Sqlite::Store2WithReconnectType<Sqlite::Store2Type<Sqlite::Handle>> {
    };
#endif
    class SqliteStoreCache : public Sqlite::StoreCacheType<Sqlite::Handle> {
    };
    class SqliteStoreInspector : public Sqlite::StoreInspectorType<Sqlite::Handle> {
    };
    class SqliteStoreLimit : public Sqlite::StoreLimitType<Sqlite::Handle> {
    };

    SERVICE_REGISTRATION(SqliteStore2, 1, 0);
    SERVICE_REGISTRATION(SqliteStoreCache, 1, 0);
    SERVICE_REGISTRATION(SqliteStoreInspector, 1, 0);
    SERVICE_REGISTRATION(SqliteStoreLimit, 1, 0);

    SERVICE_REGISTRATION(PersistentStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    const string PersistentStore::Initialize(PluginHost::IShell* service)
    {
        string result;

        ASSERT(service != nullptr);

        auto configLine = service->ConfigLine();
        _config.FromString(configLine);

        ASSERT(!_config.Path.Value().empty());

        {
            Core::File file(_config.Path.Value());
            Core::File oldFile(_config.LegacyPath.Value());
            if (file.Name() != oldFile.Name()) {
                if (oldFile.Exists()) {
                    if (!file.Exists()) {
                        // Rename will fail if the files are not on the same filesystem
                        // Make a copy and then remove the original file
                        std::ifstream in(oldFile.Name(), std::ios::in | std::ios::binary);
                        std::ofstream out(file.Name(), std::ios::out | std::ios::binary);
                        out << in.rdbuf();
                    }

                    oldFile.Destroy();
                }
            }
        }

        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_PATH"), _config.Path.Value());
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXSIZE"), std::to_string(_config.MaxSize.Value()));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_MAXVALUE"), std::to_string(_config.MaxValue.Value()));
        Core::SystemInfo::SetEnvironment(_T("PERSISTENTSTORE_LIMIT"), std::to_string(_config.Limit.Value()));

        uint32_t connectionId;

        Store2::ScopeMapType initList1 = {
            { Exchange::IStore2::ScopeType::DEVICE, service->Root<Exchange::IStore2>(connectionId, 2000, _T("SqliteStore2")) }
        };
        _store2 = Core::Service<Store2>::Create<Exchange::IStore2>(initList1);
        _store2->Register(&_store2Sink);
        _store = Core::Service<Store>::Create<Exchange::IStore>(_store2);
        _storeCache = service->Root<Exchange::IStoreCache>(connectionId, 2000, _T("SqliteStoreCache"));
        _storeInspector = service->Root<Exchange::IStoreInspector>(connectionId, 2000, _T("SqliteStoreInspector"));
        _storeLimit = service->Root<Exchange::IStoreLimit>(connectionId, 2000, _T("SqliteStoreLimit"));

        return result;
    }

    void PersistentStore::Deinitialize(PluginHost::IShell* /* service */)
    {
        if (_store) {
            _store->Release();
            _store = nullptr;
        }
        if (_store2) {
            _store2->Unregister(&_store2Sink);
            _store2->Release();
            _store2 = nullptr;
        }
        if (_storeCache) {
            _storeCache->Release();
            _storeCache = nullptr;
        }
        if (_storeInspector) {
            _storeInspector->Release();
            _storeInspector = nullptr;
        }
        if (_storeLimit) {
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
