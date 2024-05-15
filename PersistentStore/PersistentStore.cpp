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
#ifdef WITH_RFC
#include "rfcapi.h"
#endif

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 12

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

        auto uri = _config.Uri.Value();

#ifdef WITH_RFC
        RFC_ParamData_t rfcParam;
        auto rfcStatus = getRFCParameter(nullptr, URI_RFC, &rfcParam);
        if (rfcStatus == WDMP_SUCCESS) {
            if (rfcParam.value[0]) {
                uri = rfcParam.value;
            }
        } else {
            TRACE(Trace::Error, (_T("%s rfc error %d"), __FUNCTION__, rfcStatus));
        }
#endif

        Core::SystemInfo::SetEnvironment(URI_ENV, uri);
        Core::SystemInfo::SetEnvironment(PATH_ENV, _config.Path.Value());
        Core::SystemInfo::SetEnvironment(MAXSIZE_ENV, std::to_string(_config.MaxSize.Value()));
        Core::SystemInfo::SetEnvironment(MAXVALUE_ENV, std::to_string(_config.MaxValue.Value()));
        Core::SystemInfo::SetEnvironment(LIMIT_ENV, std::to_string(_config.Limit.Value()));

        uint32_t connectionId;

        _deviceStore2 = service->Root<Exchange::IStore2>(connectionId, RPC::CommunicationTimeOut, _T("SqliteStore2"));
        if (_deviceStore2 != nullptr) {
            _deviceStore2->Register(&_store2Sink);
            _deviceStore2->Register(_store);
            _deviceStoreCache = _deviceStore2->QueryInterface<Exchange::IStoreCache>();
            _deviceStoreInspector = _deviceStore2->QueryInterface<Exchange::IStoreInspector>();
            _deviceStoreLimit = _deviceStore2->QueryInterface<Exchange::IStoreLimit>();
        }

        _accountStore2 = service->Root<Exchange::IStore2>(connectionId, RPC::CommunicationTimeOut, _T("GrpcStore2"));
        if (_accountStore2 != nullptr) {
            _accountStore2->Register(&_store2Sink);
        }

        return result;
    }

    void PersistentStore::Deinitialize(PluginHost::IShell* /* service */)
    {
        if (_deviceStore2 != nullptr) {
            _deviceStore2->Unregister(&_store2Sink);
            _deviceStore2->Unregister(_store);
            _deviceStore2->Release();
            _deviceStore2 = nullptr;
        }
        if (_deviceStoreCache != nullptr) {
            _deviceStoreCache->Release();
            _deviceStoreCache = nullptr;
        }
        if (_deviceStoreInspector != nullptr) {
            _deviceStoreInspector->Release();
            _deviceStoreInspector = nullptr;
        }
        if (_deviceStoreLimit != nullptr) {
            _deviceStoreLimit->Release();
            _deviceStoreLimit = nullptr;
        }
        if (_accountStore2 != nullptr) {
            _accountStore2->Unregister(&_store2Sink);
            _accountStore2->Release();
            _accountStore2 = nullptr;
        }
    }

    string PersistentStore::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
