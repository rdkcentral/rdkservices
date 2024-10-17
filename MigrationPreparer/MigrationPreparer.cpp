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

#include "MigrationPreparer.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::MigrationPreparer> metadata(
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
     *Register MigrationPreparer module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(MigrationPreparer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    MigrationPreparer::MigrationPreparer() : _service(nullptr), _connectionId(0), _migrationPreparer(nullptr)
                                            , _migrationPreparerNotification(this)
    {
        SYSLOG(Logging::Startup, (_T("MigrationPreparer Constructor")));
        RegisterAll();
    }

    MigrationPreparer::~MigrationPreparer()
    {
        SYSLOG(Logging::Shutdown, (string(_T("MigrationPreparer Destructor"))));
        UnregisterAll();
    }

    const string MigrationPreparer::Initialize(PluginHost::IShell* service)
    {
        string message="";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == _migrationPreparer);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("MigrationPreparer::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        _service->Register(&_migrationPreparerNotification);
        _migrationPreparer = _service->Root<Exchange::IMigrationPreparer>(_connectionId, 5000, _T("MigrationPreparerImplementation"));
        if(nullptr != _migrationPreparer)
        {
            // Register for notifications
            _migrationPreparer->Register(&_migrationPreparerNotification);
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("MigrationPreparer::Initialize: Failed to initialise MigrationPreparer plugin")));
            message = _T("MigrationPreparer plugin could not be initialised");
        }

        if (0 != message.length())
        {
           Deinitialize(service);
        }

        return message;
    }

    void MigrationPreparer::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("MigrationPreparer::Deinitialize"))));
        // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
        _service->Unregister(&_migrationPreparerNotification);

        if (nullptr != _migrationPreparer)
        {
            _migrationPreparer->Unregister(&_migrationPreparerNotification);
            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = _migrationPreparer->Release();

            _migrationPreparer = nullptr;

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
        SYSLOG(Logging::Shutdown, (string(_T("MigrationPreparer de-initialised"))));
    }

    string MigrationPreparer::Information() const
    {
       return (string("{\"service\": \"") + string("org.rdk.MigrationPreparer") + string("\"}"));
    }

    void MigrationPreparer::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework