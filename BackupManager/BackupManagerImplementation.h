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

#pragma once

#include "Module.h"

#include <interfaces/IBackup.h>
#include <interfaces/IConfiguration.h>

#include <com/com.h>
#include <core/core.h>

#include <unordered_map>

#include "UtilsLogging.h"

namespace WPEFramework {
namespace Plugin {
    class BackupManagerImplementation : public Exchange::IBackupManager, public Exchange::IConfiguration
    {
        private:

        class MonitorObjects : public PluginHost::IPlugin::INotification
        {
        public:
            explicit MonitorObjects(BackupManagerImplementation *parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }

         void Activated (const string& callsign, PluginHost::IShell* service) override
            {
                LOGINFO("Plugin Activated: %s", callsign.c_str());
                _parent.PluginActivated(callsign, service);
            }

            void Deactivated (const string& callsign, PluginHost::IShell* service) override
            {
                LOGINFO("Plugin Deactivated: %s", callsign.c_str());
                _parent.PluginDeactivated(callsign, service);
            }

            void Unavailable(const string&, PluginHost::IShell*) override
            {
            }

            BEGIN_INTERFACE_MAP(MonitorObjects)
            INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
            END_INTERFACE_MAP
               
        private:
            BackupManagerImplementation& _parent;

        };

    public:
        // We do not allow this plugin to be copied !!
        BackupManagerImplementation();
        ~BackupManagerImplementation() override;

        // We do not allow this plugin to be copied !!
        BackupManagerImplementation(const BackupManagerImplementation&) = delete;
        BackupManagerImplementation& operator=(const BackupManagerImplementation&) = delete;

        BEGIN_INTERFACE_MAP(BackupManagerImplementation)
        INTERFACE_ENTRY(Exchange::IBackupManager)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    public:
        uint32_t Configure(PluginHost::IShell* service) override;

        Core::hresult BackupSettings(const Exchange::BackupContext& context) override;
        Core::hresult RestoreSettings(const Exchange::BackupContext& context) override;
        Core::hresult DeleteBackup(const Exchange::BackupContext& context) override;

        void PluginActivated(const string& callsign, PluginHost::IShell* service);
        void PluginDeactivated(const string& callsign, PluginHost::IShell* service);
        
    private:
        void MakeContext(const Exchange::BackupContext &contextIn, Exchange::BackupContext &contextOut) const;
        
        mutable Core::CriticalSection _adminLock;
        PluginHost::IShell* _service;
        Core::Sink<MonitorObjects> _monitor;
        using BackupProviderContainer = std::unordered_map<string, Exchange::IBackupProvider*>;
        BackupProviderContainer _backupProviders;        
    };
} // namespace Plugin
} // namespace WPEFramework
