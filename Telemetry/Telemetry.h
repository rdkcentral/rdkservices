/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#pragma once

#include "Module.h"

#ifdef HAS_RBUS
#include <interfaces/IUserSettings.h>
#endif

namespace WPEFramework {

    namespace Plugin {


        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.
        class Telemetry : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

#ifdef HAS_RBUS
            class UserSettingsNotification : public Exchange::IUserSettings::INotification {
            private:
                UserSettingsNotification(const UserSettingsNotification&) = delete;
                UserSettingsNotification& operator=(const UserSettingsNotification&) = delete;

            public:
                explicit UserSettingsNotification(Telemetry& parent)
                    : _parent(parent)
                {
                }

                ~UserSettingsNotification() override = default;

                void OnPrivacyModeChanged(const string& privacyMode)
                {
                    _parent.notifyT2PrivacyMode(privacyMode);
                }

                BEGIN_INTERFACE_MAP(UserSettingsNotification)
                INTERFACE_ENTRY(Exchange::IUserSettings::INotification)
                END_INTERFACE_MAP

            private:
                Telemetry& _parent;
            };
#endif
            // We do not allow this plugin to be copied !!
            Telemetry(const Telemetry&) = delete;
            Telemetry& operator=(const Telemetry&) = delete;

            //Begin methods
            uint32_t setReportProfileStatus(const JsonObject& parameters, JsonObject& response);
            uint32_t logApplicationEvent(const JsonObject& parameters, JsonObject& response);
            uint32_t uploadReport(const JsonObject& parameters, JsonObject& response);
            uint32_t abortReport(const JsonObject& parameters, JsonObject& response);
            //End methods

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            void InitializeIARM();
            void DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

        public:
            Telemetry();
            virtual ~Telemetry();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            uint32_t UploadReport();
            uint32_t AbortReport();

            BEGIN_INTERFACE_MAP(Telemetry)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

#ifdef HAS_RBUS
            void onReportUploadStatus(const char* status);
            void notifyT2PrivacyMode(std::string privacyMode);
#endif
        public:
            static Telemetry* _instance;
        private:
            std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> m_systemServiceConnection;
#ifdef HAS_RBUS
            Exchange::IUserSettings* _userSettingsPlugin;
            Core::Sink<UserSettingsNotification> _userSettingsNotification;
#endif
        };
    } // namespace Plugin
} // namespace WPEFramework
