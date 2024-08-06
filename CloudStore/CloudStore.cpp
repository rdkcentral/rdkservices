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

#include "CloudStore.h"
#ifdef WITH_RFC
#include "rfcapi.h"
#endif

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

namespace Plugin {

    namespace {

        static Metadata<CloudStore> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {});
    }

    SERVICE_REGISTRATION(CloudStore, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    const string CloudStore::Initialize(PluginHost::IShell* service)
    {
        string result;

        ASSERT(service != nullptr);
        ASSERT(_store2 == nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_connectionId == 0);

        SYSLOG(Logging::Startup, (_T("CloudStore::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();

        auto configLine = _service->ConfigLine();
        _config.FromString(configLine);

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

        _service->Register(&_notification);

        _store2 = _service->Root<Exchange::IStore2>(_connectionId, RPC::CommunicationTimeOut, _T("CloudStoreImplementation"));
        if (_store2 != nullptr) {
            Exchange::JStore2::Register(*this, _store2);
            _store2->Register(&_store2Sink);
        } else {
            result = _T("Couldn't create implementation instance");
        }

        return result;
    }

    void CloudStore::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("CloudStore::Deinitialize"))));

        _service->Unregister(&_notification);

        if (_store2 != nullptr) {
            _store2->Unregister(&_store2Sink);
            Exchange::JStore2::Unregister(*this);

            auto connection = _service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED auto result = _store2->Release();
            _store2 = nullptr;
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
            if (connection != nullptr) {
                connection->Terminate();
                connection->Release();
            }
        }

        _connectionId = 0;
        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("CloudStore de-initialised"))));
    }

    string CloudStore::Information() const
    {
        return (string());
    }

} // namespace Plugin
} // namespace WPEFramework
