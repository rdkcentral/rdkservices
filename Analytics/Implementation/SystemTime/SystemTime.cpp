/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/
#include "SystemTime.h"
#include "UtilsLogging.h"
#include "secure_wrapper.h"

#define JSONRPC_THUNDER_TIMEOUT 2000
#define THUNDER_ACCESS_DEFAULT_VALUE "127.0.0.1:9998"
#define SYSTEM_CALLSIGN "org.rdk.System.1"

namespace WPEFramework
{
    namespace Plugin
    {
        const std::string TIME_QUALITY_STALE{"Stale"};
        const std::string TIME_QUALITY_GOOD{"Good"};
        const std::string TIME_QUALITY_SECURE{"Secure"};

        SystemTime::SystemTime(PluginHost::IShell *shell) : mQueueLock(),
                                                            mQueueCondition(),
                                                            mQueue(),
                                                            mLock(),
                                                            mSystemLink(nullptr),
                                                            mTimeQuality(TIME_QUALITY_STALE),
                                                            mTimeZone(),
                                                            mTimeZoneAccuracyString(),
                                                            mTransitionMap(),
                                                            mIsSystemTimeAvailable(false),
                                                            mShell(shell)
        {
            mEventThread = std::thread(&SystemTime::EventLoop, this);

            Event event = {EVENT_INITIALISE, std::string()};
            std::unique_lock<std::mutex> lock(mQueueLock);
            mQueue.push(event);
            lock.unlock();
            mQueueCondition.notify_one();
        }

        SystemTime::~SystemTime()
        {
            LOGINFO("SystemTime::~SystemTime");
            Event event = {EVENT_SHUTDOWN, std::string()};
            {
                std::lock_guard<std::mutex> lock(mQueueLock);
                mQueue.push(event);
            }
            mQueueCondition.notify_one();
            mEventThread.join();
            if (mSystemLink != nullptr)
            {
                mSystemLink->Unsubscribe(JSONRPC_THUNDER_TIMEOUT, _T("onTimeStatusChanged"));
                mSystemLink->Unsubscribe(JSONRPC_THUNDER_TIMEOUT, _T("onTimeZoneDSTChanged"));
            }
        }

        bool SystemTime::IsSystemTimeAvailable()
        {
            // Time status is updated during init and on event
            std::lock_guard<std::mutex> guard(mLock);
            return mIsSystemTimeAvailable;
        }

        ISystemTime::TimeZoneAccuracy SystemTime::GetTimeZoneOffset(int32_t &offsetSec)
        {
            std::lock_guard<std::mutex> guard(mLock);

            if (mIsSystemTimeAvailable)
            {
                std::pair<ISystemTime::TimeZoneAccuracy, int32_t> tzParsed = ParseTimeZone();
                offsetSec = tzParsed.second;
                return tzParsed.first;
            }
            return ACC_UNDEFINED;
        }

        void SystemTime::onTimeStatusChanged(const JsonObject& parameters)
        {
            std::string parametersString;
            parameters.ToString(parametersString);
            LOGINFO("onTimeStatusChanged: %s", parametersString.c_str());
            Event event = {EVENT_TIME_STATUS_CHANGED, parametersString};
            std::lock_guard<std::mutex> lock(mQueueLock);
            mQueue.push(event);
            mQueueCondition.notify_one();
        }

        void SystemTime::onTimeZoneDSTChanged(const JsonObject& parameters)
        {
            std::string parametersString;
            parameters.ToString(parametersString);
            LOGINFO("onTimeZoneDSTChanged: %s", parametersString.c_str());
            Event event = {EVENT_TIME_ZONE_CHANGED, parametersString};
            std::lock_guard<std::mutex> lock(mQueueLock);
            mQueue.push(event);
            mQueueCondition.notify_one();
        }

        void SystemTime::CreateSystemLink()
        {
            if (mSystemLink == nullptr)
            {
                std::string thunderAccessValue = THUNDER_ACCESS_DEFAULT_VALUE;
                char *thunderAccessValueEnv = getenv("THUNDER_ACCESS_VALUE");
                if (NULL != thunderAccessValueEnv)
                {
                    thunderAccessValue = thunderAccessValueEnv;
                }

                // Generate jsonrpc token
                std::string token;
                // TODO: use interfaces and remove token
                auto security = mShell->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
                if (security != nullptr)
                {
                    std::string payload = "http://localhost";
                    if (security->CreateToken(
                            static_cast<uint16_t>(payload.length()),
                            reinterpret_cast<const uint8_t *>(payload.c_str()),
                            token) == Core::ERROR_NONE)
                    {
                        LOGINFO("Got security token\n");
                    }
                    else
                    {
                        LOGINFO("Failed to get security token\n");
                    }
                    security->Release();
                }
                else
                {
                    LOGINFO("No security agent\n");
                }

                std::string query = "token=" + token;
                Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T(thunderAccessValue)));
                mSystemLink = std::make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>(SYSTEM_CALLSIGN, "", false, query);
            }
        }

        void SystemTime::SubscribeForEvents()
        {
            if (mSystemLink != nullptr)
            {
                uint32_t ret = mSystemLink->Subscribe<JsonObject>(JSONRPC_THUNDER_TIMEOUT, _T("onTimeStatusChanged"), &SystemTime::onTimeStatusChanged, this);
                if (ret != Core::ERROR_NONE)
                {
                    LOGERR("Failed to subscribe to onTimeStatusChanged");
                }

                ret = mSystemLink->Subscribe<JsonObject>(JSONRPC_THUNDER_TIMEOUT, _T("onTimeZoneDSTChanged"), &SystemTime::onTimeZoneDSTChanged, this);
                if (ret != Core::ERROR_NONE)
                {
                    LOGERR("Failed to subscribe to onTimeZoneDSTChanged");
                }
            }
        }

        void SystemTime::UpdateTimeStatus()
        {
            JsonObject params;
            JsonObject response;

            uint32_t result = mSystemLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getTimeStatus", params, response);
            if (result == Core::ERROR_NONE && response.HasLabel("TimeQuality"))
            {
                std::lock_guard<std::mutex> guard(mLock);
                mTimeQuality = response["TimeQuality"].String();
                if (mTimeQuality == TIME_QUALITY_GOOD || mTimeQuality == TIME_QUALITY_SECURE)
                {
                    mIsSystemTimeAvailable = true;
                }
                else
                {
                    mIsSystemTimeAvailable = false;
                }
            }
            else
            {
                LOGERR("getTimeStatus not available, assuming time is OK");
                std::lock_guard<std::mutex> guard(mLock);
                mIsSystemTimeAvailable = true;
            }
        }

        void SystemTime::UpdateTimeZone()
        {
            JsonObject params;
            JsonObject response;
            // Get timeZone from System.1.getTimeZoneDST
            uint32_t result = mSystemLink->Invoke<JsonObject, JsonObject>(JSONRPC_THUNDER_TIMEOUT, "getTimeZoneDST", params, response);
            if (result == Core::ERROR_NONE && response.HasLabel("timeZone") && response.HasLabel("accuracy"))
            {
                std::string tz = response["timeZone"].String();
                std::string accuracy = response["accuracy"].String();

                std::lock_guard<std::mutex> guard(mLock);
                mTimeZoneAccuracyString = accuracy;
                if (accuracy == "FINAL")
                {
                    if (mTimeZone != tz)
                    {
                        mTransitionMap.clear();
                        mTimeZone = tz;
                    }
                }
                else
                {
                    mTimeZone.clear();
                    mTransitionMap.clear();
                }
            }
        }

        std::pair<ISystemTime::TimeZoneAccuracy, int32_t> SystemTime::ParseTimeZone()
        {
            std::pair<ISystemTime::TimeZoneAccuracy, int32_t> result = {ACC_UNDEFINED, 0};
            if (mTimeZone.empty())
            {
                return result;
            }

            static const std::map<std::string, ISystemTime::TimeZoneAccuracy> accuracyMap = {
                {"INITIAL", INITIAL},
                {"INTERIM", INTERIM},
                {"FINAL", FINAL}};

            auto accuracyItr = accuracyMap.find(mTimeZoneAccuracyString);
            if (accuracyItr == accuracyMap.end())
            {
                result.first = ACC_UNDEFINED;
            } else {
                result.first = accuracyItr->second;
            }

            if (mTimeZone == "Universal")
            {
                result.second = 0;
                return result;
            }

            PopulateTimeZoneTransitionMap();

            if (mTransitionMap.empty())
            {
                result.first = ACC_UNDEFINED;
                return result;
            }

            time_t currentTime = time(NULL);

            auto currentTimeEndItr = mTransitionMap.lower_bound(currentTime);

            if (currentTimeEndItr != mTransitionMap.end())
            {
                result.second = currentTimeEndItr->second;
            }
            else if (mTransitionMap.empty() == false)
            {
                currentTimeEndItr--; // take the last transition when all transitions are from past
                result.second = currentTimeEndItr->second;
            }
            else
            {
                LOGERR( "There is no time transition information for this timezone: %s", mTimeZone.c_str());
                result.second = 0;
                result.first = ACC_UNDEFINED;
            }

            return result;
        }

        void SystemTime::PopulateTimeZoneTransitionMap()
        {
            if (mTransitionMap.empty() && !mTimeZone.empty())
            {
                std::string cmd = "zdump -v " + mTimeZone;
                FILE *fp = popen(cmd.c_str(), "r");
                if (fp != NULL)
                {
                    LOGINFO("popen of '%s' succeeded", cmd.c_str());

                    // Read entire output of zdump
                    char buf[4096] = {0};
                    std::string output;
                    while (fgets(buf, sizeof(buf), fp) != NULL)
                    {
                        output += buf;
                    }

                    pclose(fp);

                    // Convert output to read line by line
                    std::istringstream iss(output);
                    std::string line;
                    while (std::getline(iss, line))
                    {
                        // "<timezone>  Tue Jan 19 03:14:07 2038 UT = Tue Jan 19 04:14:07 2038 CET isdst=0 gmtoff=3600"
                        struct tm utcTime = {0};

                        line.erase(0, line.find(" ") + 2); // Remove "<timezone>  " -> 2 spaces

                        auto utOff = line.find(" UT = ");
                        std::string date = line.substr(0, utOff); // Capture UTC date "Tue Jan 19 03:14:07 2038"

                        // Remove everything to " UT = ", "Tue Jan 19 04:14:07 2038 CET isdst=0 gmtoff=3600" is left
                        line.erase(0, utOff + sizeof(" UT = ") - 1);

                        // Remove until next 5 spaces, "CET isdst=0 gmtoff=3600" will be left
                        auto spaceCount = 5;
                        while (spaceCount > 0)
                        {
                            auto spOff = line.find(" ");
                            // In case of one-digit day of month there are two spaces before that
                            if (spaceCount == 4)
                            {
                                // thus + 2 to remove it (it applies also to two-digits but it's ok because it's removed anyway)
                                line.erase(0, spOff + 2);
                            }
                            else
                            {
                                line.erase(0, spOff + 1);
                            }
                            spaceCount--;
                        }

                        char timeZone[5] = {0};
                        int32_t isDst = 0;
                        int32_t gmtOff = 0;
                        sscanf(line.c_str(), "%s isdst=%d gmtoff=%d", timeZone, &isDst, &gmtOff);
                        strptime(date.c_str(), "%A %B %d %H:%M:%S %Y", &utcTime);

                        // Years below 70 are not supported by epoch
                        if (utcTime.tm_year > 70)
                        {
                            utcTime.tm_zone = "UTC";
                            utcTime.tm_gmtoff = 0;

                            // years after 2038 are rounding off to -1
                            if (utcTime.tm_year < 138)
                            {
                                time_t utcTimeGm = timegm(&utcTime);
                                mTransitionMap[utcTimeGm] = gmtOff;
                            }
                            else
                            {
                                break; // stop parsing as we wont add anything after 2038 to map
                            }
                        }
                        else
                        {
                            // validate the line
                            if (utcTime.tm_year > 0)
                            {
                                mTransitionMap[utcTime.tm_year] = gmtOff;
                            }
                        }
                    }
                }
                else
                {
                    LOGERR("popen of zdump -v %s failed", mTimeZone.c_str());
                }
            }
        }


        void SystemTime::EventLoop()
        {
            while (true)
            {
                Event event;
                {
                    std::unique_lock<std::mutex> lock(mQueueLock);
                    mQueueCondition.wait(lock, [this]
                                         { return !mQueue.empty(); });
                    event = mQueue.front();
                    mQueue.pop();
                }

                switch (event.type)
                {
                case EVENT_INITIALISE:
                {
                    if (mSystemLink == nullptr)
                    {
                        CreateSystemLink();
                        if (mSystemLink != nullptr)
                        {
                            SubscribeForEvents();
                            UpdateTimeStatus();
                            UpdateTimeZone();
                        }
                        else
                        {
                            LOGERR("Failed to create JSONRPC link with %s", SYSTEM_CALLSIGN);
                        }
                    }
                }
                break;
                case EVENT_TIME_STATUS_CHANGED:
                {
                    JsonObject response(event.payload);
                    if (response.HasLabel("TimeQuality"))
                    {
                        std::lock_guard<std::mutex> guard(mLock);
                        mTimeQuality = response["TimeQuality"].String();
                        if (mTimeQuality == TIME_QUALITY_GOOD || mTimeQuality == TIME_QUALITY_SECURE)
                        {
                            mIsSystemTimeAvailable = true;
                        }
                        else
                        {
                            mIsSystemTimeAvailable = false;
                        }
                    }
                }
                break;
                case EVENT_TIME_ZONE_CHANGED:
                {
                    JsonObject response(event.payload);
                    if (response.HasLabel("newTimeZone") && response.HasLabel("newAccuracy"))
                    {
                        std::string tz = response["newTimeZone"].String();
                        std::string accuracy = response["newAccuracy"].String();

                        std::lock_guard<std::mutex> guard(mLock);
                        mTimeZoneAccuracyString = accuracy;
                        if (accuracy == "FINAL")
                        {
                            if (mTimeZone != tz)
                            {
                                mTransitionMap.clear();
                                mTimeZone = tz;
                            }
                        }
                        else
                        {
                            mTimeZone.clear();
                            mTransitionMap.clear();
                        }
                    }
                }
                break;
                case EVENT_SHUTDOWN:
                {
                    LOGINFO("Shutting down SystemTime event loop");
                    return;
                }
                default:
                {
                    LOGERR("Unhandled event received, event: %d", event.type);
                    break;
                }
                }
            }
        }
    }
}
