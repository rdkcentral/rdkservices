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

#include "WifiManagerWPS.h"
#include "UtilsJsonRpc.h"
#include "libIBus.h"
#include "wifiSrvMgrIarmIf.h"

namespace WPEFramework
{
    namespace Plugin
    {
        WifiManagerWPS::WifiManagerWPS()
        {
            m_useCachePairedSSID = false;
            m_cachePairedSSID = "";
            m_cachePairedBSSID = "";
        }

        WifiManagerWPS::~WifiManagerWPS()
        {
        }

        void WifiManagerWPS::updateWifiWPSCache(bool value)
        {
            m_useCachePairedSSID = value;
        }

        uint32_t WifiManagerWPS::initiateWPSPairing(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_initiateWPSPairing, (void *)&param, sizeof(param));
            LOGINFO("[%s] : retVal:%d status:%d", IARM_BUS_WIFI_MGR_API_initiateWPSPairing, retVal, param.status);

            response["result"] = string();
            returnResponse(retVal == IARM_RESULT_SUCCESS);
        }

        uint32_t WifiManagerWPS::initiateWPSPairing2(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();

            if (!parameters.HasLabel("method"))
            {
                LOGERR("parameter 'method' is missing.");
                returnResponse(false);
            }

            string method = "";
            IARM_Bus_WiFiSrvMgr_WPS_Parameters_t wps_parameters;

            getStringParameter("method", method);
            if (method == "PBC")
            {
                wps_parameters.pbc = true;
            }
            else if (method == "PIN")
            {
                if (!parameters.HasLabel("wps_pin"))
                {
                    LOGERR("parameter 'wps_pin' is missing");
                    returnResponse(false);
                }
                std::string wps_pin;
                getStringParameter("wps_pin", wps_pin);
		if (wps_pin.length() != 8)
                {
                    LOGERR("parameter 'wps_pin' should be 8 digits");
                    returnResponse(false);
                }
                snprintf(wps_parameters.pin, sizeof(wps_parameters.pin), "%s", wps_pin.c_str());
                wps_parameters.pbc = false;
            }
            else if (method == "SERIALIZED_PIN")
            {
                snprintf(wps_parameters.pin, sizeof(wps_parameters.pin), "xxxxxxxx");
                wps_parameters.pbc = false;
            }
            else
            {
                LOGERR("parameter 'method' is invalid.");
                returnResponse(false);
            }

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME,
                    IARM_BUS_WIFI_MGR_API_initiateWPSPairing2,
                    (void *)&wps_parameters, sizeof(wps_parameters));
            LOGINFO("[%s] : retVal:%d status:%d",
                    IARM_BUS_WIFI_MGR_API_initiateWPSPairing2, retVal, wps_parameters.status);

            if (IARM_RESULT_SUCCESS != retVal)
                returnResponse(false);

            response["result"] = wps_parameters.status ? 0 : 1;
            if (method == "PIN" || method == "SERIALIZED_PIN")
                response["pin"] = string(wps_parameters.pin);
            returnResponse(true);
        }

        uint32_t WifiManagerWPS::cancelWPSPairing(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_cancelWPSPairing, (void *)&param, sizeof(param));
            LOGINFO("[%s] : retVal:%d status:%d", IARM_BUS_WIFI_MGR_API_cancelWPSPairing, retVal, param.status);

            response["result"] = string();
            returnResponse(retVal == IARM_RESULT_SUCCESS);
        }

        uint32_t WifiManagerWPS::saveSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();

            returnIfStringParamNotFound(parameters, "ssid");
            returnIfStringParamNotFound(parameters, "passphrase");
            returnIfNumberParamNotFound(parameters, "securityMode");

            bool saved = false;
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            strncpy(param.data.connect.ssid, parameters["ssid"].String().c_str(), SSID_SIZE - 1);
            strncpy(param.data.connect.passphrase, parameters["passphrase"].String().c_str(), PASSPHRASE_BUFF - 1);
            param.data.connect.security_mode = static_cast<SsidSecurity>(parameters["securityMode"].Number());

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_saveSSID, (void *)&param, sizeof(param));
            saved = (retVal == IARM_RESULT_SUCCESS) && param.status;
            LOGINFO("[%s] : retVal:%d status:%d", IARM_BUS_WIFI_MGR_API_saveSSID, retVal, param.status);

            response["result"] = (saved ? 0 : 1);
            returnResponse(true);
        }

        uint32_t WifiManagerWPS::clearSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            bool cleared = false;

            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));

            IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_clearSSID, (void *)&param, sizeof(param));
            cleared = (retVal == IARM_RESULT_SUCCESS) && param.status;
            LOGINFO("[%s] : retVal:%d status:%d", IARM_BUS_WIFI_MGR_API_clearSSID, retVal, param.status);

            response["result"] = (cleared ? 0 : 1);
            returnResponse(true);
        }

        uint32_t WifiManagerWPS::getPairedSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));
            bool result = false;

            if (m_useCachePairedSSID)
            {
                response["ssid"] = m_cachePairedSSID;
                result = true;
            }
            else if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo, (void *)&param, sizeof(param)))
            {
                response["ssid"] = m_cachePairedSSID = string(param.data.getPairedSSIDInfo.ssid, SSID_SIZE);
                m_cachePairedBSSID = string(param.data.getPairedSSIDInfo.bssid, BSSID_BUFF);
                m_useCachePairedSSID = true;
                result = true;
            }
            else
            {
                LOGWARN ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
                result = false;
            }

            returnResponse(result);
        }

        uint32_t WifiManagerWPS::getPairedSSIDInfo(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));
            bool result = false;

            if (m_useCachePairedSSID)
            {
                response["ssid"] = m_cachePairedSSID;
                response["bssid"] = m_cachePairedBSSID;
                result = true;
            }
            else if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo, (void *)&param, sizeof(param)))
            {
                response["ssid"] = m_cachePairedSSID = string(param.data.getPairedSSIDInfo.ssid, SSID_SIZE);
                response["bssid"] = m_cachePairedBSSID = string(param.data.getPairedSSIDInfo.bssid, BSSID_BUFF);
                m_useCachePairedSSID = true;
                result = true;
            }
            else
            {
                LOGWARN ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
                result = false;
            }

            returnResponse(result);
        }

        uint32_t WifiManagerWPS::isPaired(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            IARM_Bus_WiFiSrvMgr_Param_t param;
            memset(&param, 0, sizeof(param));
            int ssid_len = 0;

            if (m_useCachePairedSSID)
            {
                ssid_len = strlen(m_cachePairedSSID.c_str());
            }
            else if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_getPairedSSIDInfo, (void *)&param, sizeof(param)))
            {
                m_cachePairedSSID = string(param.data.getPairedSSIDInfo.ssid, SSID_SIZE);
                m_cachePairedBSSID = string(param.data.getPairedSSIDInfo.bssid, BSSID_BUFF);
                ssid_len = strlen(m_cachePairedSSID.c_str());
                m_useCachePairedSSID = true;
            }
            else
            {
                LOGWARN ("Call to %s for %s failed", IARM_BUS_NM_SRV_MGR_NAME, __FUNCTION__);
            }

            response["result"] = (ssid_len) ? 0 : 1;
            returnResponse(true);
        }
    } // namespace Plugin
} // namespace WPEFramework
