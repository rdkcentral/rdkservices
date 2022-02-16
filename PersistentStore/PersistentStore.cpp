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

#include "SqliteStore.h"

namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(PersistentStore, 1, 0);

PersistentStore::PersistentStore()
    : PluginHost::JSONRPC(),
      _store(Core::Service<SqliteStore>::Create<Exchange::IStore>()),
      _storeCache(static_cast<SqliteStore *>(_store)),
      _storeListing(static_cast<SqliteStore *>(_store)),
      _storeSink(this)
{
    RegisterAll();
}

PersistentStore::~PersistentStore()
{
    UnregisterAll();

    _store->Release();
}

const string PersistentStore::Initialize(PluginHost::IShell *service)
{
    string result;

    ASSERT(service != nullptr);

    _config.FromString(service->ConfigLine());

    ASSERT(!_config.Path.Value().empty());

    Core::Directory(Core::File(_config.Path.Value()).PathName().c_str()).CreatePath();

    if (!Core::File(_config.Path.Value()).Exists()) {
        for (auto i : LegacyLocations()) {
            if (Core::File(i).Exists()) {
                if (!Core::File(i).Move(_config.Path.Value())) {
                    result = "move failed";
                }
                break;
            }
        }
    }

    if (result.empty()) {
        if (static_cast<SqliteStore *>(_store)->Open(_config.Path.Value(),
                         _config.Key.Value(),
                         _config.MaxSize.Value(),
                         _config.MaxValue.Value()) != Core::ERROR_NONE) {
            result = "init failed";
        }
    }

    if (result.empty()) {
        _storeSink.Initialize(_store);
    }

    return result;
}

void PersistentStore::Deinitialize(PluginHost::IShell * /* service */)
{
    static_cast<SqliteStore *>(_store)->Term();

    _storeSink.Deinitialize();
}

string PersistentStore::Information() const
{
    return (string());
}

std::vector<string> PersistentStore::LegacyLocations() const
{
    return {"/opt/persistent/rdkservicestore"};
}

} // namespace Plugin
} // namespace WPEFramework
