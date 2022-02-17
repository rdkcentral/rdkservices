/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
* Copyright 2021 Liberty Global Service B.V.
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

#include <string>

#include "LgiHdcpProfile.h"

#include "videoOutputPort.hpp"
#include "videoOutputPortConfig.hpp"
#include "dsMgr.h"
#include "manager.hpp"
#include "host.hpp"

#include "utils.h"

#define HDCP_PROFILE_METHOD_GET_Rx_HDCP_SUPPORT "getRxHDCPSupportedVersion"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(LgiHdcpProfile, 1, 0);
        static const char* getHdcpReasonStr (int eHDCPEnabledStatus);

        LgiHdcpProfile::LgiHdcpProfile()
        : HdcpProfile()
        {
            registerMethod(HDCP_PROFILE_METHOD_GET_Rx_HDCP_SUPPORT, &LgiHdcpProfile::getRxHDCPSupportedVersion, this);
        }

        LgiHdcpProfile::~LgiHdcpProfile()
        {
        }

        //Begin methods
        uint32_t LgiHdcpProfile::getRxHDCPSupportedVersion(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            string rxHDCPSupportedVersion;
            bool res = false;
            bool isConnected = false;
            dsHdcpProtocolVersion_t HDCPVersion = dsHDCP_VERSION_1X;
            const string defaultVideoDisplay = device::Host::getInstance().getDefaultVideoPortName();
            const string videoDisplay = parameters.HasLabel("videoDisplay") ? parameters["videoDisplay"].String() : defaultVideoDisplay;

            try
            {
                device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort(videoDisplay);
                isConnected = vPort.isDisplayConnected();
                if(isConnected)
                {
                    isConnected = vPort.isActive();
                    if(isConnected)
                    {
                        HDCPVersion = (dsHdcpProtocolVersion_t)vPort.getRxHDCPSupportedVersion();
                    }
                }
            }
            catch (const std::exception e)
            {
                LOGWARN("DS exception caught: %s\n",e.what());
                isConnected = false;
            }

            if(!isConnected)
            {
                rxHDCPSupportedVersion = "0.0";
                LOGWARN("HDMI not connected; HDCP version unknown\n");
            }
            else
            {
                LOGINFO("HDMI connected, HDCP version %d\n",int(HDCPVersion));
                switch(HDCPVersion)
                {
                    case dsHDCP_VERSION_2X:
                        rxHDCPSupportedVersion = "2.2";
                        res = true;
                        break;
                    case dsHDCP_VERSION_1X:
                        res = true;
                        rxHDCPSupportedVersion = "1.4";
                        break;
                    default:
                        LOGWARN("protocol version value '%d' unknown; returning 0.0\n", HDCPVersion);
                        rxHDCPSupportedVersion = "0.0";
                        break;
                }
            }

            response["supportedRxHDCPVersion"] = rxHDCPSupportedVersion;
            returnResponse(res);
        }

        //End methods


    } // namespace Plugin
} // namespace WPEFramework

