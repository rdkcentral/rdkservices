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

#pragma once

#include "Module.h"
#include <interfaces/json/JsonData_UserSettings.h>
#include <interfaces/json/JUserSettings.h>
#include <interfaces/json/JUserSettingsInspector.h>
#include <interfaces/json/JsonData_UserSettingsInspector.h>
#include <interfaces/IUserSettings.h>
#include <interfaces/IConfiguration.h>
#include "UtilsLogging.h"
#include "tracing/Logging.h"
#include <mutex>

namespace WPEFramework {
namespace Plugin {

    class UserSettings: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {
     private:
        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IUserSettings::INotification
        {
            private:
                Notification() = delete;
                Notification(const Notification&) = delete;
                Notification& operator=(const Notification&) = delete;

            public:
            explicit Notification(UserSettings* parent)
                : _parent(*parent)
                {
                    if (parent == nullptr)
                    {
                       LOGERR("parent is null");
                    }
                }

                virtual ~Notification()
                {
                }

                BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IUserSettings::INotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                END_INTERFACE_MAP

                void Activated(RPC::IRemoteConnection*) override
                {
                }

                void Deactivated(RPC::IRemoteConnection *connection) override
                {
                   _parent.Deactivated(connection);
                }

                void OnAudioDescriptionChanged(const bool enabled) override
                {
                    LOGINFO("AudioDescriptionChanged: %d\n", enabled);
                    Exchange::JUserSettings::Event::OnAudioDescriptionChanged(_parent, enabled);
                }

                void OnPreferredAudioLanguagesChanged(const string& preferredLanguages) override
                {
                    LOGINFO("PreferredAudioLanguagesChanged: %s\n", preferredLanguages.c_str());
                    Exchange::JUserSettings::Event::OnPreferredAudioLanguagesChanged(_parent, preferredLanguages);
                }

                void OnPresentationLanguageChanged(const string& presentationLanguage) override
                {
                    LOGINFO("PresentationLanguageChanged: %s\n", presentationLanguage.c_str());
                    Exchange::JUserSettings::Event::OnPresentationLanguageChanged(_parent, presentationLanguage);
                }

                void OnCaptionsChanged(bool enabled) override
                {
                    LOGINFO("CaptionsChanged: %d\n", enabled);
                    Exchange::JUserSettings::Event::OnCaptionsChanged(_parent, enabled);
                }

                void OnPreferredCaptionsLanguagesChanged(const string& preferredLanguages) override
                {
                    LOGINFO("PreferredCaptionsLanguagesChanged: %s\n", preferredLanguages.c_str());
                    Exchange::JUserSettings::Event::OnPreferredCaptionsLanguagesChanged(_parent, preferredLanguages);
                }

                void OnPreferredClosedCaptionServiceChanged(const string& service) override
                {
                    LOGINFO("PreferredClosedCaptionServiceChanged: %s\n", service.c_str());
                    Exchange::JUserSettings::Event::OnPreferredClosedCaptionServiceChanged(_parent, service);
                }

                void OnPrivacyModeChanged(const string &privacyMode) override
                {
                    LOGINFO("PrivacyModeChanged: %s\n", privacyMode.c_str());
                    Exchange::JUserSettings::Event::OnPrivacyModeChanged(_parent, privacyMode);
                }

                void OnPinControlChanged(const bool pinControl) override
                {
                    LOGINFO("PinControlChanged: %d\n", pinControl);
                    Exchange::JUserSettings::Event::OnPinControlChanged(_parent, pinControl);
                }

                void OnViewingRestrictionsChanged(const string& viewingRestrictions) override
                {
                    LOGINFO("ViewingRestrictionsChanged: %s\n", viewingRestrictions.c_str());
                    Exchange::JUserSettings::Event::OnViewingRestrictionsChanged(_parent, viewingRestrictions);
                }

                void OnViewingRestrictionsWindowChanged(const string& viewingRestrictionsWindow) override
                {
                    LOGINFO("ViewingRestrictionsWindowChanged: %s\n", viewingRestrictionsWindow.c_str());
                    Exchange::JUserSettings::Event::OnViewingRestrictionsWindowChanged(_parent, viewingRestrictionsWindow);
                }

                void OnLiveWatershedChanged(const bool liveWatershed) override
                {
                    LOGINFO("LiveWatershedChanged: %d\n", liveWatershed);
                    Exchange::JUserSettings::Event::OnLiveWatershedChanged(_parent, liveWatershed);
                }

                void OnPlaybackWatershedChanged(const bool playbackWatershed) override
                {
                    LOGINFO("PlaybackWatershedChanged: %d\n", playbackWatershed);
                    Exchange::JUserSettings::Event::OnPlaybackWatershedChanged(_parent, playbackWatershed);
                }

                void OnBlockNotRatedContentChanged(const bool blockNotRatedContent) override
                {
                    LOGINFO("BlockNotRatedContentChanged: %d\n", blockNotRatedContent);
                    Exchange::JUserSettings::Event::OnBlockNotRatedContentChanged(_parent, blockNotRatedContent);
                }

                void OnPinOnPurchaseChanged(const bool pinOnPurchase) override
                {
                    LOGINFO("PinOnPurchaseChanged: %d\n", pinOnPurchase);
                    Exchange::JUserSettings::Event::OnPinOnPurchaseChanged(_parent, pinOnPurchase);
                }

                void OnHighContrastChanged(const bool enabled) override
                {
                    LOGINFO("HighContrastChanged: %d\n", enabled);
                    Exchange::JUserSettings::Event::OnHighContrastChanged(_parent, enabled);
                }

                void OnVoiceGuidanceChanged(const bool enabled) override
                {
                    LOGINFO("VoiceGuidanceChanged: %d\n", enabled);
                    Exchange::JUserSettings::Event::OnVoiceGuidanceChanged(_parent, enabled);
                }

                void OnVoiceGuidanceRateChanged(const double rate) override
                {
                    LOGINFO("GuidanceRateChanged: %lf\n", rate);
                    Exchange::JUserSettings::Event::OnVoiceGuidanceRateChanged(_parent, rate);
                }

                void OnVoiceGuidanceHintsChanged(const bool hints) override
                {
                    LOGINFO("GuidanceHintsChanged: %d\n", hints);
                    Exchange::JUserSettings::Event::OnVoiceGuidanceHintsChanged(_parent, hints);
                }

            private:
                UserSettings& _parent;
        };

        public:
            // We do not allow this plugin to be copied !!
            UserSettings(const UserSettings&) = delete;
            UserSettings& operator=(const UserSettings&) = delete;

            UserSettings();
            virtual ~UserSettings();

            BEGIN_INTERFACE_MAP(UserSettings)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IUserSettings, _userSetting)
            INTERFACE_AGGREGATE(Exchange::IUserSettingsInspector, _userSettingsInspector)
            END_INTERFACE_MAP

            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell* service) override;
            string Information() const override;

        private:
            void Deactivated(RPC::IRemoteConnection* connection);

        private:
            PluginHost::IShell* _service{};
            uint32_t _connectionId{};
            Exchange::IUserSettings* _userSetting{};
            Exchange::IUserSettingsInspector* _userSettingsInspector{};
            Core::Sink<Notification> _usersettingsNotification;
            Exchange::IConfiguration* configure;
    };

} // namespace Plugin
} // namespace WPEFramework
