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

#pragma once

#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include "rialto/ServerManagerServiceFactory.h"

namespace WPEFramework
{
    using namespace rialto::servermanager::service;
    // Shadowing to keep Rialto contained in this
    typedef firebolt::rialto::common::SessionServerState RialtoServerStates;

    class RialtoConnector : public IStateObserver, public std::enable_shared_from_this<RialtoConnector>
    {
    public:
        RialtoConnector() : isInitialized(false) {}
        virtual ~RialtoConnector() = default;
        void initialize(std::string &, const std::string &);
        bool initialized() { return isInitialized; }
        bool waitForStateChange(const std::string &appid, const RialtoServerStates &state, int timeout);
        bool createAppSession(const std::string &callsign, const std::string &displayName, const std::string &appId);
        bool resumeSession(const std::string &callsign);
        bool suspendSession(const std::string &callsign);

        bool deactivateSession(const std::string &callsign);
        void stateChanged(const std::string &appId, const RialtoServerStates &state) override;

        RialtoConnector(const RialtoConnector &) = delete;            // No copying
        RialtoConnector &operator=(const RialtoConnector &) = delete; // No assignment

    private:
        bool isInitialized;
        std::mutex m_stateMutex;
        std::condition_variable m_stateCond;
        std::unique_ptr<IServerManagerService> m_serverManagerService;
        std::map<std::string, RialtoServerStates> appStateMap;

        const RialtoServerStates getCurrentAppState(const std::string &callsign);
    };
} // namespace WPEFramework
