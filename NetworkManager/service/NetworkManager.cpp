/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
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
**/

#include "NetworkManager.h"

#include <random>

namespace WPEFramework
{
    namespace Plugin
    {
        static Plugin::Metadata<Plugin::NetworkManager> metadata(
            // Version (Major, Minor, Patch)
            NETWORKMANAGER_MAJOR_VERSION, NETWORKMANAGER_MINOR_VERSION, NETWORKMANAGER_PATCH_VERSION,
            // Preconditions
            { subsystem::PLATFORM },
            // Terminations
            {},
            // Controls
            { subsystem::INTERNET }
        );

        NetworkManager::NetworkManager()
            : _connectionId(0),
              _service(nullptr),
              _NetworkManager(nullptr),
              _notification(this)
        {
            // Don't do any work in the constructor - all set up should be done in Initialize
        }

        NetworkManager::~NetworkManager()
        {
            // Don't do any work in the constructor - all tear down should be done in Deinitialize
        }

        /**
         * Initialise the plugin and register ourselves
         *
         * This should aim to be as fast as possible
         *
         * Even if we're running in Out Of Process mode, this will still run in the
         * main WPEFramework process - the new process is actually spawned from this method
         */
        const string NetworkManager::Initialize(PluginHost::IShell *service)
        {
            // Make sure everything is null as we expect
            ASSERT(_service == nullptr);
            ASSERT(_NetworkManager == nullptr);

            // Syslog Startup messages are always printed by default
            SYSLOG(Logging::Startup, (_T("Initializing NetworkManager")));
            SYSLOG(Logging::Startup, (_T("Initialize running in process %d"), Core::ProcessInfo().Id()));
            NetworkManagerLogger::Init();
            // Register the Connection::Notification first. Do this before we start our actual plugin
            // in case something goes wrong or is disconnected - this way we know we're at least registered
            // for activate/deactivate events
            _service = service;
            _service->Register(&_notification);

            // Register ourselves in the PluginHost so other plugins know where to find us
            // If we are running out of process (as per our config file), this is what will actually spawn the WPEProcess process
            // which will run our plugin instance
            //
            // Ideally for large, complex plugins we would actually split the plugin into two libraries - a thin library that just calls
            // _service->Root to launch WPEProcess, and a larger library that is only ever run inside WPEProcess only (we do this for Cobalt and WebKitBrowser)
            _NetworkManager = service->Root<Exchange::INetworkManager>(_connectionId, 5000, _T("NetworkManagerImplementation"));

            // Still running inside the main WPEFramework process - the child process will have now been spawned and registered if necessary
            if (_NetworkManager != nullptr)
            {
                // set the plugin configuration
                Exchange::INetworkManager::NMLogging _loglevel;
                _NetworkManager->Configure(_service->ConfigLine(), _loglevel);
                // configure loglevel in libWPEFrameworkNetworkManager.so
                NetworkManagerLogger::SetLevel(static_cast <NetworkManagerLogger::LogLevel>(_loglevel));
                //Exchange::JNetworkManager::Register(*this, _NetworkManager);
                _NetworkManager->Register(&_notification);

                // Register all custom JSON-RPC methods
                RegisterAllMethods();
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                RegisterLegacyMethods();
#endif
            }
            else
            {
                // Something went wrong, clean up
                TRACE(Trace::Error, (_T("Failed to initialize NetworkManager")));
                _service->Unregister(&_notification);
                _service = nullptr;

                // Returning a string signals that we failed to initialize - WPEFramework will print this as an error message
                return "Failed to initialize NetworkManager";
            }

            // Success
            return "";
        }

        /**
         * Clean up the plugin when we're deactivated. Should release any resources we were holding
         *
         * Note again this code runs inside the main WPEFramework daemon even if the plugin is set to run out-of-process
         */
        void NetworkManager::Deinitialize(PluginHost::IShell *service)
        {
            ASSERT(_service == service);
            ASSERT(_NetworkManager != nullptr);

            TRACE(Trace::Information, (_T("Deinitializing NetworkManager")));
            TRACE(Trace::Information, (_T("Deinitialize running in process %d"), Core::ProcessInfo().Id()));

            if (_NetworkManager != nullptr)
            {
                // TODO:: Work out exactly what triggers the shutdown of the out-of-process host
                _service->Unregister(&_notification);
                _NetworkManager->Unregister(&_notification);

                //Exchange::JNetworkManager::Unregister(*this);
                // Unregister all our JSON-RPC methods
                UnregisterAllMethods();
#ifdef ENABLE_LEGACY_NSM_SUPPORT
                UnregisterLegacyMethods();
#endif
                _NetworkManager->Release();
            }

            // Set everything back to default
            _connectionId = 0;
            _service = nullptr;
            _NetworkManager = nullptr;
        }

        string NetworkManager::Information() const
        {
            // No additional info to report
            return string();
        }


        void NetworkManager::Deactivated(RPC::IRemoteConnection *connection)
        {
            // Gracefully handle an unexpected termination from the other side of the connection (for example if the remote process crashed)
            // and deactivate ourselves as we cannot continue safely
            if (connection->Id() == _connectionId)
            {
                ASSERT(_service != nullptr);
                Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
            }
        }
    }
}
