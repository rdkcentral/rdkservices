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
#include "PrivacyImplementation.h"
#include "UtilsLogging.h"

#include <fstream>
#include <streambuf>
#include <sys/sysinfo.h>

namespace WPEFramework {
namespace Plugin {


    class PrivacyConfig : public Core::JSON::Container {
        private:
            PrivacyConfig(const PrivacyConfig&) = delete;
            PrivacyConfig& operator=(const PrivacyConfig&) = delete;
          
        public:
            PrivacyConfig()
                : Core::JSON::Container()
                , EssUrl()
            {
                Add(_T("essurl"), &EssUrl);
            }
            ~PrivacyConfig()
            {
            }

        public:
            Core::JSON::String EssUrl;
        };

    SERVICE_REGISTRATION(PrivacyImplementation, 1, 0);

    PrivacyImplementation::PrivacyImplementation():
        mQueueMutex(),
        mQueueCondition(),
        mActionQueue(),
        mShell(nullptr),
        mNotificationMutex(),
        mNotifications()

    {
        mThread = std::thread(&PrivacyImplementation::ActionLoop, this);
    }

    PrivacyImplementation::~PrivacyImplementation()
    {
        LOGINFO("PrivacyImplementation::~PrivacyImplementation()");
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mActionQueue.push({ACTION_TYPE_SHUTDOWN});
        lock.unlock();
        mQueueCondition.notify_one();
        mThread.join();
        mShell->Release();
    }


    uint32_t PrivacyImplementation::Configure(PluginHost::IShell* shell)
    {
        LOGINFO("Configuring Privacy");
        uint32_t result = Core::ERROR_NONE;
        ASSERT(shell != nullptr);
        mShell = shell;
        mShell->AddRef();

        return result;
    }

    Core::hresult PrivacyImplementation::Register(Exchange::IPrivacy::INotification *notification)
    {
        ASSERT (nullptr != notification);
        mNotificationMutex.Lock();
        // Make sure we can't register the same notification callback multiple times
        if (std::find(mNotifications.begin(), mNotifications.end(), notification) == mNotifications.end())
        {
            LOGINFO("Register notification");
            mNotifications.push_back(notification);
            notification->AddRef();
        }
        mNotificationMutex.Unlock();
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::Unregister(Exchange::IPrivacy::INotification *notification)
    {
        Core::hresult status = Core::ERROR_GENERAL;
        ASSERT (nullptr != notification);
        mNotificationMutex.Lock();
        // Make sure we can't unregister the same notification callback multiple times
        auto itr = std::find(mNotifications.begin(), mNotifications.end(), notification);
        if (itr != mNotifications.end())
        {
            (*itr)->Release();
            LOGINFO("Unregister notification");
            mNotifications.erase(itr);
            status = Core::ERROR_NONE;
        }
        else
        {
            LOGERR("notification not found");
        }
        mNotificationMutex.Unlock();
        return status;
    }

    Core::hresult PrivacyImplementation::SetAllowACRCollection(const bool allow)
    {
        LOGINFO("SetAllowACRCollection: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowACRCollection(bool &allow) const
    {
        LOGINFO("GetAllowACRCollection");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowAppContentAdTargeting(const bool allow)
    {
        LOGINFO("SetAllowAppContentAdTargeting: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowAppContentAdTargeting(bool &allow) const
    {
        LOGINFO("GetAllowAppContentAdTargeting");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowCameraAnalytics(const bool allow)
    {
        LOGINFO("SetAllowCameraAnalytics: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowCameraAnalytics(bool &allow) const
    {
        LOGINFO("GetAllowCameraAnalytics");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowPersonalization(const bool allow)
    {
        LOGINFO("SetAllowPersonalization: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowPersonalization(bool &allow) const
    {
        LOGINFO("GetAllowPersonalization");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowPrimaryBrowseAdTargeting(const bool allow)
    {
        LOGINFO("SetAllowPrimaryBrowseAdTargeting: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowPrimaryBrowseAdTargeting(bool &allow) const
    {
        LOGINFO("GetAllowPrimaryBrowseAdTargeting");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowPrimaryContentAdTargeting(const bool allow)
    {
        LOGINFO("SetAllowPrimaryContentAdTargeting: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowPrimaryContentAdTargeting(bool &allow) const
    {
        LOGINFO("GetAllowPrimaryContentAdTargeting");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowProductAnalytics(const bool allow)
    {
        LOGINFO("SetAllowProductAnalytics: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowProductAnalytics(bool &allow) const
    {
        LOGINFO("GetAllowProductAnalytics");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowRemoteDiagnostics(const bool allow)
    {
        LOGINFO("SetAllowRemoteDiagnostics: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowRemoteDiagnostics(bool &allow) const
    {
        LOGINFO("GetAllowRemoteDiagnostics");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowResumePoints(const bool allow)
    {
        LOGINFO("SetAllowResumePoints: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowResumePoints(bool &allow) const
    {
        LOGINFO("GetAllowResumePoints");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowUnentitledPersonalization(const bool allow)
    {
        LOGINFO("SetAllowUnentitledPersonalization: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowUnentitledPersonalization(bool &allow) const
    {
        LOGINFO("GetAllowUnentitledPersonalization");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowUnentitledResumePoints(const bool allow)
    {
        LOGINFO("SetAllowUnentitledResumePoints: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowUnentitledResumePoints(bool &allow) const
    {
        LOGINFO("GetAllowUnentitledResumePoints");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowWatchHistory(const bool allow)
    {
        LOGINFO("SetAllowWatchHistory: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowWatchHistory(bool &allow) const
    {
        LOGINFO("GetAllowWatchHistory");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowContinueWatchingOptOut(const bool allow)
    {
        LOGINFO("SetAllowContinueWatchingOptOut: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowContinueWatchingOptOut(bool &allow) const
    {
        LOGINFO("GetAllowContinueWatchingOptOut");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowPersonalisedRecsOptOut(const bool allow)
    {
        LOGINFO("SetAllowPersonalisedRecsOptOut: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowPersonalisedRecsOptOut(bool &allow) const
    {
        LOGINFO("GetAllowPersonalisedRecsOptOut");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::SetAllowProductAnalyticsOptOut(const bool allow)
    {
        LOGINFO("SetAllowProductAnalyticsOptOut: %d", allow);
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetAllowProductAnalyticsOptOut(bool &allow) const
    {
        LOGINFO("GetAllowProductAnalyticsOptOut");
        allow = false;
        return Core::ERROR_NONE;
    }

    Core::hresult PrivacyImplementation::GetSettigs(bool &allowACRCollection,
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
            bool &allowProductAnalyticsOptOut) const
    {
        LOGINFO("GetSettigs");
        allowACRCollection = false;
        allowAppContentAdTargeting = false;
        allowCameraAnalytics = false;
        allowPersonalization = false;
        allowPrimaryBrowseAdTargeting = false;
        allowPrimaryContentAdTargeting = false;
        allowProductAnalytics = false;
        allowRemoteDiagnostics = false;
        allowResumePoints = false;
        allowUnentitledPersonalization = false;
        allowUnentitledResumePoints = false;
        allowWatchHistory = false;
        allowContinueWatchingOptOut = false;
        allowPersonalisedRecsOptOut = false;
        allowProductAnalyticsOptOut = false;
        return Core::ERROR_NONE;
    }

    void PrivacyImplementation::ActionLoop()
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);

        while (true) {

            std::chrono::milliseconds queueTimeout(std::chrono::milliseconds::max());

            if (mActionQueue.empty())
            {
                if (queueTimeout == std::chrono::milliseconds::max())
                {
                    mQueueCondition.wait(lock, [this]
                                         { return !mActionQueue.empty(); });
                }
                else
                {
                    mQueueCondition.wait_for(lock, queueTimeout, [this]
                                             { return !mActionQueue.empty(); });
                }
            }

            Action action = {ACTION_TYPE_UNDEF};

            if (!mActionQueue.empty())
            {
                action = mActionQueue.front();
                mActionQueue.pop();
            }

            lock.unlock();

            switch (action.type) {
  
                case ACTION_TYPE_SHUTDOWN:
                    LOGINFO("Shutting down Privacy");
                    return;
                default:
                    break;
            }

            lock.lock();
        }
    }


    
}
}