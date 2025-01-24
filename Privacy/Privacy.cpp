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

#include "Privacy.h"
#include <interfaces/IConfiguration.h>

#define API_VERSION_NUMBER_MAJOR    PRIVACY_MAJOR_VERSION
#define API_VERSION_NUMBER_MINOR    PRIVACY_MINOR_VERSION
#define API_VERSION_NUMBER_PATCH    PRIVACY_PATCH_VERSION

namespace WPEFramework {

namespace {
    static Plugin::Metadata<Plugin::Privacy> metadata(
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

namespace Plugin {
    SERVICE_REGISTRATION(Privacy, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    Privacy::Privacy(): mService(nullptr), mConnectionId(0), mPrivacy(nullptr), mNotification(this)
    {
        SYSLOG(Logging::Startup, (_T("Privacy Constructor")));
    }

    Privacy::~Privacy()
    {
        SYSLOG(Logging::Shutdown, (string(_T("Privacy Destructor"))));
    }

    /* virtual */ const string Privacy::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        ASSERT(mPrivacy == nullptr);

        SYSLOG(Logging::Startup, (_T("Privacy::Initialize: PID=%u"), getpid()));

        mService = service;
        mService->AddRef();
        mService->Register(&mNotification);

        mPrivacy = service->Root<Exchange::IPrivacy>(mConnectionId, 2000, _T("PrivacyImplementation"));
        ASSERT(mPrivacy != nullptr);

        if (mPrivacy != nullptr) {

            // Register for notifications
            mPrivacy->Register(&mNotification);

            auto configConnection = mPrivacy->QueryInterface<Exchange::IConfiguration>();
            if (configConnection != nullptr) {
                configConnection->Configure(service);
                configConnection->Release();
            }
            // Invoking Plugin API register to wpeframework
            Exchange::JPrivacy::Register(*this, mPrivacy);  
        }
        else
        {
            SYSLOG(Logging::Startup, (_T("Privacy::Initialize: Failed to initialise Privacy plugin")));
        }
        // On success return empty, to indicate there is no error text.
        return ((mPrivacy != nullptr))
            ? EMPTY_STRING
            : _T("Could not retrieve the Privacy interface.");
    }

    /* virtual */ void Privacy::Deinitialize(PluginHost::IShell* service)
    {
        SYSLOG(Logging::Shutdown, (string(_T("Privacy::Deinitialize"))));
        ASSERT(service == mService);

        mService->Unregister(&mNotification);

        if (mPrivacy != nullptr) {

            mPrivacy->Unregister(&mNotification);
            Exchange::JPrivacy::Unregister(*this);

            RPC::IRemoteConnection *connection(service->RemoteConnection(mConnectionId));
            VARIABLE_IS_NOT_USED uint32_t result = mPrivacy->Release();
            mPrivacy = nullptr;

            // It should have been the last reference we are releasing,
            // so it should end up in a DESCRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (connection != nullptr)
            {
                // Lets trigger a cleanup sequence for
                // out-of-process code. Which will guard
                // that unwilling processes, get shot if
                // not stopped friendly :~)
                connection->Terminate();
                connection->Release();
            }
        }

        mConnectionId = 0;
        mService->Release();
        mService = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("Privacy de-initialised"))));
    }

    void Privacy::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == mConnectionId) {

            ASSERT(mService != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(mService, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework   