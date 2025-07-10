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
#include "AnalyticsBackendLoader.h"
#include "SystemTime.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <unordered_map>

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
            std::string appId;
            std::string eventPayload;
        };

        struct Action
        {
            ActionType type;
            std::shared_ptr<Event> payload;
            std::string id;
        };

        class EventMapper
        {
        private:

            struct Key
            {
                std::string eventName;
                std::string eventSource;
                std::string eventSourceVersion;
                std::string eventVersion;

                bool operator==(const Key &other) const
                {
                    return eventName == other.eventName &&
                           eventSource == other.eventSource &&
                           eventSourceVersion == other.eventSourceVersion &&
                           eventVersion == other.eventVersion;
                }
            };

            struct KeyHasher
            {
                std::size_t operator()(const Key &key) const
                {
                    std::size_t h1 = std::hash<std::string>()(key.eventName);
                    std::size_t h2 = std::hash<std::string>()(key.eventSource);
                    std::size_t h3 = std::hash<std::string>()(key.eventSourceVersion);
                    std::size_t h4 = std::hash<std::string>()(key.eventVersion);

                    // Combine the hashes
                    std::size_t combined = h1;
                    combined ^= h2 + 0x9e3779b9 + (combined << 6) + (combined >> 2);
                    combined ^= h3 + 0x9e3779b9 + (combined << 6) + (combined >> 2);
                    combined ^= h4 + 0x9e3779b9 + (combined << 6) + (combined >> 2);
                    return combined;
                }
            };

        public:

            void FromString(const std::string &jsonArrayStr);
            std::string MapEventNameIfNeeded(const std::string &eventName,
                                            const std::string &eventSource,
                                            const std::string &eventSourceVersion,
                                            const std::string &eventVersion) const;

        private:
            std::unordered_map<Key, std::string, KeyHasher> map;
        };

        // IAnalyticsImplementation interface
        Core::hresult SendEvent(const string& eventName,
                                   const string& eventVersion,
                                   const string& eventSource,
                                   const string& eventSourceVersion,
                                   IStringIterator* const& cetList,
                                   const uint64_t epochTimestamp,
                                   const uint64_t uptimeTimestamp,
                                   const string& appId,
                                   const string& eventPayload) override;


        // IConfiguration interface
        uint32_t Configure(PluginHost::IShell* shell);

        void ActionLoop();
        bool IsSysTimeValid();
        void SendEventToBackend(const Event& event);
        void ParseEventsMapFile(const std::string& eventsMapFile);

        static uint64_t GetCurrentTimestampInMs();
        static uint64_t GetCurrentUptimeInMs();
        static uint64_t ConvertUptimeToTimestampInMs(uint64_t uptimeMs);

        std::mutex mQueueMutex;
        std::condition_variable mQueueCondition;
        std::thread mThread;
        std::queue<Action> mActionQueue;
        std::queue<Event> mEventQueue;
        std::map<std::string, IAnalyticsBackendPtr> mBackends;
        AnalyticsBackendLoader mBackendLoader;
        bool mSysTimeValid;
        PluginHost::IShell* mShell;
        SystemTimePtr mSysTime;
        EventMapper mEventMapper;
    };
}
}
