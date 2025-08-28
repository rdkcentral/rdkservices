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
#include "UtilsLogging.h"
#include "UtilsTelemetry.h"
#include "LocalStore.h"

#include <fstream>
#include <streambuf>
#include <sys/sysinfo.h>

namespace WPEFramework {
namespace Plugin {

    const uint32_t POPULATE_DEVICE_INFO_RETRY_MS = 3000;

    class AnalyticsConfig : public Core::JSON::Container {
        private:
            AnalyticsConfig(const AnalyticsConfig&) = delete;
            AnalyticsConfig& operator=(const AnalyticsConfig&) = delete;
          
        public:
            AnalyticsConfig()
                : Core::JSON::Container()
                , EventsMap()
                , BackendLib()
            {
                Add(_T("eventsmap"), &EventsMap);
                Add(_T("backendlib"), &BackendLib);
            }
            ~AnalyticsConfig()
            {
            }

        public:
            Core::JSON::String EventsMap;
            Core::JSON::String BackendLib;
        };

    SERVICE_REGISTRATION(AnalyticsImplementation, 1, 0);

    AnalyticsImplementation::AnalyticsImplementation():
        mQueueMutex(),
        mQueueCondition(),
        mActionQueue(),
        mEventQueue(),
        mBackends(),
        mBackendLoader(),
        mSysTimeValid(false),
        mShell(nullptr)
    {
        LOGINFO("AnalyticsImplementation::AnalyticsImplementation()");
        Utils::Telemetry::init();
        Utils::Telemetry::sendMessage("AnalyticsImplementation::Init");
        mThread = std::thread(&AnalyticsImplementation::ActionLoop, this);
    }

    AnalyticsImplementation::~AnalyticsImplementation()
    {
        LOGINFO("AnalyticsImplementation::~AnalyticsImplementation()");
        Utils::Telemetry::sendMessage("AnalyticsImplementation::Shutdown");
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mActionQueue.push({ACTION_TYPE_SHUTDOWN, nullptr});
        lock.unlock();
        mQueueCondition.notify_one();
        mThread.join();
    }

    /* virtual */ Core::hresult AnalyticsImplementation::SendEvent(const string& eventName,
                                    const string& eventVersion,
                                    const string& eventSource,
                                    const string& eventSourceVersion,
                                    IStringIterator* const& cetList,
                                    const uint64_t epochTimestamp,
                                    const uint64_t uptimeTimestamp,
                                    const string& appId,
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
        event->appId = appId;
        event->eventPayload = eventPayload;

        LOGINFO("Epoch Timestamp:  %" PRIu64, epochTimestamp);
        LOGINFO("Uptime Timestamp: %" PRIu64, uptimeTimestamp);
        LOGINFO("Event Payload: %s", eventPayload.c_str());

        bool valid = true;
        if (eventName.empty())
        {
            LOGERR("eventName is empty");
            valid = false;
        }
        if (eventSource.empty())
        {
            LOGERR("eventSource is empty");
            valid = false;
        }
        if (eventSourceVersion.empty())
        {
            LOGERR("eventSourceVersion is empty");
            valid = false;
        }
        if (eventPayload.empty())
        {
            LOGERR("eventPayload is empty");
            valid = false;
        }

        if (valid == false)
        {
            Utils::Telemetry::sendError("AnalyticsImplementation::SendEvent: Invalid event parameters for event '%s' from source '%s' with version '%s'",
                                        eventName.c_str(), eventSource.c_str(), eventSourceVersion.c_str());
            return Core::ERROR_GENERAL;
        }

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

    uint32_t AnalyticsImplementation::Configure(PluginHost::IShell* shell)
    {
        LOGINFO("Configuring Analytics");
        uint32_t result = Core::ERROR_NONE;
        ASSERT(shell != nullptr);
        mShell = shell;

        mSysTime = std::make_shared<SystemTime>(shell);
        if(mSysTime == nullptr)
        {
            LOGERR("Failed to create SystemTime instance");
            Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to create SystemTime instance");
        }


        std::string configLine = mShell->ConfigLine();
        Core::OptionalType<Core::JSON::Error> error;
        AnalyticsConfig config;

        if (config.FromString(configLine, error) == false)
        {
            SYSLOG(Logging::ParsingError,
                   (_T("Failed to parse config line, error: '%s', config line: '%s'."),
                    (error.IsSet() ? error.Value().Message().c_str() : "Unknown"),
                    configLine.c_str()));
            Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to parse config line, error: '%s', config line: '%s'.",
                                        (error.IsSet() ? error.Value().Message().c_str() : "Unknown"), configLine.c_str());
        }

        LOGINFO("EventsMap: %s", config.EventsMap.Value().c_str());
        ParseEventsMapFile(config.EventsMap.Value());

        // TODO: Add support for multiple backends
        uint32_t ret = mBackendLoader.Load(config.BackendLib.Value());
        if (ret != Core::ERROR_NONE)
        {
            LOGERR("Failed to load backend library: %s, error code: %u",
                   config.BackendLib.Value().c_str(), ret);
            Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to load backend library: %s, error code: %u",
                                        config.BackendLib.Value().c_str(), ret);
        }
        else
        {
            IAnalyticsBackendPtr backend = mBackendLoader.GetBackend();
            if (backend != nullptr)
            {
                mBackends[backend->Name()] = backend;
                LOGINFO("Created backend: %s", backend->Name().c_str());
                // Configure backend
                ILocalStorePtr localStore = std::make_shared<LocalStore>();
                if (localStore == nullptr)
                {
                    LOGERR("Failed to create LocalStore instance");
                    Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to create LocalStore instance");
                    result = Core::ERROR_GENERAL;
                }
                if (backend->Configure(mShell, mSysTime, std::move(localStore)) != Core::ERROR_NONE)
                {
                    LOGERR("Failed to configure backend: %s", backend->Name().c_str());
                    Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to configure backend: %s",
                                                backend->Name().c_str());
                    result = Core::ERROR_GENERAL;
                }
                else
                {
                    LOGINFO("Backend %s configured successfully", backend->Name().c_str());
                }
            }
            else
            {
                LOGERR("Failed to get backend from loader");
                Utils::Telemetry::sendError("AnalyticsImplementation::Configure - Failed to get backend from loader");
                result = Core::ERROR_GENERAL;
            }
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

            Action action = {ACTION_TYPE_UNDEF, nullptr};

            if (mActionQueue.empty() && !mSysTimeValid)
            {
                action = {ACTION_POPULATE_TIME_INFO, nullptr};
            }
            else if (!mActionQueue.empty())
            {
                action = mActionQueue.front();
                mActionQueue.pop();
            }

            lock.unlock();

            switch (action.type) {
                case ACTION_POPULATE_TIME_INFO:

                mSysTimeValid = IsSysTimeValid();

                if ( mSysTimeValid )
                {
                    // Send the events from the queue, if there are any.
                    while ( !mEventQueue.empty() )
                    {
                        AnalyticsImplementation::Event event = mEventQueue.front();
                        // convert uptime to epoch timestamp
                        if (event.epochTimestamp == 0)
                        {
                            event.epochTimestamp = ConvertUptimeToTimestampInMs(event.uptimeTimestamp);
                        }

                        SendEventToBackend( event );
                        mEventQueue.pop();
                    }
                }
                break;
                case ACTION_TYPE_SEND_EVENT:

                    if (mSysTimeValid)
                    {
                        // Add epoch timestamp if needed
                        // It should have at least uptime already
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
                            // Store the event in the queue with uptime only
                            mEventQueue.push(*action.payload);
                            LOGINFO("SysTime not ready, event awaiting in queue: %s", action.payload->eventName.c_str());
                        }
                    }
                    break;
                case ACTION_TYPE_SHUTDOWN:
                    LOGINFO("Shutting down Analytics");
                    return;
                default:
                    break;
            }

            lock.lock();
        }
    }

    bool AnalyticsImplementation::IsSysTimeValid()
    {
        bool ret = false;
        // Time is valid if system time is available and time zone is set
        if (mSysTime != nullptr)
        {
            if (mSysTime->IsSystemTimeAvailable())
            {
                int32_t offset = 0;
                SystemTime::TimeZoneAccuracy acc = mSysTime->GetTimeZoneOffset(offset);
                if (acc == SystemTime::TimeZoneAccuracy::FINAL)
                {
                   ret = true;
                }
            }
        }

        return ret;
    }

    void AnalyticsImplementation::SendEventToBackend(const AnalyticsImplementation::Event& event)
    {
        IAnalyticsBackend::Event backendEvent;
        backendEvent.eventName = mEventMapper.MapEventNameIfNeeded(event.eventName, event.eventSource,
            event.eventSourceVersion, event.eventVersion);
        backendEvent.eventVersion = event.eventVersion;
        backendEvent.eventSource = event.eventSource;
        backendEvent.eventSourceVersion = event.eventSourceVersion;
        backendEvent.epochTimestamp = event.epochTimestamp;
        backendEvent.eventPayload = event.eventPayload;
        backendEvent.appId = event.appId;
        backendEvent.cetList = event.cetList;

        //TODO: Add mapping of event source/name to the desired backend
        if (mBackends.empty())
        {
            LOGINFO("No backends available!");
        }
        else //send to the first backend
        {
            LOGINFO("Sending event '%s' to backend: %s", backendEvent.eventName.c_str(), mBackends.begin()->first.c_str());
            mBackends.begin()->second->SendEvent(backendEvent);
        }
    }

    void AnalyticsImplementation::ParseEventsMapFile(const std::string& eventsMapFile)
    {
        if (eventsMapFile.empty())
        {
            LOGINFO("Events map file path is empty, skipping parsing");
            return;
        }
        std::ifstream t(eventsMapFile);
        std::string str((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
        mEventMapper.FromString(str);
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

    void AnalyticsImplementation::EventMapper::FromString(const std::string &jsonArrayStr)
    {
        // expect json array:
        // [
        //     {
        //       "event_name": "event_name",
        //       "event_source": "event_source",
        //       "event_source_version": "event_source_version", - optional
        //       "event_version": "event_version", - optional
        //       "mapped_event_name": "mapped_event_name"
        //     }
        // ]
        if (jsonArrayStr.empty())
        {
            LOGERR("Empty events map json array string");
            Utils::Telemetry::sendError("AnalyticsImplementation::EventMapper::FromString - Empty events map json array string");
            return;
        }

        JsonArray array;
        array.FromString(jsonArrayStr);

        if (array.Length() == 0)
        {
            LOGERR("Empty or corrupted events map json array");
            Utils::Telemetry::sendError("AnalyticsImplementation::EventMapper::FromString - Empty or corrupted events map json array");
            return;
        }

        for (int i = 0; i < array.Length(); i++)
        {
            JsonObject entry = array[i].Object();
            if (entry.HasLabel("event_name") && entry.HasLabel("event_source") && entry.HasLabel("mapped_event_name"))
            {
                AnalyticsImplementation::EventMapper::Key key{
                    entry["event_name"].String(),
                    entry["event_source"].String(),
                    entry.HasLabel("event_source_version") ? entry["event_source_version"].String() : "",
                    entry.HasLabel("event_version") ? entry["event_version"].String() : ""};

                std::string mapped_event_name = entry["mapped_event_name"].String();
                map[key] = mapped_event_name;
                LOGINFO("Index %d: Mapped event: %s -> %s", i, entry["event_name"].String().c_str(), mapped_event_name.c_str());
            }
            else
            {
                LOGERR("Invalid entry in events map file at index %d", i);
                Utils::Telemetry::sendError("AnalyticsImplementation::EventMapper::FromString - Invalid entry in events map file at index %d", i);
            }
        }
    }

    std::string AnalyticsImplementation::EventMapper::MapEventNameIfNeeded(const std::string &eventName,
                                            const std::string &eventSource,
                                            const std::string &eventSourceVersion,
                                            const std::string &eventVersion) const
    {
        if (map.empty())
        {
            return eventName; // No mapping available, return original event name
        }
        // Try exact match
        AnalyticsImplementation::EventMapper::Key key{eventName, eventSource, eventSourceVersion, eventVersion};
        auto it = map.find(key);
        if (it != map.end())
        {
            return it->second;
        }

        // Try without eventVersion
        if (!eventVersion.empty())
        {
            AnalyticsImplementation::EventMapper::Key partialKey{eventName, eventSource, eventSourceVersion, ""};
            it = map.find(partialKey);
            if (it != map.end())
            {
                return it->second;
            }
        }

        // Try without eventSourceVersion
        if (!eventSourceVersion.empty())
        {
            AnalyticsImplementation::EventMapper::Key partialKey{eventName, eventSource, "", eventVersion};
            it = map.find(partialKey);
            if (it != map.end())
            {
                return it->second;
            }
        }

        // Try without both eventSourceVersion and eventVersion
        if (!eventSourceVersion.empty() || !eventVersion.empty())
        {
            AnalyticsImplementation::EventMapper::Key partialKey{eventName, eventSource, "", ""};
            it = map.find(partialKey);
            if (it != map.end())
            {
                return it->second;
            }
        }

        return eventName; // Not found, nothing to map
    }
}
}
