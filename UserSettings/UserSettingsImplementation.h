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
#include <interfaces/Ids.h>
#include <interfaces/IUserSettings.h>
#include <interfaces/IStore2.h>
#include <interfaces/IConfiguration.h>
#include "tracing/Logging.h"
#include <vector>

#include <com/com.h>
#include <core/core.h>
#include <plugins/plugins.h>

#define USERSETTINGS_NAMESPACE "UserSettings"

#define USERSETTINGS_AUDIO_DESCRIPTION_KEY                    "audioDescription"
#define USERSETTINGS_PREFERRED_AUDIO_LANGUAGES_KEY            "preferredAudioLanguages"
#define USERSETTINGS_PRESENTATION_LANGUAGE_KEY                "presentationLanguage"
#define USERSETTINGS_CAPTIONS_KEY                             "captions"
#define USERSETTINGS_PREFERRED_CAPTIONS_LANGUAGES_KEY         "preferredCaptionsLanguages"
#define USERSETTINGS_PREFERRED_CLOSED_CAPTIONS_SERVICE_KEY    "preferredClosedCaptionsService"
#define USERSETTINGS_PRIVACY_MODE_KEY                         "privacyMode"
#define USERSETTINGS_PIN_CONTROL_KEY                          "pinControl"
#define USERSETTINGS_VIEWING_RESTRICTIONS_KEY                 "viewingRestrictions"
#define USERSETTINGS_VIEWING_RESTRICTIONS_WINDOW_KEY          "viewingRestrictionsWindow"
#define USERSETTINGS_LIVE_WATERSHED_KEY                       "liveWaterShed"
#define USERSETTINGS_PLAYBACK_WATERSHED_KEY                   "playbackWaterShed"
#define USERSETTINGS_BLOCK_NOT_RATED_CONTENT_KEY              "blockNotRatedContent"
#define USERSETTINGS_PIN_ON_PURCHASE_KEY                      "pinOnPurchase"
#define USERSETTINGS_HIGH_CONTRAST_KEY                        "highContrast"
#define USERSETTINGS_VOICE_GUIDANCE_KEY                       "voiceGuidance"
#define USERSETTINGS_VOICE_GUIDANCE_RATE_KEY                  "voiceGuidanceRate"
#define USERSETTINGS_VOICE_GUIDANCE_HINTS_KEY                 "voiceGuidanceHints"
#define USERSETTINGS_CONTENT_PIN_KEY                          "contentPin"

namespace WPEFramework {
namespace Plugin {
    class UserSettingsImplementation : public Exchange::IUserSettings,
                                       public Exchange::IUserSettingsInspector,
                                       public Exchange::IConfiguration {

    public:
        static const std::map<string, string> usersettingsDefaultMap;
        static const std::map<SettingsKey, string> _userSettingsInspectorMap;
        static const double minVGR;
        static const double maxVGR;

    private:
        class Store2Notification : public Exchange::IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(UserSettingsImplementation& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                _parent.ValueChanged(scope, ns, key, value);
            }

            BEGIN_INTERFACE_MAP(Store2Notification)
            INTERFACE_ENTRY(Exchange::IStore2::INotification)
            END_INTERFACE_MAP

        private:
            UserSettingsImplementation& _parent;
        };

    public:
        // We do not allow this plugin to be copied !!
        UserSettingsImplementation();
        ~UserSettingsImplementation() override;

        static UserSettingsImplementation* instance(UserSettingsImplementation *UserSettingsImpl = nullptr);

        // We do not allow this plugin to be copied !!
        UserSettingsImplementation(const UserSettingsImplementation&) = delete;
        UserSettingsImplementation& operator=(const UserSettingsImplementation&) = delete;

        BEGIN_INTERFACE_MAP(UserSettingsImplementation)
        INTERFACE_ENTRY(Exchange::IUserSettings)
        INTERFACE_ENTRY(Exchange::IUserSettingsInspector)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    public:
        enum Event
        {
                AUDIO_DESCRIPTION_CHANGED,
                PREFERRED_AUDIO_CHANGED,
                PRESENTATION_LANGUAGE_CHANGED,
                CAPTIONS_CHANGED,
                PREFERRED_CAPTIONS_LANGUAGE_CHANGED,
                PREFERRED_CLOSED_CAPTIONS_SERVICE_CHANGED,
                PRIVACY_MODE_CHANGED,
                PIN_CONTROL_CHANGED,
                VIEWING_RESTRICTIONS_CHANGED,
                VIEWING_RESTRICTIONS_WINDOW_CHANGED,
                LIVE_WATERSHED_CHANGED,
                PLAYBACK_WATERSHED_CHANGED,
                BLOCK_NOT_RATED_CONTENT_CHANGED,
                PIN_ON_PURCHASE_CHANGED,
                HIGH_CONTRAST_CHANGED,
                VOICE_GUIDANCE_CHANGED,
                VOICE_GUIDANCE_RATE_CHANGED,
                VOICE_GUIDANCE_HINTS_CHANGED,
                CONTENT_PIN_CHANGED
            };

        class EXTERNAL Job : public Core::IDispatch {
        protected:
             Job(UserSettingsImplementation *usersettingImplementation, Event event, JsonValue &params)
                : _userSettingImplementation(usersettingImplementation)
                , _event(event)
                , _params(params) {
                if (_userSettingImplementation != nullptr) {
                    _userSettingImplementation->AddRef();
                }
            }

       public:
            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job() {
                if (_userSettingImplementation != nullptr) {
                    _userSettingImplementation->Release();
                }
            }

       public:
            static Core::ProxyType<Core::IDispatch> Create(UserSettingsImplementation *usersettingImplementation, Event event, JsonValue params) {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(usersettingImplementation, event, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(usersettingImplementation, event, params)));
#endif
            }

            virtual void Dispatch() {
                _userSettingImplementation->Dispatch(_event, _params);
            }
        private:
            UserSettingsImplementation *_userSettingImplementation;
            const Event _event;
            const JsonValue _params;
        };

    public:
        virtual Core::hresult Register(Exchange::IUserSettings::INotification *notification ) override ;
        virtual Core::hresult Unregister(Exchange::IUserSettings::INotification *notification ) override ;
        Core::hresult SetAudioDescription(const bool enabled) override;
        Core::hresult GetAudioDescription(bool &enabled) const override;
        Core::hresult SetPreferredAudioLanguages(const string& preferredLanguages) override;
        Core::hresult GetPreferredAudioLanguages(string &preferredLanguages) const override;
        Core::hresult SetPresentationLanguage(const string& presentationLanguage) override;
        Core::hresult GetPresentationLanguage(string &presentationLanguage) const override;
        Core::hresult SetCaptions(const bool enabled) override;
        Core::hresult GetCaptions(bool &enabled) const override;
        Core::hresult SetPreferredCaptionsLanguages(const string& preferredLanguages) override;
        Core::hresult GetPreferredCaptionsLanguages(string &preferredLanguages) const override;
        Core::hresult SetPreferredClosedCaptionService(const string& service) override;
        Core::hresult GetPreferredClosedCaptionService(string &service) const override;
        Core::hresult SetPrivacyMode(const string& privacyMode) override;
        Core::hresult GetPrivacyMode(string &privacyMode) const override;
        Core::hresult SetPinControl(const bool pinControl) override;
        Core::hresult GetPinControl(bool &pinControl) const override;
        Core::hresult SetViewingRestrictions(const string& viewingRestrictions) override;
        Core::hresult GetViewingRestrictions(string &viewingRestrictions) const override;
        Core::hresult SetViewingRestrictionsWindow(const string& viewingRestrictionsWindow) override;
        Core::hresult GetViewingRestrictionsWindow(string &viewingRestrictionsWindow) const override;
        Core::hresult SetLiveWatershed(const bool liveWatershed) override;
        Core::hresult GetLiveWatershed(bool &liveWatershed) const override;
        Core::hresult SetPlaybackWatershed(const bool playbackWatershed) override;
        Core::hresult GetPlaybackWatershed(bool &playbackWatershed) const override;
        Core::hresult SetBlockNotRatedContent(const bool blockNotRatedContent) override;
        Core::hresult GetBlockNotRatedContent(bool &blockNotRatedContent) const override;
        Core::hresult SetPinOnPurchase(const bool pinOnPurchase) override;
        Core::hresult GetPinOnPurchase(bool &pinOnPurchase) const override;
        Core::hresult SetHighContrast(const bool enabled) override;
        Core::hresult GetHighContrast(bool &enabled) const override;
        Core::hresult SetVoiceGuidance(const bool enabled) override;
        Core::hresult GetVoiceGuidance(bool &enabled) const override;
        Core::hresult SetVoiceGuidanceRate(const double rate) override;
        Core::hresult GetVoiceGuidanceRate(double &rate) const override;
        Core::hresult SetVoiceGuidanceHints(const bool hints) override;
        Core::hresult GetVoiceGuidanceHints(bool &hints) const override;
        Core::hresult SetContentPin(const string& contentPin) override;
        Core::hresult GetContentPin(string& contentPin) const override;

        // IUserSettingsInspector methods
        Core::hresult GetMigrationState(const SettingsKey key, bool &requiresMigration) const override;
        Core::hresult GetMigrationStates(IUserSettingsMigrationStateIterator *&states) const override;

        // IConfiguration methods
        uint32_t Configure(PluginHost::IShell* service) override;

        void registerEventHandlers();
        void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value);

    private:
        uint32_t SetUserSettingsValue(const string& key, const string& value);
        uint32_t GetUserSettingsValue(const string& key, string &value) const;

    private:
        mutable Core::CriticalSection _adminLock;
        Exchange::IStore2* _remotStoreObject;
        std::list<Exchange::IUserSettings::INotification*> _userSettingNotification;
        Core::Sink<Store2Notification> _storeNotification;
        bool _registeredEventHandlers;
        PluginHost::IShell* _service;

        void dispatchEvent(Event, const JsonValue &params);
        void Dispatch(Event event, const JsonValue params);

        friend class Job;
    };
} // namespace Plugin
} // namespace WPEFramework
