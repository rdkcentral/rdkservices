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

#include "Account.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {
    namespace {
        static Plugin::Metadata<Plugin::Account> metadata(
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
        SERVICE_REGISTRATION(Account, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
   
    Account::Account() : _service(nullptr), _connectionId(0), _account(nullptr), _configure(nullptr), _accountNotification(this)
    {
        SYSLOG(Logging::Startup, (_T("Account Constructor")));
    }
    
    Account::~Account()
    {
        SYSLOG(Logging::Shutdown, (_T("Account Destructor")));
    }

    const string Account::Initialize(PluginHost::IShell* service)
    {
        string message = "";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == _account);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("Account::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        _service->Register(&_accountNotification);
        _account = _service->Root<Exchange::IAccount>(_connectionId, 5000, _T("AccountImplementation"));

        if (nullptr != _account)
        {
            _configure = _account->QueryInterface<Exchange::IConfiguration>();
            if (_configure != nullptr)
            {
                uint32_t result = _configure->Configure(_service);
                if (result != Core::ERROR_NONE)
                {
                    message = _T("Account could not be configured");
                }
            }
            else
            {
                message = _T("Account implementation did not provide a configuration interface");
            }

            // Invoking Plugin API register to wpeframework
            Exchange::JAccount::Register(*this, _account);
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("Account::Initialize: Failed to initialise Account plugin")));
            message = _T("Account plugin could not be initialised");
        }
        return message;
    }

    void Account::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("Account::Deinitialize"))));

        // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
        _service->Unregister(&_accountNotification);

        // Release configuration interface if it was acquired during initialization.
        if (nullptr != _configure)
        {
            _configure->Release();
            _configure = nullptr;
        }

        if (nullptr != _account)
        {
            Exchange::JAccount::Unregister(*this);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = _account->Release();

            _account = nullptr;

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
        SYSLOG(Logging::Shutdown, (string(_T("Account de-initialised"))));
    }

    string Account::Information() const
    {
       return ("This Account Plugin facilitates managing Hotel checkout operations");
    }

    void Account::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
}
}