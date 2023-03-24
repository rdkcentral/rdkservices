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

/**
 * WifiManager implementation related to performing scans of wireless networks.
 *
 */

#include "WifiManagerScan.h"
// #include "../WifiManager.h" // Need access to WifiManager::getInstance so can't use 'WifiManagerInterface.h'
#include "UtilsJsonRpc.h"

// std
#include <sstream>
#include <regex>


using namespace WPEFramework;
using namespace WPEFramework::Plugin;

/**
 * \brief Register event handlers.
 *
 */
std::string WifiManagerScan::Initialize(PluginHost::IShell*)
{
    LOGINFO("initializing");
    // Successful
    return string();
}

/**
 * \brief Unregister event handlers.
 *
 */
void WifiManagerScan::Deinitialize(PluginHost::IShell* service)
{
    LOGINFO("deinitializing");
}

/**
 * \brief Get the available access points asynchronously.
 *
 * The results are published on via the "onAvailableSSIDs" event.
 *
 * \param parameters        Must include 'incremental'. Optionally includes 'ssid' and/or 'frequency'.
 * \param[out] response     Always includes 'success' if successful.
 * \return                  A code indicating success.
 *
 */

uint32_t WifiManagerScan::startScan(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD();
    returnResponse(false);
}

/**
 * \brief Cancel a current incremental asynchronous scan started by 'getAvailableSSIDsAsyncIncr'.
 *
 * \param parameters    Ignored.
 * \param[out] response Will contain 'success' key.
 * \return Error code.
 *
 */
uint32_t WifiManagerScan::stopScan(const JsonObject& parameters, JsonObject& response)
{
    returnResponse(false);
}
