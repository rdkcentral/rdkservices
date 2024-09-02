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
#include "AnalyticsImplementation.h"
#include "Backend/AnalyticsBackend.h"
#include "UtilsLogging.h"

#include <fstream>
#include <streambuf>
#include <sys/sysinfo.h>

namespace WPEFramework {
namespace Plugin {

    const uint32_t POPULATE_DEVICE_INFO_RETRY_MS = 3000;

    SERVICE_REGISTRATION(AnalyticsImplementation, 1, 0);

    AnalyticsImplementation::AnalyticsImplementation():
        mQueueMutex(),
        mQueueCondition(),
        mBackends(IAnalyticsBackendAdministrator::Instances())
    {
        mThread = std::thread(&AnalyticsImplementation::ActionLoop, this);
        mThread.detach();
    }

    AnalyticsImplementation::~AnalyticsImplementation()
    {
        mQueueMutex.lock();
        mActionQueue.push({ACTION_TYPE_SHUTDOWN, nullptr});
        mQueueMutex.unlock();
        mQueueCondition.notify_one();
        mThread.join();
    }

    /* virtual */ uint32_t AnalyticsImplementation::SendEvent(const string& eventName,
                                    const string& eventVersion,
                                    const string& eventSource,
                                    const string& eventSourceVersion,
                                    RPC::IStringIterator* const& cetList,
                                    const uint64_t& epochTimestamp,
                                    const uint64_t& uptimeTimestamp,
                                    const string& eventPayload)
    {
        std::shared_ptr<Event> event = std::make_shared<Event>();
        event->eventName = eventName;
        event->eventVersion = eventVersion;
        event->eventSource = eventSource;
        event->eventSourceVersion = eventSourceVersion;

        LOGINFO("Event Name: %s", eventName.c_str());
        LOGINFO("Event Version: %s", eventVersion.c_str());
        LOGINFO("Event Source: %s", eventSource.c_str());
        LOGINFO("Event Source Version: %s", eventSourceVersion.c_str());
        LOGINFO("cetList[]: ");
        std::string entry;
        while (cetList->Next(entry) == true) {
            event->cetList.push_back(entry);
            LOGINFO("     %s ", entry.c_str());
        }
        event->epochTimestamp = epochTimestamp;
        event->uptimeTimestamp = uptimeTimestamp;
        event->eventPayload = eventPayload;

        LOGINFO("Epoch Timestamp:  %" PRIu64, epochTimestamp);
        LOGINFO("Uptime Timestamp: %" PRIu64, uptimeTimestamp);
        LOGINFO("Event Payload: %s", eventPayload.c_str());

        // Fill the uptime if no time provided
        if (event->epochTimestamp == 0 && event->uptimeTimestamp == 0)
        {
            event->uptimeTimestamp = GetCurrentUptimeInMs();
        }

        std::unique_lock<std::mutex> lock(mQueueMutex);
        mActionQueue.push({ACTION_TYPE_SEND_EVENT, event});
        lock.unlock();
        mQueueCondition.notify_one();
        return Core::ERROR_NONE;
    }

    uint32_t AnalyticsImplementation::SetSessionId(const string& id)
    {
        uint32_t ret = Core::ERROR_GENERAL;
        // set session id in sift backend
        if (mBackends.find(IAnalyticsBackend::SIFT) != mBackends.end())
        {
            ret = mBackends.at(IAnalyticsBackend::SIFT).SetSessionId(id);
        }

        return ret;
    }

    uint32_t AnalyticsImplementation::SetTimeReady()
    {
        // set time ready action
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mActionQueue.push({ACTION_TYPE_SET_TIME_READY, nullptr});
        lock.unlock();
        mQueueCondition.notify_one();
        return Core::ERROR_NONE;
    }

    uint32_t AnalyticsImplementation::Configure(PluginHost::IShell* shell)
    {
        LOGINFO("Configuring Analytics");
        uint32_t result = Core::ERROR_NONE;
        ASSERT(shell != nullptr);
        mShell = shell;

        for (auto &backend : mBackends)
        {
            LOGINFO("Configuring backend: %s", backend.first.c_str());
            backend.second.Configure(shell);
        }

        return result;
    }

    void AnalyticsImplementation::ActionLoop()
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);

        while (true) {

            std::chrono::milliseconds queueTimeout(std::chrono::milliseconds::max());

            if (!mSysTimeValid)
            {
                queueTimeout = std::chrono::milliseconds(POPULATE_DEVICE_INFO_RETRY_MS);
            }

            if (queueTimeout == std::chrono::milliseconds::max())
            {
                mQueueCondition.wait(lock, [this] { return !mActionQueue.empty(); });
            }
            else
            {
                mQueueCondition.wait_for(lock, queueTimeout, [this] { return !mActionQueue.empty(); });
            }

            Action action = {ACTION_TYPE_UNDEF, nullptr};

            if (mActionQueue.empty() && !mSysTimeValid)
            {
                action = {ACTION_POPULATE_TIME_INFO, nullptr};
            }
            else
            {
                action = mActionQueue.front();
                mActionQueue.pop();
            }

            lock.unlock();

            switch (action.type) {
                case ACTION_POPULATE_TIME_INFO:

                //mSysTimeValid = IsSysTimeValid();

                if ( mSysTimeValid )
                {
                    // Send the events from the queue, if there are any.
                    while ( !mEventQueue.empty() )
                    {
                        SendEventToBackend( mEventQueue.front() );
                        mEventQueue.pop();
                    }
                }
                break;
                case ACTION_TYPE_SEND_EVENT:

                    if (mSysTimeValid)
                    {
                        // Add epoch timestamp if needed
                        // It should have at  least uptime already
                        if (action.payload->epochTimestamp == 0)
                        {
                            action.payload->epochTimestamp = ConvertUptimeToTimestampInMs(action.payload->uptimeTimestamp);
                        }

                        SendEventToBackend(*action.payload);
                    }
                    else
                    {
                        // pass to backend if epoch available
                        if (action.payload->epochTimestamp != 0)
                        {
                            SendEventToBackend(*action.payload);
                        }
                        else
                        {
                            // Store the event in the queue
                            mEventQueue.push(*action.payload);
                        }
                    }
                    break;
                case ACTION_TYPE_SHUTDOWN:
                    return;
                case ACTION_TYPE_SET_TIME_READY:
                {
                    mSysTimeValid = true;
                }break;
                default:
                    break;
            }

            lock.lock();
        }
    }

    bool AnalyticsImplementation::IsSysTimeValid()
    {
        bool ret = false;
        //TODO: Add system time validation

        return ret;
    }

    void AnalyticsImplementation::SendEventToBackend(const AnalyticsImplementation::Event& event)
    {
        //TODO: Add mapping of event source/name to backend
        IAnalyticsBackend::Event backendEvent;
        backendEvent.eventName = event.eventName;
        backendEvent.eventVersion = event.eventVersion;
        backendEvent.eventSource = event.eventSource;
        backendEvent.eventSourceVersion = event.eventSourceVersion;
        backendEvent.epochTimestamp = event.epochTimestamp;
        backendEvent.eventPayload = event.eventPayload;
        backendEvent.cetList = event.cetList;

        if (mBackends.empty())
        {
            LOGINFO("No backends available!");
        }
        else if (mBackends.find(IAnalyticsBackend::SIFT) != mBackends.end())
        {
            LOGINFO("Sending event to Sift backend");
            mBackends.at(IAnalyticsBackend::SIFT).SendEvent(backendEvent);
        }
    }

    uint64_t AnalyticsImplementation::GetCurrentTimestampInMs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
    }

    uint64_t AnalyticsImplementation::GetCurrentUptimeInMs()
    {
        struct sysinfo info = {0};
        sysinfo(&info);
        return info.uptime * 1000;
    }

    uint64_t AnalyticsImplementation::ConvertUptimeToTimestampInMs(uint64_t uptimeMs)
    {
        uint64_t uptimeDiff = 0;
        uint64_t currentUptime = GetCurrentUptimeInMs();
        uint64_t currentTimestamp = GetCurrentTimestampInMs();
        if (currentUptime > uptimeMs)
        {
            uptimeDiff = currentUptime - uptimeMs;
        }

        return currentTimestamp - uptimeDiff;
    }

}
}