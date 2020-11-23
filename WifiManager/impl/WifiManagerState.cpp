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

#include "WifiManagerState.h"

#include "wifiSrvMgrIarmIf.h"
#include "netsrvmgrIarm.h"
#include "libIBus.h"
#include "utils.h"


using namespace WPEFramework::Plugin;
using namespace std;

namespace {
    WifiState to_wifi_state(WiFiStatusCode_t code) {
        switch(code){
            case WIFI_UNINSTALLED: return WifiState::UNINSTALLED;
            case WIFI_DISABLED: return WifiState::DISABLED;
            case WIFI_DISCONNECTED: return WifiState::DISCONNECTED;
            case WIFI_PAIRING: return WifiState::PAIRING;
            case WIFI_CONNECTING: return WifiState::CONNECTING;
            case WIFI_CONNECTED: return WifiState::CONNECTED;
            case WIFI_FAILED: return WifiState::FAILED;
        }
        return WifiState::FAILED;
    }
}

uint32_t WifiManagerState::getCurrentState(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD();
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_WiFiSrvMgr_Param_t param;
    WiFiStatusCode_t wifiStatusCode = (WiFiStatusCode_t)-1;

    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getCurrentState, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS)
    {
        wifiStatusCode = param.data.wifiStatus;
    }

    response["state"] = static_cast<int>(to_wifi_state(wifiStatusCode));
    returnResponse(retVal == IARM_RESULT_SUCCESS);
}

uint32_t WifiManagerState::getConnectedSSID(const JsonObject &parameters, JsonObject &response) const
{
    LOGINFOMETHOD();
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_WiFiSrvMgr_Param_t param;

    memset(&param, 0, sizeof(param));

    retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getConnectedSSID, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS)
    {
        auto &connectedSsid = param.data.getConnectedSSID;
        response["ssid"] = string(connectedSsid.ssid);
        response["bssid"] = string(connectedSsid.bssid);
        response["rate"] = to_string(connectedSsid.rate);
        response["noise"] = to_string(connectedSsid.noise);
        response["security"] = to_string(connectedSsid.securityMode);
        response["signalStrength"] = to_string(connectedSsid.signalStrength);
        response["frequency"] = to_string(((float)connectedSsid.frequency)/1000);
    }

    returnResponse(retVal == IARM_RESULT_SUCCESS);
}

uint32_t WifiManagerState::setEnabled(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    returnIfBooleanParamNotFound(parameters, "enable");
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;

    IARM_BUS_NetSrvMgr_Iface_EventData_t param;
    memset(&param, 0, sizeof(param));

    strncpy(param.setInterface, "WIFI", INTERFACE_SIZE - 1);

    bool enabled = parameters["enable"].Boolean();
    if(enabled)
    {
        param.isInterfaceEnabled = enabled;
    }
    else
    {
        param.isInterfaceEnabled = enabled;
    }
        retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled, (void *)&param, sizeof(param));

    returnResponse(retVal == IARM_RESULT_SUCCESS);
}
