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

#define BUFFER_SIZE 512
#define Command1 "wpa_cli status"
#define Command2 "wpa_cli signal_poll"

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

    SsidSecurity getSecurityModeValue(std::string str)
    {
        if(!str.compare("NET_WIFI_SECURITY_NONE"))
            return SsidSecurity::NET_WIFI_SECURITY_NONE;
        else if(!str.compare("NET_WIFI_SECURITY_WEP_64"))
            return SsidSecurity::NET_WIFI_SECURITY_WEP_64;
        else if(!str.compare("NET_WIFI_SECURITY_WEP_128"))
            return SsidSecurity::NET_WIFI_SECURITY_WEP_128;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_PSK_TKIP"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_PSK_TKIP;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_PSK_AES"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_PSK_AES;
        else if(!str.compare("NET_WIFI_SECURITY_WPA2_PSK_TKIP"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA2_PSK_TKIP;
        else if(!str.compare("NET_WIFI_SECURITY_WPA2_PSK_AES"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA2_PSK_AES;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_ENTERPRISE_TKIP;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_ENTERPRISE_AES"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_ENTERPRISE_AES;
        else if(!str.compare("NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA2_ENTERPRISE_TKIP;
        else if(!str.compare("NET_WIFI_SECURITY_WPA2_PSK_AES"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA2_PSK_AES;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_WPA2_PSK"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_WPA2_PSK;
        else if(!str.compare("NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA_WPA2_ENTERPRISE;
        else if(!str.compare("NET_WIFI_SECURITY_WPA3_PSK_AES"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA3_PSK_AES;
        else if(!str.compare("NET_WIFI_SECURITY_WPA3_SAE"))
            return SsidSecurity::NET_WIFI_SECURITY_WPA3_SAE;
        else
            return SsidSecurity::NET_WIFI_SECURITY_NOT_SUPPORTED;
    }
}

WifiManagerState::WifiManagerState()
{
    m_useWifiStateCache = false;
    m_wifiStateCache = WifiState::FAILED;
    m_useWifiConnectedCache = false;
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

void WifiManagerState::resetWifiStateConnectedCache(bool value)
{
    m_useWifiConnectedCache = value;
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

std::map<std::string, std::string> WifiManagerState::retrieveValues(const char *command, char *output_buffer, size_t output_buffer_size)
{
    std::string key, value;
    std::map<std::string, std::string> MyMap;

    FILE *fp = popen(command, "r");
    if (!fp)
    {
        LOGERR("Failed in getting output from command %s \n",command);
    }
    while(fgets(output_buffer, output_buffer_size, fp) != NULL)
    {
        std::istringstream mystream(output_buffer);
        if(std::getline(std::getline(mystream, key, '=') >> std::ws, value))
            MyMap[key] = value;
    }
    pclose(fp);

    return MyMap;
}

bool WifiManagerState::getConnectedSSID(const JsonObject &parameters, JsonObject &response)
{
    WiFiConnectedSSIDInfo_t param;
    char buff[BUFFER_SIZE] = {'\0'};
    int phyrate, noise, rssi,freq,avgRssi;
    bool result = false;
    std::string security_mode = "";
    std::string auth = "";
    std::string encryption = "";
    std::string wifi_wpa_state = "";

    memset(&param, '\0', sizeof(param));

    if(!m_useWifiConnectedCache)
    {
        auto list = WifiManagerState::retrieveValues(Command1, buff, sizeof (buff));

        if (!list.empty())
        {
            for(auto it = list.cbegin(); it != list.cend(); ++it)
            {
                if (it->first == "wpa_state")
                {
                    wifi_wpa_state = it->second;
                }
                else if (it->first == "ssid")
                {
                    m_ConnectedSSIDCache = it->second;
                }
                else if (it->first == "bssid")
                {
                    m_ConnectedBSSIDCache = it->second;
                }
                else if (it->first == "pairwise_cipher")
                {
                    encryption = it->second.c_str();
                    if(!encryption.compare("TKIP"))
                    {
                        encryption.clear();
                        encryption = "_TKIP";
                    }
                    if(!encryption.compare("CCMP"))
                    {
                        encryption.clear();
                        encryption = "_AES";
                    }
                }
                else if (it->first == "key_mgmt")
                {
                    auth = it->second.c_str();
                    std::replace( auth.begin(), auth.end(), '-', '_');
                }
            }
            /* if Wifi is Disconnected ,getConnectedSSID api response becomes zero and empty string */
            if(wifi_wpa_state != "COMPLETED")
            {
                result = true;
                response["ssid"] = "";
                response["bssid"] = "";
                response["rate"] = to_string(param.rate);
                response["noise"] = to_string(param.noise);
                response["security"] = m_ConnectedSecurityModeCache;
                response["signalStrength"] = to_string(param.signalStrength);
                response["frequency"] = to_string(((float)param.frequency)/1000);

                return result;
             }
             else
             {
                 if (auth.empty())
                     security_mode = "NET_WIFI_SECURITY_NONE";
                 else
                     security_mode = "NET_WIFI_SECURITY_" + auth + encryption;

                 m_ConnectedSecurityModeCache = static_cast<int>(getSecurityModeValue(security_mode));
                 result = true;
                 m_useWifiConnectedCache = true;
             }
        }
        else
        {
            LOGERR("Command failed to execute:%s\n",Command1);
            result = false;
        }
    }

    auto clist = WifiManagerState::retrieveValues(Command2, buff, sizeof (buff));

    if (!clist.empty())
    {
        for(auto it = clist.cbegin(); it != clist.cend(); ++it)
        {
            if (it->first == "LINKSPEED") // phyRate
            {
                phyrate = atoi(it->second.c_str());
                param.rate = phyrate;
            }
            else if (it->first == "RSSI")
            {
                rssi = atoi(it->second.c_str());
                param.signalStrength  = rssi;
            }
            else if (it->first == "NOISE")
            {
                noise = atoi(it->second.c_str());
                param.noise  = noise;
            }
            else if (it->first == "FREQUENCY")
            {
                freq = atoi(it->second.c_str());
                param.frequency  = freq;
            }
        }
        result = true;
    }
    else
    {
        LOGERR("Command failed to execute:%s\n",Command2);
        result = false;
    }

    if(result == true)
    {
        response["ssid"] = m_ConnectedSSIDCache;
        response["bssid"] = m_ConnectedBSSIDCache;
        response["rate"] = to_string(param.rate);
        response["noise"] = to_string(param.noise);
        response["security"] = m_ConnectedSecurityModeCache;
        response["signalStrength"] = to_string(param.signalStrength);
        response["frequency"] = to_string(((float)param.frequency)/1000);
    }

    return result;
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
