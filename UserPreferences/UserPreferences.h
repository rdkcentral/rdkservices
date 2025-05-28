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
#include <interfaces/IUserSettings.h>
#include <mutex>

namespace WPEFramework {
    namespace Plugin {

        class UserPreferences : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            UserPreferences(const UserPreferences&) = delete;
            UserPreferences& operator=(const UserPreferences&) = delete;

            class Notification : public Exchange::IUserSettings::INotification {
                public:
                    explicit Notification(UserPreferences* parent) : _parent(parent) {}
                    ~Notification() override = default;
    
                    void OnPresentationLanguageChanged(const string& language) override;
                    void OnAudioDescriptionChanged(const bool enabled) override;
                    void OnPreferredAudioLanguagesChanged(const string& preferredLanguages) override;
                    void OnCaptionsChanged(const bool enabled) override;
                    void OnPreferredCaptionsLanguagesChanged(const string& preferredLanguages) override;
                    void OnPreferredClosedCaptionServiceChanged(const string& service) override;
                    void OnPinControlChanged(const bool pinControl) override;
                    void OnViewingRestrictionsChanged(const string& viewingRestrictions) override;
                    void OnViewingRestrictionsWindowChanged(const string& viewingRestrictionsWindow) override;
                    void OnLiveWatershedChanged(const bool liveWatershed) override;
                    void OnPlaybackWatershedChanged(const bool playbackWatershed) override;
                    void OnBlockNotRatedContentChanged(const bool blockNotRatedContent) override;
                    void OnPinOnPurchaseChanged(const bool pinOnPurchase) override;
                    void OnHighContrastChanged(const bool enabled) override;
                    void OnVoiceGuidanceChanged(const bool enabled) override;
                    void OnVoiceGuidanceRateChanged(const double rate) override;
                    void OnVoiceGuidanceHintsChanged(const bool hints) override;
                    void AddRef() const override;
                    uint32_t Release() const override;
                    
    
                private:
                    UserPreferences* _parent;
    
                    BEGIN_INTERFACE_MAP(Notification)
                    INTERFACE_ENTRY(Exchange::IUserSettings::INotification)
                    END_INTERFACE_MAP
            };


            //Begin methods
            uint32_t getUILanguage(const JsonObject& parameters, JsonObject& response);
            uint32_t setUILanguage(const JsonObject& parameters, JsonObject& response);

            private:
            bool ConvertToUserSettingsFormat(const string& uiLanguage, string& presentationLanguage);
            bool ConvertToUserPrefsFormat(const string& presentationLanguage, string& uiLanguage);
            bool PerformMigration(Exchange::IUserSettings& userSettings);
            //End methods

            //Begin events
            //End events

        public:
            UserPreferences();
            virtual ~UserPreferences();
            virtual const string Initialize(PluginHost::IShell* shell) override ;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override;

            BEGIN_INTERFACE_MAP(UserPreferences)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            void OnPresentationLanguageChanged(const string& language);
            PluginHost::IShell* _service;
            Core::Sink<Notification> _notification;
            bool _isMigrationDone;
            string _lastUILanguage;
            mutable Core::CriticalSection _adminLock;
    
        public:
            static UserPreferences* _instance;

        };
    } // namespace Plugin
} // namespace WPEFramework
