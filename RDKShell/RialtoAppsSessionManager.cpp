/*
* Copyright 2024 Metrological
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

#include "RialtoAppsSessionManager.h"

using namespace WPEFramework::Plugin;

RialtoAppsSessionManager::RialtoAppsSessionManager(
    std::shared_ptr<RialtoConnector> connector,
    std::string rialtoSocketDir,
    std::set<std::string> rialtoApps,
    int timeoutMs) :
    mConnector(std::move(connector)),
    mRialtoSocketDir(std::move(rialtoSocketDir)),
    mRialtoApps(std::move(rialtoApps)),
    mTimeoutMs(timeoutMs) {

    if (mRialtoSocketDir.empty()) {
        LOGWARN("RialtoAppsSessionManager is disabled because an empty Rialto socket dir was specified");
    }
}

std::string RialtoAppsSessionManager::makeRialtoSocketPath(const std::string& rialtoSocketName) {
    return mRialtoSocketDir + "/" + rialtoSocketName;
}

void RialtoAppsSessionManager::insertSession(const std::string& callsign) {
    std::lock_guard<std::mutex> l(mSessionsMutex);
    mSessions.insert(callsign);
}

bool RialtoAppsSessionManager::hasSession(const std::string& callsign) {
    std::lock_guard<std::mutex> l(mSessionsMutex);
    return mSessions.find(callsign) != mSessions.end();
}

bool RialtoAppsSessionManager::removeSession(const std::string& callsign) {
    std::lock_guard<std::mutex> l(mSessionsMutex);
    return mSessions.erase(callsign) == 1;
}

bool RialtoAppsSessionManager::usesRialto(const std::string& app) {
    return mRialtoSocketDir.empty() == false && mRialtoApps.find(app) != mRialtoApps.end();
}

bool RialtoAppsSessionManager::createRialtoSessionAndWait(
    const std::string& app,
    const std::string& callsign,
    const std::string& rialtoSocketName,
    const std::string& displayName) {

    if (usesRialto(app) == false) return true;

    std::string rialtoSocketPath = makeRialtoSocketPath(rialtoSocketName);

    if (!mConnector->initialized()) {
        LOGINFO("Initializing connector");
        mConnector->initialize();
    }

    LOGINFO("[%s] Creating session %s %s %s %s", callsign.c_str(), app.c_str(),
        callsign.c_str(), displayName.c_str(), rialtoSocketPath.c_str());

    if (mConnector->createAppSession(callsign, displayName, rialtoSocketPath) == false) {
        LOGWARN("[%s] createAppSession() failed", callsign.c_str());
        return false;
    }

    if (mConnector->waitForStateChange(callsign, RialtoServerStates::ACTIVE, mTimeoutMs) == false) {
        LOGWARN("[%s] ACTIVE state not reached in %d ms", callsign.c_str(), mTimeoutMs);
        mConnector->deactivateSession(callsign);
        return false;
    }

    insertSession(callsign);

    LOGINFO("[%s] Session created", callsign.c_str());
    return true;
}

bool RialtoAppsSessionManager::destroyRialtoSession(const std::string& callsign) {
    if (removeSession(callsign) == false) return true;

    if (mConnector->deactivateSession(callsign) == false) {
        LOGWARN("[%s] deactivateSession() failed", callsign.c_str());
        return false;
    }

    LOGINFO("[%s] Session destroyed", callsign.c_str());
    return true;
}

bool RialtoAppsSessionManager::suspendRialtoSessionAndWait(const std::string& callsign) {
    if (hasSession(callsign) == false) return true;

    if (mConnector->suspendSession(callsign) == false) {
        LOGWARN("[%s] suspendSession() failed", callsign.c_str());
        return false;
    }

    if (mConnector->waitForStateChange(callsign, RialtoServerStates::INACTIVE, mTimeoutMs) == false) {
        LOGWARN("[%s] INACTIVE state not reached in %d ms", callsign.c_str(), mTimeoutMs);
        return false;
    }

    LOGINFO("[%s] Session suspended", callsign.c_str());
    return true;
}

bool RialtoAppsSessionManager::resumeRialtoSessionAndWait(const std::string& callsign) {
    if (hasSession(callsign) == false) return true;

    if (mConnector->resumeSession(callsign) == false) {
        LOGWARN("[%s] resumeSession() failed", callsign.c_str());
        return false;
    }

    if (mConnector->waitForStateChange(callsign, RialtoServerStates::ACTIVE, mTimeoutMs) == false) {
        LOGWARN("[%s] ACTIVE state not reached in %d ms", callsign.c_str(), mTimeoutMs);
        return false;
    }

    LOGINFO("[%s] Session resumed", callsign.c_str());
    return true;
}
