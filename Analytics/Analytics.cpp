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

#include "Analytics.h"
#include <interfaces/IConfiguration.h>

#define API_VERSION_NUMBER_MAJOR    ANALYTICS_MAJOR_VERSION
#define API_VERSION_NUMBER_MINOR    ANALYTICS_MINOR_VERSION
#define API_VERSION_NUMBER_PATCH    ANALYTICS_PATCH_VERSION

namespace WPEFramework {

namespace {
    static Plugin::Metadata<Plugin::Analytics> metadata(
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
    SERVICE_REGISTRATION(Analytics, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    /* virtual */ const string Analytics::Initialize(PluginHost::IShell* service)
    {
        ASSERT(service != nullptr);
        mService = service;

        ASSERT(mAnalytics == nullptr);

        mAnalytics = service->Root<Exchange::IAnalytics>(mConnectionId, 2000, _T("AnalyticsImplementation"));
        ASSERT(mAnalytics != nullptr);

        if (mAnalytics != nullptr) {
            auto configConnection = mAnalytics->QueryInterface<Exchange::IConfiguration>();
            if (configConnection != nullptr) {
                configConnection->Configure(service);
                configConnection->Release();
            }
            RegisterAll();
        }
        // On success return empty, to indicate there is no error text.
        return ((mAnalytics != nullptr))
            ? EMPTY_STRING
            : _T("Could not retrieve the Analytics interface.");
    }

    /* virtual */ void Analytics::Deinitialize(PluginHost::IShell* service)
    {
        TRACE(Trace::Information, (_T("Analytics::Deinitialize")));
        ASSERT(service == mService);

        if (mAnalytics != nullptr) {

            RPC::IRemoteConnection *connection(service->RemoteConnection(mConnectionId));
            VARIABLE_IS_NOT_USED uint32_t result = mAnalytics->Release();
            mAnalytics = nullptr;

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
    }

    void Analytics::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == mConnectionId) {
            TRACE(Trace::Information, (_T("Analytics::Deactivated")));

            ASSERT(mService != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(mService, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin
} // namespace WPEFramework   