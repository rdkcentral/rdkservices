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
 * WifiManager implementation related to events.
 *
 */

#include "WifiManagerEvents.h"
#include "../WifiManager.h" // Need access to WifiManager::getInstance so can't use 'WifiManagerInterface.h'
#include "UtilsIarm.h"

// RDK
#include "rdk/iarmbus/libIBus.h"
#include "wifiSrvMgrIarmIf.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;

/**
 * \brief Register event handlers.
 *
 */
std::string WifiManagerEvents::Initialize(PluginHost::IShell*)
{
    LOGINFO("initializing");

    // Register event handlers for wireless scan related events
    IARM_Result_t res;
    IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged, WifiManagerEvents::iarmEventHandler));
    IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onError, WifiManagerEvents::iarmEventHandler));
    IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged, WifiManagerEvents::iarmEventHandler));

    // Successful
    return string();
}

/**
 * \brief Unregister event handlers.
 *
 */
void WifiManagerEvents::Deinitialize(PluginHost::IShell*)
{
    LOGINFO("deinitializing");

    IARM_Result_t res;
    IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged));
    IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onError));
    IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged));
}

/**
 * \brief Event handler for the IARM bus.
 *
 * Relevant events are converted into WifiManager notifications.
 *
 * \param owner   The issuer of the event, should be network server manager.
 * \param eventId The event id.
 * \param data    Data associated with the event.
 * \param len     Length of 'data'.
 *
 */
void WifiManagerEvents::iarmEventHandler(char const* owner, IARM_EventId_t eventId, void* data, size_t len)
{
    // Only care about events originating from the network server manager
    if (strcmp(owner, IARM_BUS_NM_SRV_MGR_NAME) != 0)
        return;

    switch (eventId) {
        case IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged:
        {
            IARM_BUS_WiFiSrvMgr_EventData_t* eventData = reinterpret_cast<IARM_BUS_WiFiSrvMgr_EventData_t *>(data);
            LOGINFO("Event IARM_BUS_WIFI_MGR_EVENT_onWIFIStateChanged received; state=%d", eventData->data.wifiStateChange.state);

            // Hardcode 'isLNF' for the moment
            WifiManager::getInstance().onWIFIStateChanged(WifiState(eventData->data.wifiStateChange.state), false);
        }
        break;

        case IARM_BUS_WIFI_MGR_EVENT_onError:
        {
            IARM_BUS_WiFiSrvMgr_EventData_t* eventData = reinterpret_cast<IARM_BUS_WiFiSrvMgr_EventData_t *>(data);
            LOGINFO("Event IARM_BUS_WIFI_MGR_EVENT_onError received; code=%d", eventData->data.wifiError.code);

            WifiManager::getInstance().onError(ErrorCode(eventData->data.wifiError.code));
        }
        break;

        case IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged:
        {
            LOGINFO("Event IARM_BUS_WIFI_MGR_EVENT_onSSIDsChanged received");

            WifiManager::getInstance().onSSIDsChanged();
        }
        break;
    }
}
