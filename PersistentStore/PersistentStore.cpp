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

#if defined(SQLITE_HAS_CODEC)
#if defined(USE_PLABELS)
#include "sqlite/see/SqliteSeeDbPlabels.h"
#else
#include "sqlite/see/SqliteSeeDb.h"
#endif
#else
#include "sqlite/SqliteDb.h"
#endif

#include "sqlite/SqliteStore2.h"
#include "sqlite/SqliteStore2WithReconnect.h"
#if defined(WITH_CLOCK_SYNC)
#include "sqlite/SqliteStore2WithClockSync.h"
#endif
#include "sqlite/SqliteStoreCache.h"
#include "sqlite/SqliteStoreInspector.h"
#include "sqlite/SqliteStoreInspectorWithReconnect.h"

#include <fstream>

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 2

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

        auto sqliteDb =
#if defined(SQLITE_HAS_CODEC)
#if defined(USE_PLABELS)
            Core::Service<SqliteSeeDb>::Create<ISqliteDb>(
                _config.Path.Value(), _config.MaxSize.Value(), _config.MaxValue.Value(), _config.Key.Value())
#else
            Core::Service<SqliteSeeDbPlabels>::Create<ISqliteDb>(
                _config.Path.Value(), _config.MaxSize.Value(), _config.MaxValue.Value(), _config.Key.Value())
#endif
#else
            Core::Service<SqliteDb>::Create<ISqliteDb>(
                _config.Path.Value(), _config.MaxSize.Value(), _config.MaxValue.Value())
#endif
            ;
        if (sqliteDb->Open() != Core::ERROR_NONE) {
            result = "init failed";
        } else {
            _store2 = Core::Service<Store2>::Create<Exchange::IStore2>();
            static_cast<Store2*>(_store2)->ScopeMap.emplace(
                Exchange::IStore2::ScopeType::DEVICE,
#if defined(WITH_CLOCK_SYNC)
                Core::Service<SqliteStore2WithClockSync<SqliteStore2WithReconnect<SqliteStore2>>>::Create<Exchange::IStore2>(sqliteDb))
#else
                Core::Service<SqliteStore2WithReconnect<SqliteStore2>>::Create<Exchange::IStore2>(sqliteDb))
#endif
                ;
            _store2->Register(&_store2Sink);

            _storeCache = Core::Service<SqliteStoreCache>::Create<Exchange::IStoreCache>(sqliteDb);

            _storeInspector = Core::Service<StoreInspector>::Create<Exchange::IStoreInspector>();
            static_cast<StoreInspector*>(_storeInspector)->ScopeMap.emplace(Exchange::IStoreInspector::ScopeType::DEVICE, Core::Service<SqliteStoreInspectorWithReconnect<SqliteStoreInspector>>::Create<Exchange::IStoreInspector>(sqliteDb));

            _store = Core::Service<Store>::Create<Exchange::IStore>(_store2);
        }
        sqliteDb->Release();

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
    }

    string PersistentStore::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
