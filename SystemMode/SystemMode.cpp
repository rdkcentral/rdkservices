/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#include "SystemMode.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::SystemMode> metadata(
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

    namespace Plugin
    {

    /*
     *Register SystemMode module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(SystemMode, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    SystemMode::SystemMode() : _service(nullptr), _connectionId(0), _systemMode(nullptr)
    {
	std::cout<<"RamTesting SystemMode cons "<<std::endl;
        SYSLOG(Logging::Startup, (_T("SystemMode Constructor")));
    }

    SystemMode::~SystemMode()
    {
	std::cout<<"RamTesting SystemMode distr "<<std::endl;
        SYSLOG(Logging::Shutdown, (string(_T("SystemMode Destructor"))));
    }

    const string SystemMode::Initialize(PluginHost::IShell* service)
    {
	std::cout<<"RamTesting SystemMode Initialize "<<std::endl;
        string message="";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == _systemMode);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("SystemMode::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        //_service->Register(&_systemModeNotification);
        _systemMode = _service->Root<Exchange::ISystemMode>(_connectionId, 5000, _T("SystemModeImplementation"));

        if(nullptr != _systemMode)
        {
            // Register for notifications
          //  _systemMode->Register(&_systemModeNotification);
            // Invoking Plugin API register to wpeframework
            Exchange::JSystemMode::Register(*this, _systemMode);
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("SystemMode::Initialize: Failed to initialise SystemMode plugin")));
            message = _T("SystemMode plugin could not be initialised");
        }

        if (0 != message.length())
        {
           Deinitialize(service);
        }

        return message;
    }

    void SystemMode::Deinitialize(PluginHost::IShell* service)
    {
	std::cout<<"RamTesting SystemMode  Deinitialize"<<std::endl;
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("SystemMode::Deinitialize"))));

        // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
//        _service->Unregister(&_systemModeNotification);

        if (nullptr != _systemMode)
        {
//            _systemMode->Unregister(&_systemModeNotification);
            Exchange::JSystemMode::Unregister(*this);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = _systemMode->Release();

            _systemMode = nullptr;

            // It should have been the last reference we are releasing,
            // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (nullptr != connection)
            {
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
        SYSLOG(Logging::Shutdown, (string(_T("SystemMode de-initialised"))));
    }

    string SystemMode::Information() const
    {
       // No additional info to report
       return (string());
    }

    void SystemMode::Deactivated(RPC::IRemoteConnection* connection)
    {
	std::cout<<"RamTesting SystemMode  SystemMode::Deactivated"<<std::endl;
        if (connection->Id() == _connectionId) {
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework
