/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include "SystemAudioPlayer.h"

#define SAP_MAJOR_VERSION 1
#define SAP_MINOR_VERSION 0
#define API_VERSION_NUMBER 1

namespace WPEFramework {
namespace Plugin {

    /*
     *Register SystemAudioPlayer module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(SystemAudioPlayer, SAP_MAJOR_VERSION, SAP_MINOR_VERSION);

    const string SystemAudioPlayer::Initialize(PluginHost::IShell* service)
    {
        ASSERT(_service == nullptr);

        _connectionId = 0;
        _service = service;
        _skipURL = static_cast<uint8_t>(_service->WebPrefix().length());

        _service->Register(&_notification);

        _sap = _service->Root<Exchange::ISystemAudioPlayer>(_connectionId, 5000, _T("SystemAudioPlayerImplementation"));

        std::string message;
        if(_sap != nullptr) {
            ASSERT(_connectionId != 0);

            _sap->Configure(_service);
            _sap->Register(&_notification);
            RegisterAll();
        } else {
            message = _T("SystemAudioPlayer could not be instantiated.");
            _service->Unregister(&_notification);
            _service = nullptr;
        }

        return message;
    }

    void SystemAudioPlayer::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);
        ASSERT(_sap != nullptr);

        if (!_sap)
            return;

        _sap->Unregister(&_notification);
        _service->Unregister(&_notification);

        if(_sap->Release() != Core::ERROR_DESTRUCTION_SUCCEEDED) {
            ASSERT(_connectionId != 0);
            TRACE_L1("SystemAudioPlayer Plugin is not properly destructed. %d", _connectionId);

            RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

            // The process can disappear in the meantime...
            if (connection != nullptr) {
                // But if it did not dissapear in the meantime, forcefully terminate it. Shoot to kill :-)
                connection->Terminate();
                connection->Release();
            }
        }
        // Deinitialize what we initialized..
        _service = nullptr;
        _sap = nullptr;
    }

    SystemAudioPlayer::SystemAudioPlayer()
            : PluginHost::JSONRPC()
            , _apiVersionNumber(API_VERSION_NUMBER)
            , _notification(this)
    {
    }

    SystemAudioPlayer::~SystemAudioPlayer()
    {
    }

    void SystemAudioPlayer::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);
            SAPLOG_WARNING("SystemAudioPlayer::Deactivated - %p", this);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework
