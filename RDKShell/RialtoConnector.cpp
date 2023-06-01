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

#include <iostream>
#include <cstdlib>
#include "RialtoConnector.h"

extern char **environ;

namespace WPEFramework
{
    namespace
    {
        std::list<std::string> getEnvironmentVariables()
        {
            std::list<std::string> environmentVariables;
            char **envList = environ;
            
            for(;*envList;envList++)
            {
                environmentVariables.emplace_back(*envList);
            }             
           return environmentVariables;
        }
    } //anonymous namespace to contain private function.
 
    void RialtoConnector::initialize()
    {
        LOGWARN(" Rialto Bridge version 1.1");
        firebolt::rialto::common::ServerManagerConfig config;
        config.sessionServerEnvVars = getEnvironmentVariables();
        m_serverManagerService = create(shared_from_this(), config);
        isInitialized = true;
    }
    bool RialtoConnector::createAppSession(const std::string &callsign, const std::string &displayName, const std::string &appId)
    {
        LOGINFO("Creating app session with callsign : '%s', display name : '%s', appid : '%s'", callsign.c_str(), displayName.c_str(), appId.c_str());

        firebolt::rialto::common::AppConfig config = {appId, displayName};
        return m_serverManagerService->initiateApplication(callsign,
                                                           RialtoServerStates::ACTIVE,
                                                           config);
    }
    bool RialtoConnector::resumeSession(const std::string &callsign)
    {
        if (RialtoServerStates::INACTIVE == getCurrentAppState(callsign))
            return m_serverManagerService->changeSessionServerState(callsign,
                                                                    RialtoServerStates::ACTIVE);
        return false;
    }
    bool RialtoConnector::suspendSession(const std::string &callsign)
    {
        if (RialtoServerStates::ACTIVE == getCurrentAppState(callsign))
            return m_serverManagerService->changeSessionServerState(callsign,
                                                                    RialtoServerStates::INACTIVE);
        return false;
    }
    const RialtoServerStates RialtoConnector::getCurrentAppState(const std::string &callsign)
    {
        auto state = appStateMap.find(callsign);
        if (state != appStateMap.end())
        {
            // If the state is not inactive, we have a problem.
            return state->second;
        }
        return RialtoServerStates::ERROR;
    }
    bool RialtoConnector::deactivateSession(const std::string &callsign)
    {
        LOGINFO("Deactiving app %s", callsign.c_str());
        RialtoServerStates state = getCurrentAppState(callsign);
        if (RialtoServerStates::ACTIVE == state ||
            RialtoServerStates::INACTIVE == state)
            return m_serverManagerService->changeSessionServerState(callsign,
                                                                    RialtoServerStates::NOT_RUNNING);
        LOGINFO("Rialto server is not in active or running state. ");
        return false;
    }
    void RialtoConnector::stateChanged(const std::string &appId,
                                       const RialtoServerStates &state)
    {
        {
            std::lock_guard<std::mutex> lockguard(m_stateMutex);
            appStateMap[appId] = state;
        }
        LOGINFO("[RialtoConnector::stateChanged] State change announced for %s to %d, isActive ? %d ", appId.c_str(),
                static_cast<int>(state), (state == RialtoServerStates::ACTIVE));
        m_stateCond.notify_one();
    }

    // wait until socket is in given state
    // return true when state set, false on timeout
    bool RialtoConnector::waitForStateChange(const std::string& appId, const RialtoServerStates& state, int timeoutMillis)
    {
        bool status = false;
        std::unique_lock<std::mutex> lock(m_stateMutex);
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::milliseconds(timeoutMillis);

        while (std::chrono::steady_clock::now() < endTime)
        {
            if (appStateMap[appId] == state)
            {
                status = true;
                break;
            }

            auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - std::chrono::steady_clock::now());
            if (remainingTime.count() <= 0)
                break;

            m_stateCond.wait_for(lock, remainingTime);
        }

        return status;
    }

} // namespace WPEFramework
