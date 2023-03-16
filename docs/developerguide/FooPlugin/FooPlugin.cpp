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

#include "FooPlugin.h"


#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace {

    static Plugin::Metadata<Plugin::FooPlugin> metadata(
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

SERVICE_REGISTRATION(FooPlugin, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

FooPlugin::FooPlugin()
    : PluginHost::JSONRPC(),
{
    RegisterAll();
}

FooPlugin::~FooPlugin()
{
    UnregisterAll();
}

const string FooPlugin::Initialize(PluginHost::IShell *service)
{
    string result;

    ASSERT(service != nullptr);

    string configLine = service->ConfigLine();
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

void FooPlugin::Deinitialize(PluginHost::IShell * /* service */)
{

}

string FooPlugin::Information() const
{
    return (string());
}

void PersistentStore::RegisterAll()
{
    Register<JsonObject, JsonObject>(_T("setValue"), &PersistentStore::endpoint_setValue, this);
}

void PersistentStore::UnregisterAll()
{
    Unregister(_T("setValue"));
}

uint32_t PersistentStore::endpoint_setValue(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    bool success = false;

    if (!parameters.HasLabel("key") ||
        !parameters.HasLabel("value")) {
        response["error"] = "params missing";
    }
    else {
        string key = parameters["key"].String();
        string value = parameters["value"].String();

        if (key.empty()) {
            response["error"] = "params empty";
        }
        else {
            auto status = _store->SetValue(key, value);
            if (status == Core::ERROR_INVALID_INPUT_LENGTH) {
                response["error"] = "params too long";
            }
            success = (status == Core::ERROR_NONE);
        }
    }

    returnResponse(success);
}

void PersistentStore::event_onValueChanged(const string &key, const string &value)
{
    JsonObject params;
    params["key"] = key;
    params["value"] = value;

    sendNotify(_T("onValueChanged"), params);
}

} // namespace Plugin
} // namespace WPEFramework
