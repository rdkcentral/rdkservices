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

#pragma once

#include "RialtoConnector.h"

#include <string>
#include <set>
#include <mutex>

namespace WPEFramework {
namespace Plugin {

/**
 * RialtoAppsSessionManager simplifies management of Rialto sessions associated
 * with selected application instances.
 */
class RialtoAppsSessionManager {
public:
    /**
     * Constructor
     *
     * @param connector A connector object.
     * @param rialtoSocketDir A directory under which Rialto sockets will be created.
     *  No sockets will be created if the value is empty.
     * @param rialtoApps Types of applications for which Rialto sockets will be created.
     * @param timeoutMs Timeout in milliseconds for all waiting operations.
     */
    RialtoAppsSessionManager(
        std::shared_ptr<RialtoConnector> connector,
        std::string rialtoSocketDir,
        std::set<std::string> rialtoApps,
        int timeoutMs);

    /**
     * Checks if Rialto socket should be created for given application type.
     *
     * @param app An application type.
     *
     * @return true if Rialto socket should be created, false otherwise.
     */
    bool usesRialto(const std::string& app);

    /**
     * Creates a Rialto session for an application instance.
     *
     * @param app An application type.
     * @param callsign A unique signature of an application instance.
     * @param rialtoSocketName The name of the socket that should be created.
     * @param displayName The name of the display that should be used by the Rialto session.
     *
     * @return true if the Rialto session was created or the application of specified type do not use Rialto,
     *  false in case when it was not possible to create Rialto session.
     */
    bool createRialtoSessionAndWait(
        const std::string& app,
        const std::string& callsign,
        const std::string& rialtoSocketName,
        const std::string& displayName);

    /**
     * Destroys the Rialto session associated with the application instance.
     *
     * @param callsign A unique signature of an application instance.
     *
     * @return true if the Rialto session was destroyed or there was no Rialto session associated
     *  with the application instance, false on error.
     */
    bool destroyRialtoSession(const std::string& callsign);

    /**
     * Suspends the Rialto session associated with the application instance.
     *
     * @param callsign A unique signature of an application instance.
     *
     * @return true if the Rialto session was suspended or there was no Rialto session associated
     *  with the application instance, false on error.
     */
    bool suspendRialtoSessionAndWait(const std::string& callsign);

    /**
     * Resumes the Rialto session associated with the application instance.
     *
     * @param callsign A unique signature of an application instance.
     *
     * @return true if the Rialto session was resumed or there was no Rialto session associated
     *  with the application instance, false on error.
     */
    bool resumeRialtoSessionAndWait(const std::string& callsign);

private:
    std::shared_ptr<RialtoConnector> mConnector;
    std::string mRialtoSocketDir;
    std::set<std::string> mRialtoApps;
    int mTimeoutMs;
    std::set<std::string> mSessions;
    std::mutex mMutex;

    std::string makeRialtoSocketPath(const std::string& rialtoSocketName);
    void insertSession(const std::string& callsign);
    bool hasSession(const std::string& callsign);
    bool removeSession(const std::string& callsign);
};

}
}
