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

#define LOGINFO(fmt, ...) do { fprintf(stderr, " INFO [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define LOGWARN(fmt, ...) do { fprintf(stderr, " WARN [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); fflush(stderr); } while (0)

namespace WPEFramework
{
    namespace
    {
        std::list<std::string> getEnvironmentVariables(std::string & envVarsStr, const std::string & debugLevel)
        {
            std::list<std::string> environmentVariables;
            if(!envVarsStr.empty())
            {
                LOGINFO("SESSION_SERVER_ENV_VARS returned %s", envVarsStr.c_str());
                size_t pos = 0;
                while ((pos = envVarsStr.find(";")) != std::string::npos)
                {
                    environmentVariables.emplace_back(envVarsStr.substr(0, pos));
                    envVarsStr.erase(0, pos + 1);
                }
                environmentVariables.emplace_back(envVarsStr);
            }
            else
                LOGINFO("SESSION_SERVER_ENV_VARS returned empty");
            return environmentVariables;
            if(!debugLevel.empty())
            {
                setenv("RIALTO_DEBUG",debugLevel.c_str(),1);
            }
        }
    } //anonymous namespace to contain private function.
 
    void RialtoConnector::initialize(std::string & env,const std::string & debug)
    {
        LOGWARN(" Rialto Bridge version 1.0");
        std::list<std::string> envList = getEnvironmentVariables(env,debug);
        m_serverManagerService = create(shared_from_this(), envList);
        isInitialized = true;
    }
    bool RialtoConnector::createAppSession(const std::string &callsign, const std::string &displayName, const std::string &appId)
    {
        LOGINFO("Creating app session with callsign %s, display name %s, appid %s", callsign.c_str(), displayName.c_str(), appId.c_str());

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
        m_stateCond.notify_one();

        LOGINFO("[RialtoConnector::stateChanged] State change announced for %s, isActive ? %d ", appId.c_str(), (state == RialtoServerStates::ACTIVE));
    }

    bool RialtoConnector::waitForStateChange(const std::string &appId, const RialtoServerStates &state, int timeout)
    {
        bool status = false;
        std::unique_lock<std::mutex> lock(m_stateMutex);
        if (appStateMap[appId] == state)
        {
            status = true;
        }
        else
        {
            if (m_stateCond.wait_for(lock, std::chrono::milliseconds(timeout)) != std::cv_status::timeout)
            {
                if (appStateMap[appId] == state)
                    status = true;
            }
        }
        return status;
    }
} // namespace WPEFramework
