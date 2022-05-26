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

#include "WifiManagerConnect.h"

#include "wifiSrvMgrIarmIf.h"
#include "libIBus.h"
#include "UtilsJsonRpc.h"

#include <cstring>

using namespace WPEFramework::Plugin;

uint32_t WifiManagerConnect::connect(const JsonObject &parameters, JsonObject &response)
{
    JsonObject params = parameters;

    if (params.HasLabel("passphrase"))
    {
        params["passphrase"] = "<passphrase>";

        std::string json;
        params.ToString(json);
        LOGINFO( "params=%s", json.c_str() );
    }
    else
        LOGINFOMETHOD();

    string ssid;
    string passphrase;
    char const* const ssid_label = "ssid";
    char const* const passphrase_label = "passphrase";
    char const* const security_label = "securityMode";
    SecurityMode securityMode = SecurityMode::NONE;

    if (parameters.HasLabel(ssid_label)) {
        getStringParameter(ssid_label, ssid);
    }

    if (parameters.HasLabel(passphrase_label)) {
        getStringParameter(passphrase_label, passphrase);
    }

    if (parameters.HasLabel(security_label)) {
        int security = 0;
        getNumberParameter(security_label, security);
        securityMode = static_cast<SecurityMode>(security);
    }

    returnResponse(connect(ssid, passphrase, securityMode));
}

uint32_t WifiManagerConnect::disconnect(const JsonObject &parameters, JsonObject &response)
{
    LOGINFOMETHOD();
    IARM_Bus_WiFiSrvMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    IARM_Result_t retVal = IARM_Bus_Call(IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_disconnectSSID, (void *)&param, sizeof(param));
    LOGINFO("[%s] : retVal:%d status:%d", IARM_BUS_WIFI_MGR_API_disconnectSSID, retVal, param.status);

    response["result"] = string();
    returnResponse(retVal == IARM_RESULT_SUCCESS);
}

bool WifiManagerConnect::connect(const std::string &ssid, const std::string &passphrase, WPEFramework::Plugin::SecurityMode securityMode)
{
    bool success = false;
    IARM_Result_t retVal = IARM_RESULT_SUCCESS;
    IARM_Bus_WiFiSrvMgr_Param_t param;
    memset(&param, 0, sizeof(param));

    if(ssid.length() || passphrase.length())
    {
        ssid.copy(param.data.connect.ssid, sizeof(param.data.connect.ssid) - 1);
        passphrase.copy(param.data.connect.passphrase, sizeof(param.data.connect.passphrase) - 1);
        param.data.connect.security_mode = (SsidSecurity)securityMode;
    }

    retVal = IARM_Bus_Call( IARM_BUS_NM_SRV_MGR_NAME, IARM_BUS_WIFI_MGR_API_connect, (void *)&param, sizeof(param));

    if(retVal == IARM_RESULT_SUCCESS && param.status)
    {
        success = true;
    }

    return success;
}
