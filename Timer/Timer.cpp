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

#include "Timer.h"


#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIBus.h"
#include "ccec/drivers/CecIARMBusMgr.h"
#endif

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

// Methods
#define TIMER_METHOD_START_TIMER          "startTimer"
#define TIMER_METHOD_CANCEL               "cancel"
#define TIMER_METHOD_SUSPEND              "suspend"
#define TIMER_METHOD_RESUME               "resume"
#define TIMER_METHOD_GET_TIMER_STATUS     "getTimerStatus"
#define TIMER_METHOD_GET_TIMERS           "getTimers"

// Events
#define TIMER_EVT_TIMER_EXPIRED           "timerExpired"
#define TIMER_EVT_TIMER_EXPIRY_REMINDER   "timerExpiryReminder"

#define TIMER_ACCURACY 0.001 // 10 milliseconds

static const char* stateStrings[] = {
    "",
    "RUNNING",
    "SUSPENDED",
    "CANCELED",
    "EXPIRED"
};

static const char* modeStrings[] = {
    "GENERIC",
    "SLEEP",
    "WAKE"
};

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 1

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::Timer> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(Timer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        Timer* Timer::_instance = nullptr;

        Timer::Timer()
        : PluginHost::JSONRPC()
        {
            Timer::_instance = this;

            Register(TIMER_METHOD_START_TIMER, &Timer::startTimerWrapper, this);
            Register(TIMER_METHOD_CANCEL, &Timer::cancelWrapper, this);
            Register(TIMER_METHOD_SUSPEND, &Timer::suspendWrapper, this);
            Register(TIMER_METHOD_RESUME, &Timer::resumeWrapper, this);
            Register(TIMER_METHOD_GET_TIMER_STATUS, &Timer::getTimerStatusWrapper, this);
            Register(TIMER_METHOD_GET_TIMERS, &Timer::getTimersWrapper, this);

            m_timer.connect(std::bind(&Timer::onTimerCallback, this));
        }

        Timer::~Timer()
        {
        }
        
        void Timer::InitializeIARM()
        {
            Utils::IARM::init();
        }
        void Timer::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected()) { }
        }

        const string Timer::Initialize(PluginHost::IShell * /* service */)
        {
            InitializeIARM();

            return (string());
        }

        void Timer::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            Timer::_instance = nullptr;
        }

        void Timer::checkTimers()
        {
            double minTimeout = 100000;
            for (auto it = m_runningItems.cbegin(); it != m_runningItems.cend(); ++it)
            {
                unsigned int timerId = *it;
                if (timerId < 0 || timerId >= m_timerItems.size())
                {
                    LOGERR("Internal error: wrong timerId");
                    continue;
                }
                if (m_timerItems[timerId].state != RUNNING)
                {
                    LOGERR("Internal error: timer %d has wrong state", timerId);
                    continue;
                }

                std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_timerItems[timerId].lastExpired;
                double timeout =  m_timerItems[timerId].interval - elapsed.count();

                if (!m_timerItems[timerId].reminderSent && m_timerItems[timerId].remindBefore > TIMER_ACCURACY)
                {
                    if (timeout > m_timerItems[timerId].remindBefore)
                    {
                        timeout -= m_timerItems[timerId].remindBefore;
                    }
                    else
                    {
                        sendTimerExpiryReminder(timerId);
                        m_timerItems[timerId].reminderSent = true;
                    }
                }

                if (timeout < minTimeout)
                    minTimeout = timeout;
            }

            if (minTimeout < TIMER_ACCURACY)
                minTimeout = TIMER_ACCURACY;

            if (minTimeout < 100000)
                m_timer.start(int(minTimeout * 1000));
            else
                m_timer.stop();
        }

        void Timer::startTimer(int timerId)
        {
            m_runningItems.push_back(timerId);
            m_timerItems[timerId].state = RUNNING;

            m_timerItems[timerId].lastExpired = std::chrono::system_clock::now();
            m_timerItems[timerId].lastExpiryReminder = std::chrono::system_clock::now();
            m_timerItems[timerId].reminderSent = false;

            checkTimers();
        }

        bool Timer::cancelTimer(int timerId)
        {
            m_timerItems[timerId].state = CANCELED;

            auto it = std::find(m_runningItems.begin(), m_runningItems.end(), timerId);
            if (m_runningItems.end() != it)
            {
                m_runningItems.erase(it);
                m_timer.stop();
                checkTimers();
                return true;
            }

            return false;
        }

        bool Timer::suspendTimer(int timerId)
        {
            m_timerItems[timerId].state = SUSPENDED;

            auto it = std::find(m_runningItems.begin(), m_runningItems.end(), timerId);
            if (m_runningItems.end() != it)
            {
                m_runningItems.erase(it);
                m_timer.stop();
                checkTimers();
                return true;
            }

            return false;
        }

        void Timer::onTimerCallback()
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            std::list <int> itemsToDelete;

            for (auto it = m_runningItems.cbegin(); it != m_runningItems.cend(); ++it)
            {
                unsigned int timerId = *it;
                if (timerId < 0 || timerId >= m_timerItems.size())
                {
                    LOGERR("Internal error: wrong timerId");
                    continue;
                }
                if (m_timerItems[timerId].state != RUNNING)
                {
                    LOGERR("Internal error: timer %d has wrong state", timerId);
                    continue;
                }

                std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_timerItems[timerId].lastExpired;
                double timeout =  m_timerItems[timerId].interval - elapsed.count();

                if (!m_timerItems[timerId].reminderSent && m_timerItems[timerId].remindBefore > TIMER_ACCURACY)
                {
                    if (timeout < m_timerItems[timerId].remindBefore + TIMER_ACCURACY)
                    {
                        sendTimerExpiryReminder(timerId);
                        m_timerItems[timerId].reminderSent = true;
                    }
                }

                if (timeout <= TIMER_ACCURACY)
                {
                    sendTimerExpired(timerId);

                    m_timerItems[timerId].lastExpired = std::chrono::system_clock::now();

                    if (m_timerItems[timerId].repeatInterval > 0)
                    {
                        m_timerItems[timerId].interval = m_timerItems[timerId].repeatInterval;
                    }
                    else
                    {
                        m_timerItems[timerId].state = EXPIRED;
                        itemsToDelete.push_back(timerId);
                    }

                    m_timerItems[timerId].reminderSent = false;
                }
            }

            for (auto delIt = itemsToDelete.cbegin(); delIt != itemsToDelete.cend(); ++delIt)
            {
                auto it = std::find(m_runningItems.begin(), m_runningItems.end(), *delIt);
                if (m_runningItems.end() != it)
                    m_runningItems.erase(it);

            }

            checkTimers();
        }

        void Timer::getTimerStatus(int timerId, JsonObject& output, bool writeTimerId)
        {
            if (writeTimerId)
                output["timerId"] = timerId;

            output["state"] = stateStrings[m_timerItems[timerId].state];
            output["mode"] = modeStrings[m_timerItems[timerId].mode];

            std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_timerItems[timerId].lastExpired;
            double timeRemaining =  m_timerItems[timerId].interval - elapsed.count();

            char buf[256];

            snprintf(buf, sizeof(buf), "%.3f", timeRemaining);
            output["timeRemaining"] = (const char *)buf;

            snprintf(buf, sizeof(buf), "%.3f", m_timerItems[timerId].repeatInterval);
            output["repeatInterval"] = (const char *)buf;

            snprintf(buf, sizeof(buf), "%.3f", m_timerItems[timerId].remindBefore);
            output["remindBefore"] = (const char *)buf;
        }

        uint32_t Timer::startTimerWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if (!parameters.HasLabel("interval"))
            {
                LOGERR("\"interval\" parameter is required");
                returnResponse(false);
            }

            TimerItem item;

            item.state = INITIAL;
            item.interval = std::stod(parameters["interval"].String());

            item.mode = GENERIC;
            if (parameters.HasLabel("mode"))
            {
                if (parameters["mode"].String() == "SLEEP")
                    item.mode = SLEEP;
                else if (parameters["mode"].String() == "WAKE")
                    item.mode = WAKE;
            }

            item.repeatInterval = parameters.HasLabel("repeatInterval") ? std::stod(parameters["repeatInterval"].String()) : 0.0;
            item.remindBefore = parameters.HasLabel("remindBefore") ? std::stod(parameters["remindBefore"].String()) : 0.0;

            m_timerItems.push_back(item);

            startTimer(m_timerItems.size() - 1);
            response["timerId"] = m_timerItems.size() - 1;

            returnResponse(true);
        }

        uint32_t Timer::cancelWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if (!parameters.HasLabel("timerId"))
            {
                LOGERR("\"timerId\" parameter is required");
                returnResponse(false);
            }

            unsigned int timerId;
            getNumberParameter("timerId", timerId);

            if (timerId >=0 && timerId < m_timerItems.size())
            {
                if (CANCELED != m_timerItems[timerId].state)
                {
                    returnResponse(cancelTimer(timerId));
                }

                LOGERR("timer %d is already canceled", timerId);
                returnResponse(false);
            }

            LOGERR("Bad timerId");
            returnResponse(false);
        }

        uint32_t Timer::suspendWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if (!parameters.HasLabel("timerId"))
            {
                LOGERR("\"timerId\" parameter is required");
                returnResponse(false);
            }

            unsigned int timerId;
            getNumberParameter("timerId", timerId);

            if (timerId >=0 && timerId < m_timerItems.size())
            {
                if (RUNNING == m_timerItems[timerId].state)
                {
                    returnResponse(suspendTimer(timerId));
                }

                LOGERR("timer %d is not in running state", timerId);
                returnResponse(false);
            }

            LOGERR("Bad timerId");
            returnResponse(false);
        }

        uint32_t Timer::resumeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if (!parameters.HasLabel("timerId"))
            {
                LOGERR("\"timerId\" parameter is required");
                returnResponse(false);
            }

            unsigned int timerId;
            getNumberParameter("timerId", timerId);

            if (timerId >=0 && timerId < m_timerItems.size())
            {
                if (SUSPENDED == m_timerItems[timerId].state)
                {
                    startTimer(timerId);
                    returnResponse(true);
                }

                LOGERR("timer %d is not in suspended state", timerId);
                returnResponse(false);
            }

            LOGERR("Bad timerId");
            returnResponse(false);
        }

        uint32_t Timer::getTimerStatusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if (!parameters.HasLabel("timerId"))
            {
                LOGERR("\"timerId\" parameter is required");
                returnResponse(false);
            }

            unsigned int timerId;
            getNumberParameter("timerId", timerId);

            if (timerId >= 0 && timerId < m_timerItems.size())
            {
                getTimerStatus(timerId, response);
            }
            else
            {
                LOGERR("Wrong timerId");
                returnResponse(false);
            }

            returnResponse(true);
        }

        uint32_t Timer::getTimersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            JsonArray timers;
            for (unsigned int n = 0; n < m_timerItems.size(); n++)
            {
                JsonObject timer;
                getTimerStatus(n, timer, true);
                timers.Add(timer);
            }

            response["timers"] = timers;

            returnResponse(true);
        }

        void Timer::sendTimerExpired(int timerId)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            if (SLEEP == m_timerItems[timerId].mode || WAKE == m_timerItems[timerId].mode)
            {
                // Taken from power iarm manager
                IARM_Bus_CECMgr_Send_Param_t dataToSend;
                unsigned char buf[] = {0x30, 0x36}; //standby msg, from TUNER to TV

                if (WAKE == m_timerItems[timerId].mode)
                    buf[1] = 0x4; // Image On instead of Standby

                memset(&dataToSend, 0, sizeof(dataToSend));
                dataToSend.length = sizeof(buf);
                memcpy(dataToSend.data, buf, dataToSend.length);
                LOGINFO("Timer send CEC %s", SLEEP == m_timerItems[timerId].mode ? "Standby" : "Wake");
                IARM_Bus_Call(IARM_BUS_CECMGR_NAME,IARM_BUS_CECMGR_API_Send,(void *)&dataToSend, sizeof(dataToSend));
            }
#endif
            JsonObject params;
            params["timerId"] = timerId;
            params["mode"] = modeStrings[m_timerItems[timerId].mode];
            params["status"] = 0;
            sendNotify(TIMER_EVT_TIMER_EXPIRED, params);
        }

        void Timer::sendTimerExpiryReminder(int timerId)
        {
            JsonObject params;
            params["timerId"] = timerId;
            params["mode"] = modeStrings[m_timerItems[timerId].mode];

            std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_timerItems[timerId].lastExpired;

            params["timeRemaining"] = (int)(m_timerItems[timerId].interval - elapsed.count() + 0.5);
            sendNotify(TIMER_EVT_TIMER_EXPIRY_REMINDER, params);
        }
    } // namespace Plugin
} // namespace WPEFramework

