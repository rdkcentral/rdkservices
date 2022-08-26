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

#include "UtilsFile.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

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
        {}
    );
}

namespace Plugin {

SERVICE_REGISTRATION(PersistentStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

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

    string configLine = service->ConfigLine();

    // TODO
    if (configLine == "{}") {
        configLine = "{\n"
                     "\"path\":\"/opt/secure/persistent/rdkservicestore\",\n"
                     "\"key\":null,\n"
                     "\"maxsize\":1000000,\n"
                     "\"maxvalue\":1000\n"
                     "}";
    }

    _config.FromString(configLine);

    ASSERT(!_config.Path.Value().empty());

    Core::File file(_config.Path.Value());

    Core::Directory(file.PathName().c_str()).CreatePath();

    if (!file.Exists()) {
        for (auto i : LegacyLocations()) {
            Core::File from(i);

            if (from.Exists()) {
                if (!Utils::MoveFile(from.Name(), file.Name())) {
                    result = "move failed";
                }
                break;
            }
        }
    }

    if (result.empty()) {
        if (static_cast<SqliteStore *>(_store)->Open(
            _config.Path.Value(),
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
