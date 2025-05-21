/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
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

#include "CryptoTP.h"

namespace WPEFramework {
namespace Plugin {

    namespace {

        static Metadata<CryptoTP> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            //{ subsystem::PLATFORM, subsystem::PROVISIONING },
            { subsystem::PLATFORM },
            // Terminations
            {},
            // Controls
            //{ subsystem::CRYPTOGRAPHY }
            { }
        );
    }

    const string CryptoTP::Initialize(PluginHost::IShell* service) /* override */
    {
        string message;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_CryptoTP == nullptr);
        ASSERT(_connectionId == 0);

        _service = service;
        _service->AddRef();

        _service->Register(&_notification);
        _CryptoTP = _service->Root<Exchange::IConfiguration>(_connectionId, Core::infinite, _T("CryptographyImplementation"));

        if (_CryptoTP == nullptr) {
            message = _T("CryptoTP could not be instantiated.");
        }
        else {
            _CryptoTP->Configure(_service);
            /*if (_CryptoTP->Configure(_service) == Core::ERROR_NONE) {
                PluginHost::ISubSystem* const subSystems = service->SubSystems();
                ASSERT(subSystems != nullptr);

                if (subSystems != nullptr) {
                    subSystems->Set(PluginHost::ISubSystem::CRYPTOGRAPHY, nullptr);
                    subSystems->Release();
                }
            }*/
        }

        return message;
    }

    void CryptoTP::Deinitialize(PluginHost::IShell* service) /* override */
    {
        ASSERT(service != nullptr);

        if (_service != nullptr) {
            ASSERT(_service == service);

            _service->Unregister(&_notification);

            if (_CryptoTP != nullptr) {
                RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));

                VARIABLE_IS_NOT_USED uint32_t result = _CryptoTP->Release();
                _CryptoTP = nullptr;
                ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

                if (connection != nullptr) {
                    TRACE(Trace::Error, (_T("CryptoTP is not properly destructed. %d"), _connectionId));

                    connection->Terminate();
                    connection->Release();
                }
            }

            _connectionId = 0;
            _service->Release();
            _service = nullptr;
        }
    }

    string CryptoTP::Information() const /* override */
    {
        return string();
    }

    void CryptoTP::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED,
                PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace
