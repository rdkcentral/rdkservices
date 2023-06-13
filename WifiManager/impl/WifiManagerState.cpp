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
#include "UtilsJsonRpc.h"

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

WifiManagerState::WifiManagerState()
{
    m_useWifiStateCache = false;
    m_wifiStateCache = WifiState::FAILED;
    m_ConnectedSSIDCache = "";
    m_ConnectedBSSIDCache = "";
    m_ConnectedSecurityModeCache = 0;
}

WifiManagerState::~WifiManagerState()
{
}


void WifiManagerState::setWifiStateCache(bool value,WifiState Cstate)
{
    m_useWifiStateCache = value;
    m_wifiStateCache = Cstate;
}

uint32_t WifiManagerState::getCurrentState(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_WiFiSrvMgr_Param_t param;

    memset(&param, 0, sizeof(param));

    if (!m_useWifiStateCache)
    {
        if(IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getCurrentState, (void *)&param, sizeof(param)))
        {
            setWifiStateCache(true,(to_wifi_state(param.data.wifiStatus)));
        }
    }
    response["state"] = static_cast<int>(m_wifiStateCache);
    returnResponse(retVal == IARM_RESULT_SUCCESS);
}

uint32_t WifiManagerState::getConnectedSSID(const JsonObject &parameters, JsonObject &response) const
{
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

    IARM_BUS_NetSrvMgr_Iface_EventData_t param;
    memset(&param, 0, sizeof(param));

    strncpy(param.setInterface, "WIFI", INTERFACE_SIZE - 1);
    param.isInterfaceEnabled = parameters["enable"].Boolean();

    // disables wifi interface when ethernet interface is active
    IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_NETSRVMGR_API_setInterfaceEnabled, (void *)&param, sizeof(param));

    // Update wifi state cache if wifi interface was disabled
    if (retVal == IARM_RESULT_SUCCESS && !param.isInterfaceEnabled) {
        setWifiStateCache(true, WifiState::DISABLED);
    }

    returnResponse(retVal == IARM_RESULT_SUCCESS);
}

uint32_t WifiManagerState::getSupportedSecurityModes(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();

    JsonObject security_modes;

    //from the SsidSecurity enum in wifiSrvMgrIarmIf.h
    security_modes["NET_WIFI_SECURITY_NONE"]                 = (int)NET_WIFI_SECURITY_NONE;
    security_modes["NET_WIFI_SECURITY_WEP_64"]               = (int)NET_WIFI_SECURITY_WEP_64;
    security_modes["NET_WIFI_SECURITY_WEP_128"]              = (int)NET_WIFI_SECURITY_WEP_128;
    security_modes["NET_WIFI_SECURITY_WPA_PSK_TKIP"]         = (int)NET_WIFI_SECURITY_WPA_PSK_TKIP;
    security_modes["NET_WIFI_SECURITY_WPA_PSK_AES"]          = (int)NET_WIFI_SECURITY_WPA_PSK_AES;
    security_modes["NET_WIFI_SECURITY_WPA2_PSK_TKIP"]        = (int)NET_WIFI_SECURITY_WPA2_PSK_TKIP;
    security_modes["NET_WIFI_SECURITY_WPA2_PSK_AES"]         = (int)NET_WIFI_SECURITY_WPA2_PSK_AES;
    security_modes["NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP"]  = (int)NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP;
    security_modes["NET_WIFI_SECURITY_WPA_ENTERPRISE_AES"]   = (int)NET_WIFI_SECURITY_WPA_ENTERPRISE_AES;
    security_modes["NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP"] = (int)NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP;
    security_modes["NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES"]  = (int)NET_WIFI_SECURITY_WPA2_ENTERPRISE_AES;
    security_modes["NET_WIFI_SECURITY_WPA_WPA2_PSK"]         = (int)NET_WIFI_SECURITY_WPA_WPA2_PSK;
    security_modes["NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE"]  = (int)NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE;
    security_modes["NET_WIFI_SECURITY_WPA3_PSK_AES"]         = (int)NET_WIFI_SECURITY_WPA3_PSK_AES;
    security_modes["NET_WIFI_SECURITY_WPA3_SAE"]             = (int)NET_WIFI_SECURITY_WPA3_SAE;
    security_modes["NET_WIFI_SECURITY_NOT_SUPPORTED"]        = (int)NET_WIFI_SECURITY_NOT_SUPPORTED;

    response["security_modes"] = security_modes;

    returnResponse(true);
}
