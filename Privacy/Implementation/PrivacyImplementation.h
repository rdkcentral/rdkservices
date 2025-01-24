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
#pragma once

#include "../Module.h"
#include <interfaces/IPrivacy.h>
#include <interfaces/IConfiguration.h>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

namespace WPEFramework {
namespace Plugin {
    class PrivacyImplementation : public Exchange::IPrivacy, public Exchange::IConfiguration {
    private:
        PrivacyImplementation(const PrivacyImplementation&) = delete;
        PrivacyImplementation& operator=(const PrivacyImplementation&) = delete;

    public:
        PrivacyImplementation();
        ~PrivacyImplementation();

        BEGIN_INTERFACE_MAP(PrivacyImplementation)
        INTERFACE_ENTRY(Exchange::IPrivacy)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:

        enum ActionType
        {
            ACTION_TYPE_UNDEF,
            ACTION_TYPE_SHUTDOWN
        };

        struct Action
        {
            ActionType type;
        };


        // IPrivacyImplementation interface 
        Core::hresult Register(Exchange::IPrivacy::INotification *notification) override;
        Core::hresult Unregister(Exchange::IPrivacy::INotification *notification) override;

        Core::hresult SetAllowACRCollection(const bool allow) override;
        Core::hresult GetAllowACRCollection(bool &allow) const override;
        Core::hresult SetAllowAppContentAdTargeting(const bool allow) override;
        Core::hresult GetAllowAppContentAdTargeting(bool &allow) const override;
        Core::hresult SetAllowCameraAnalytics(const bool allow) override;
        Core::hresult GetAllowCameraAnalytics(bool &allow) const override;
        Core::hresult SetAllowPersonalization(const bool allow) override;
        Core::hresult GetAllowPersonalization(bool &allow) const override;
        Core::hresult SetAllowPrimaryBrowseAdTargeting(const bool allow) override;
        Core::hresult GetAllowPrimaryBrowseAdTargeting(bool &allow) const override;
        Core::hresult SetAllowPrimaryContentAdTargeting(const bool allow) override;
        Core::hresult GetAllowPrimaryContentAdTargeting(bool &allow) const override;
        Core::hresult SetAllowProductAnalytics(const bool allow) override;
        Core::hresult GetAllowProductAnalytics(bool &allow) const override;
        Core::hresult SetAllowRemoteDiagnostics(const bool allow) override;
        Core::hresult GetAllowRemoteDiagnostics(bool &allow) const override;
        Core::hresult SetAllowResumePoints(const bool allow) override;
        Core::hresult GetAllowResumePoints(bool &allow) const override;
        Core::hresult SetAllowUnentitledPersonalization(const bool allow) override;
        Core::hresult GetAllowUnentitledPersonalization(bool &allow) const override;
        Core::hresult SetAllowUnentitledResumePoints(const bool allow) override;
        Core::hresult GetAllowUnentitledResumePoints(bool &allow) const override;
        Core::hresult SetAllowWatchHistory(const bool allow) override;
        Core::hresult GetAllowWatchHistory(bool &allow) const override;
        Core::hresult SetAllowContinueWatchingOptOut(const bool allow) override;
        Core::hresult GetAllowContinueWatchingOptOut(bool &allow) const override;
        Core::hresult SetAllowPersonalisedRecsOptOut(const bool allow) override;
        Core::hresult GetAllowPersonalisedRecsOptOut(bool &allow) const override;
        Core::hresult SetAllowProductAnalyticsOptOut(const bool allow) override;
        Core::hresult GetAllowProductAnalyticsOptOut(bool &allow) const override;
        Core::hresult GetSettigs(bool &allowACRCollection,
                 bool &allowAppContentAdTargeting,
                 bool &allowCameraAnalytics,
                 bool &allowPersonalization,
                 bool &allowPrimaryBrowseAdTargeting,
                 bool &allowPrimaryContentAdTargeting,
                 bool &allowProductAnalytics,
                 bool &allowRemoteDiagnostics,
                 bool &allowResumePoints,
                 bool &allowUnentitledPersonalization,
                 bool &allowUnentitledResumePoints,
                 bool &allowWatchHistory,
                 bool &allowContinueWatchingOptOut,
                 bool &allowPersonalisedRecsOptOut,
                 bool &allowProductAnalyticsOptOut) const override;

        // IConfiguration interface
        uint32_t Configure(PluginHost::IShell* shell);

        void ActionLoop();

        std::mutex mQueueMutex;
        std::condition_variable mQueueCondition;
        std::thread mThread;
        std::queue<Action> mActionQueue;
        PluginHost::IShell* mShell;

        Core::CriticalSection mNotificationMutex;
        std::list<Exchange::IPrivacy::INotification*> mNotifications;
    };
}
}