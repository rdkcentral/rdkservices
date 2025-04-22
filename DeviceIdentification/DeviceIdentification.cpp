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
#include <interfaces/IConfiguration.h>
#include "tracing/Logging.h"
#include "UtilsJsonRpc.h"
#include "UtilsController.h"
#ifdef USE_THUNDER_R4
#include <interfaces/IDeviceInfo.h>
#else
#include <interfaces/IDeviceInfo2.h>
#endif /* USE_THUNDER_R4 */

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 7

namespace WPEFramework {
namespace {
    static Plugin::Metadata<Plugin::DeviceIdentification> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
#ifdef DISABLE_DEVICEID_CONTROL
        { PluginHost::ISubSystem::IDENTIFIER },
#else
        {},
#endif
        // Terminations
        {},
        // Controls
#ifdef DISABLE_DEVICEID_CONTROL
        {}
#else
        { PluginHost::ISubSystem::IDENTIFIER }
#endif
    );
}

namespace Plugin {

    SERVICE_REGISTRATION(DeviceIdentification, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    /* virtual */ const string DeviceIdentification::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_identifier == nullptr);
        ASSERT(_connectionId == 0);

        _service = service;
        _service->AddRef();

         string message;

        // Register the Process::Notification stuff. The Remote process might die before we get a
        // change to "register" the sink for these events !!! So do it ahead of instantiation.
        service->Register(&_notification);

        _identifier = service->Root<PluginHost::ISubSystem::IIdentifier>(_connectionId, RPC::CommunicationTimeOut, _T("DeviceImplementation"));

        if (_identifier != nullptr) {

            Exchange::IConfiguration* configure = _identifier->QueryInterface<Exchange::IConfiguration>();
            if (configure != nullptr) {
                configure->Configure(service);
                configure->Release();
            }

            _deviceId = GetDeviceId();

            RegisterAll();

            if (_deviceId.empty() != true) {
#ifndef DISABLE_DEVICEID_CONTROL
                service->SubSystems()->Set(PluginHost::ISubSystem::IDENTIFIER, _identifier);
#endif
            }
            else {
                message = _T("DeviceIdentification plugin could not be instantiated. No DeviceID available");
            }
        }
        else {
            message = _T("DeviceIdentification plugin could not be instantiated.");
        }

        if (message.length() != 0) {
            Deinitialize(service);
        }

        return message;
    }

    /* virtual */ void DeviceIdentification::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        _service->Unregister(&_notification);

        if (_deviceId.empty() != true) {
#ifndef DISABLE_DEVICEID_CONTROL
            service->SubSystems()->Set(PluginHost::ISubSystem::IDENTIFIER, nullptr);
#endif
            _deviceId.clear();
        }
        if(_identifier != nullptr) {

            UnregisterAll();

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);

            VARIABLE_IS_NOT_USED uint32_t result = _identifier->Release();
            _identifier = nullptr;

            // It should have been the last reference we are releasing,
            // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (connection != nullptr) {
                // Lets trigger the cleanup sequence for
                // out-of-process code. Which will guard
                // that unwilling processes, get shot if
                // not stopped friendly :-)
                connection->Terminate();
                connection->Release();
            }
         }

         _connectionId = 0;

        _service->Release();
        _service = nullptr;
    }

    /* virtual */ string DeviceIdentification::Information() const
    {
        // No additional info to report.
        return (string());
    }

    string DeviceIdentification::GetDeviceId() const
    {
        string result;
        string serial;
#ifndef DISABLE_DEVICEID_CONTROL
        ASSERT(_identifier != nullptr);

        if (_identifier != nullptr) {
            uint8_t myBuffer[64];

            myBuffer[0] = _identifier->Identifier(sizeof(myBuffer) - 1, &(myBuffer[1]));

            if (myBuffer[0] != 0) {
                result = Core::SystemInfo::Instance().Id(myBuffer, ~0);
            }
            else
            {
                serial = RetrieveSerialNumberThroughCOMRPC();

                if (!serial.empty()) {
                    uint8_t ret = serial.length();

                    if (ret > (sizeof(myBuffer) - 1)){
                        ret = sizeof(myBuffer) - 1;
                    }
                    myBuffer[0] = ret;
                    ::memcpy(&(myBuffer[1]), serial.c_str(), ret);

                    if(myBuffer[0] != 0){
                        result = Core::SystemInfo::Instance().Id(myBuffer, ~0);
                    }
                }
            }
            
        }
#else
        // extract DeviceId set by Thunder
        if (_service->SubSystems()->IsActive(PluginHost::ISubSystem::IDENTIFIER) == true) {

            const PluginHost::ISubSystem::IIdentifier* identifier(_service->SubSystems()->Get<PluginHost::ISubSystem::IIdentifier>());

            if (identifier != nullptr) {
                uint8_t myBuffer[64];

                if ((myBuffer[0] = identifier->Identifier(sizeof(myBuffer) - 1, &(myBuffer[1]))) != 0) {
                    result = Core::SystemInfo::Instance().Id(myBuffer, ~0);
                }
                identifier->Release();
            }
         }
#endif
        return result;
    }
    string DeviceIdentification::RetrieveSerialNumberThroughCOMRPC() const
    {
        std::string Number;
        if (_service)
        {
            PluginHost::IShell::state state;

        if ((Utils::getServiceState(_service, "DeviceInfo", state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
        {
            Utils::activatePlugin(_service, "DeviceInfo");
        }
        if ((Utils::getServiceState(_service, "DeviceInfo", state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
        {
            auto _remoteDeviceInfoObject = _service->QueryInterfaceByCallsign<Exchange::IDeviceInfo>("DeviceInfo");

        if(_remoteDeviceInfoObject)
                {
                    _remoteDeviceInfoObject->SerialNumber(Number);
                    _remoteDeviceInfoObject->Release();
                }
        }
        else
        {
            LOGERR("Failed to create DeviceInfo object\n");
        }
        }
        return Number;
    }
    void DeviceIdentification::Info(JsonData::DeviceIdentification::DeviceidentificationData& deviceInfo) const
    {
        deviceInfo.Firmwareversion = _identifier->FirmwareVersion();
        deviceInfo.Chipset = _identifier->Chipset();

        if (_deviceId.empty() != true) {
            deviceInfo.Deviceid = _deviceId;
        }
    }

    void DeviceIdentification::Deactivated(RPC::IRemoteConnection* connection)
    {
        // This can potentially be called on a socket thread, so the deactivation (wich in turn kills this object) must be done
        // on a seperate thread. Also make sure this call-stack can be unwound before we are totally destructed.
        if (_connectionId == connection->Id()) {

            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
} // namespace Plugin
} // namespace WPEFramework
