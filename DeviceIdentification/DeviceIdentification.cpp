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
 
#include "DeviceIdentification.h"
#include "IdentityProvider.h"

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(DeviceIdentification, 1, 0);

    /* virtual */ const string DeviceIdentification::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

        string message;

        _identifier = service->Root<PluginHost::ISubSystem::IIdentifier>(_connectionId, RPC::CommunicationTimeOut, _T("DeviceImplementation"));

        if (_identifier != nullptr) {
            _deviceId = GetDeviceId();

            if (_deviceId.empty() != true) {
                service->SubSystems()->Set(PluginHost::ISubSystem::IDENTIFIER, _identifier);
            }
        }

        if (_identifier == nullptr) {
            message = _T("DeviceIdentification plugin could not be instantiated.");
        }

        return message;
    }

    /* virtual */ void DeviceIdentification::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);

        ASSERT(_identifier != nullptr);
        if (_identifier != nullptr) {
            if (_deviceId.empty() != true) {
                service->SubSystems()->Set(PluginHost::ISubSystem::IDENTIFIER, nullptr);
                _deviceId.clear();
            }
            _identifier->Release();
            _identifier = nullptr;
        }
        _connectionId = 0;
    }

    /* virtual */ string DeviceIdentification::Information() const
    {
        // No additional info to report.
        return (string());
    }

    string DeviceIdentification::GetDeviceId() const
    {
        string result;
        ASSERT(_identifier != nullptr);

        if (_identifier != nullptr) {
            uint8_t myBuffer[64];

            myBuffer[0] = _identifier->Identifier(sizeof(myBuffer) - 1, &(myBuffer[1]));

            if (myBuffer[0] != 0) {
                result = Core::SystemInfo::Instance().Id(myBuffer, ~0);
            }
        }

        return result;
    }

    void DeviceIdentification::Info(JsonData::DeviceIdentification::DeviceidentificationData& deviceInfo) const
    {
        deviceInfo.Firmwareversion = _identifier->FirmwareVersion();
        deviceInfo.Chipset = _identifier->Chipset();

        if (_deviceId.empty() != true) {
            deviceInfo.Deviceid = _deviceId;
        }
    }

} // namespace Plugin
} // namespace WPEFramework
