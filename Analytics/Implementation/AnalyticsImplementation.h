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
#include <interfaces/IAnalytics.h>
#include <interfaces/IConfiguration.h>
#include "Backend/AnalyticsBackend.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

namespace WPEFramework {
namespace Plugin {
    class AnalyticsImplementation : public Exchange::IAnalytics, public Exchange::IConfiguration {
    private:
        AnalyticsImplementation(const AnalyticsImplementation&) = delete;
        AnalyticsImplementation& operator=(const AnalyticsImplementation&) = delete;

    public:
        AnalyticsImplementation();
        ~AnalyticsImplementation();

        BEGIN_INTERFACE_MAP(AnalyticsImplementation)
        INTERFACE_ENTRY(Exchange::IAnalytics)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:

        enum ActionType
        {
            ACTION_TYPE_UNDEF,
            ACTION_POPULATE_TIME_INFO,
            ACTION_TYPE_SEND_EVENT,
            ACTION_TYPE_SHUTDOWN,
            ACTION_TYPE_SET_TIME_READY
        };

        struct Event
        {
            std::string eventName;
            std::string eventVersion;
            std::string eventSource;
            std::string eventSourceVersion;
            std::list<std::string> cetList;
            uint64_t epochTimestamp;
            uint64_t uptimeTimestamp;
            std::string eventPayload;
        };

        struct Action
        {
            ActionType type;
            std::shared_ptr<Event> payload;
            std::string id;
        };


        // IAnalyticsImplementation interface
        uint32_t SendEvent(const string& eventName,
                                   const string& eventVersion,
                                   const string& eventSource,
                                   const string& eventSourceVersion,
                                   RPC::IStringIterator* const& cetList,
                                   const uint64_t& epochTimestamp,
                                   const uint64_t& uptimeTimestamp,
                                   const string& eventPayload) override;
        uint32_t SetSessionId(const string& id) override;
        uint32_t SetTimeReady() override;


        // IConfiguration interface
        uint32_t Configure(PluginHost::IShell* shell);

        void ActionLoop();
        bool IsSysTimeValid();
        void SendEventToBackend(const Event& event);

        static uint64_t GetCurrentTimestampInMs();
        static uint64_t GetCurrentUptimeInMs();
        static uint64_t ConvertUptimeToTimestampInMs(uint64_t uptimeMs);

        std::mutex mQueueMutex;
        std::condition_variable mQueueCondition;
        std::thread mThread;
        std::queue<Action> mActionQueue;
        std::queue<Event> mEventQueue;
        const IAnalyticsBackends mBackends;
        bool mSysTimeValid;
        PluginHost::IShell* mShell;
    };
}
}