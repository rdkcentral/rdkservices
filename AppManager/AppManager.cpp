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

#include "AppManager.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::AppManager> metadata(
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
     *Register AppManager module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(AppManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    AppManager::AppManager() : _service(nullptr), _connectionId(0), mAppManager(nullptr), mAppManagerNotification(this)
    {
        SYSLOG(Logging::Startup, (_T("AppManager Constructor")));
    }

    AppManager::~AppManager()
    {
        SYSLOG(Logging::Shutdown, (string(_T("AppManager Destructor"))));
    }

    const string AppManager::Initialize(PluginHost::IShell* service)
    {
        string message="";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == mAppManager);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("AppManager::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        _service->Register(&mAppManagerNotification);
        mAppManager = _service->Root<Exchange::IAppManager>(_connectionId, 5000, _T("AppManagerImplementation"));

        if(nullptr != mAppManager)
        {
	    Core::hresult result = mAppManager->Initialize(_service);
	    if (result == Core::ERROR_NONE)
	    {	    
                // Register for notifications
                mAppManager->Register(&mAppManagerNotification);
                // Invoking Plugin API register to wpeframework
                Exchange::JAppManager::Register(*this, mAppManager);
            }
            else
	    {
                message = _T("AppManager plugin could not be initialised, as remote failed");
	    }	    
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("AppManager::Initialize: Failed to initialise AppManager plugin")));
            Deinitialize(service);
            message = _T("AppManager plugin could not be initialised");
        }
        return message;
    }

    void AppManager::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("AppManager::Deinitialize"))));

        // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
        _service->Unregister(&mAppManagerNotification);

        if (nullptr != mAppManager)
        {
            mAppManager->Unregister(&mAppManagerNotification);
	    mAppManager->Deinitialize(_service);
            Exchange::JAppManager::Unregister(*this);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = mAppManager->Release();

            mAppManager = nullptr;

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
        SYSLOG(Logging::Shutdown, (string(_T("AppManager de-initialised"))));
    }

    string AppManager::Information() const
    {
        // No additional info to report
        return (string());
    }

    void AppManager::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId)
	{
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }
} // namespace Plugin
} // namespace WPEFramework
