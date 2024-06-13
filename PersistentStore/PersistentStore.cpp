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
#define API_VERSION_NUMBER_PATCH 9

namespace WPEFramework {

namespace {

#ifdef WITH_THUNDER_NAMESPACE
    static Thunder::Plugin::Metadata<Plugin::PersistentStore> metadata(
#else
    static Plugin::Metadata<Plugin::PersistentStore> metadata(
#endif
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
        ASSERT(_store == nullptr);
        ASSERT(_store2 == nullptr);
        ASSERT(_storeCache == nullptr);
        ASSERT(_storeInspector == nullptr);
        ASSERT(_storeLimit == nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_connectionId == 0);

        SYSLOG(Logging::Startup, (_T("PersistentStore::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();

        auto configLine = _service->ConfigLine();
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

        _service->Register(&_notification);

        _store = _service->Root<Exchange::IStore>(_connectionId, RPC::CommunicationTimeOut, _T("PersistentStoreImplementation"));
        if (_store != nullptr) {
            _store2 = _store->QueryInterface<Exchange::IStore2>();
            _storeCache = _store->QueryInterface<Exchange::IStoreCache>();
            _storeInspector = _store->QueryInterface<Exchange::IStoreInspector>();
            _storeLimit = _store->QueryInterface<Exchange::IStoreLimit>();

            ASSERT(_store2 != nullptr);
            ASSERT(_storeCache != nullptr);
            ASSERT(_storeInspector != nullptr);
            ASSERT(_storeLimit != nullptr);
        } else {
            result = _T("Couldn't create implementation instance");
        }

        return result;
    }

    void PersistentStore::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("DTV::Deinitialize"))));

        _service->Unregister(&_notification);

        if (_store != nullptr) {
            if (_store2 != nullptr) {
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

            auto connection = _service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED auto result = _store->Release();
            _store = nullptr;
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
            if (connection != nullptr) {
                connection->Terminate();
                connection->Release();
            }
        }

        _connectionId = 0;
        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("PersistentStore de-initialised"))));
    }

    string PersistentStore::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
