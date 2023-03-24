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
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::initiateWPSPairing2(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::cancelWPSPairing(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::saveSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::clearSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::getPairedSSID(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::getPairedSSIDInfo(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }

        uint32_t WifiManagerWPS::isPaired(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFOMETHOD();
            returnResponse(false);
        }
    } // namespace Plugin
} // namespace WPEFramework
