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
#include "tracing/Logging.h"
#include <vector>

#include <com/com.h>
#include <core/core.h>
#include <plugins/plugins.h>

#ifdef HAS_RBUS
#include "rbus.h"
#endif

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

namespace WPEFramework {
namespace Plugin {
    class UserSettingsImplementation : public Exchange::IUserSettings{

    public:
        static const std::map<string, string> usersettingsDefaultMap;

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
        END_INTERFACE_MAP

    public:
        enum Event {
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
                PIN_ON_PURCHASE_CHANGED
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
        virtual uint32_t Register(Exchange::IUserSettings::INotification *notification ) override ;
        virtual uint32_t Unregister(Exchange::IUserSettings::INotification *notification ) override ;
        uint32_t setAudioDescription(const bool enabled) override;
        uint32_t getAudioDescription(bool &enabled) const override;
        uint32_t setPreferredAudioLanguages(const string& preferredLanguages) override;
        uint32_t getPreferredAudioLanguages(string &preferredLanguages) const override;
        uint32_t setPresentationLanguage(const string& presentationLanguage) override;
        uint32_t getPresentationLanguage(string &presentationLanguage) const override;
        uint32_t setCaptions(const bool enabled) override;
        uint32_t getCaptions(bool &enabled) const override;
        uint32_t setPreferredCaptionsLanguages(const string& preferredLanguages) override;
        uint32_t getPreferredCaptionsLanguages(string &preferredLanguages) const override;
        uint32_t setPreferredClosedCaptionService(const string& service) override;
        uint32_t getPreferredClosedCaptionService(string &service) const override;
        uint32_t setPrivacyMode(const string& privacyMode) override;
        uint32_t getPrivacyMode(string &privacyMode) const override;
        uint32_t setPinControl(const bool enabled) override;
        uint32_t getPinControl(bool &enabled) const override;
        uint32_t setViewingRestrictions(const string& viewingRestrictions) override;
        uint32_t getViewingRestrictions(string &viewingRestrictions) const override;
        uint32_t setViewingRestrictionsWindow(const string& viewingRestrictionsWindow) override;
        uint32_t getViewingRestrictionsWindow(string &viewingRestrictionsWindow) const override;
        uint32_t setLiveWatershed(const bool enabled) override;
        uint32_t getLiveWatershed(bool &enabled) const override;
        uint32_t setPlaybackWatershed(const bool enabled) override;
        uint32_t getPlaybackWatershed(bool &enabled) const override;
        uint32_t setBlockNotRatedContent(const bool enabled) override;
        uint32_t getBlockNotRatedContent(bool &enabled) const override;
        uint32_t setPinOnPurchase(const bool enabled) override;
        uint32_t getPinOnPurchase(bool &enabled) const override;

        void registerEventHandlers();
        void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value);

    private:
        uint32_t SetUserSettingsValue(const string& key, const string& value);
        uint32_t GetUserSettingsValue(const string& key, string &value) const;

    private:
        mutable Core::CriticalSection _adminLock;
        Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> _engine;
        Core::ProxyType<RPC::CommunicatorClient> _communicatorClient;
        PluginHost::IShell *_controller;
        Exchange::IStore2* _remotStoreObject;
        std::list<Exchange::IUserSettings::INotification*> _userSettingNotification;
        Core::Sink<Store2Notification> _storeNotification;
        bool _registeredEventHandlers;

#ifdef HAS_RBUS
        rbusError_t _rbusHandleStatus;
        rbusHandle_t _rbusHandle;
#endif
        void dispatchEvent(Event, const JsonValue &params);
        void Dispatch(Event event, const JsonValue params);

        friend class Job;
    };
} // namespace Plugin
} // namespace WPEFramework
