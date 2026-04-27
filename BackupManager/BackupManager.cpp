/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2026 RDK Management
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

#include "BackupManager.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {
    namespace {
        static Plugin::Metadata<Plugin::BackupManager> metadata(
            API_VERSION_NUMBER_MAJOR,
            API_VERSION_NUMBER_MINOR,
            API_VERSION_NUMBER_PATCH,
            // Precondition: none
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {
        SERVICE_REGISTRATION(BackupManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
   
    BackupManager::BackupManager() : _service(nullptr), _connectionId(0), _backup(nullptr), _configure(nullptr), _backupNotification(this)
    {
        SYSLOG(Logging::Startup, (_T("BackupManager Constructor")));
    }
    
    BackupManager::~BackupManager()
    {
        SYSLOG(Logging::Shutdown, (_T("BackupManager Destructor")));
    }

    const string BackupManager::Initialize(PluginHost::IShell* service)
    {
        string message = "";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == _backup);
        ASSERT(nullptr == _configure);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("BackupManager::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        _service->Register(&_backupNotification);
        _backup = _service->Root<Exchange::IBackupManager>(_connectionId, 5000, _T("BackupManagerImplementation"));

        if (nullptr != _backup)
        {
            _configure = _backup->QueryInterface<Exchange::IConfiguration>();
            if (_configure != nullptr)
            {
                uint32_t result = _configure->Configure(_service);
                if (result != Core::ERROR_NONE)
                {
                    LOGERR("Failed to configure BackupManager plugin");
                    message = _T("BackupManager could not be configured");
                }
            }
            else
            {
                LOGERR("IConfiguration interface is not available");
                message = _T("BackupManager could not be configured");
            }
            // Invoking Plugin API register to wpeframework
            Exchange::JBackupManager::Register(*this, _backup);
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("BackupManager::Initialize: Failed to initialise BackupManager plugin")));
            message = _T("BackupManager plugin could not be initialised");
        }
        return message;
    }

    void BackupManager::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("BackupManager::Deinitialize"))));

        // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
        _service->Unregister(&_backupNotification);

        if (nullptr != _backup)
        {
            if (_configure != nullptr)
            {
                _configure->Release();
                _configure = nullptr;
            }

            Exchange::JBackupManager::Unregister(*this);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = _backup->Release();

            _backup = nullptr;

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
               try
               {
                   connection->Terminate();
                   // Log success if needed
                   LOGWARN("Connection terminated successfully.");
               }
               catch (const std::exception& e)
               {
                   std::string errorMessage = "Failed to terminate connection: ";
                   errorMessage += e.what();
                   LOGWARN("%s",errorMessage.c_str());
               }

               connection->Release();
            }
        }

        _connectionId = 0;
        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("BackupManager de-initialised"))));
    }

    string BackupManager::Information() const
    {
       return ("This BackupManager Plugin provides backup and restore functionalities for other plugins.");
    }

    void BackupManager::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
}
}