/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "HdcpProfile.h"

#include "videoOutputPort.hpp"
#include "videoOutputPortConfig.hpp"
#include "dsMgr.h"
#include "pwrMgr.h"
#include "manager.hpp"
#include "host.hpp"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0
#define HDMI_HOT_PLUG_EVENT_DISCONNECTED 1

#define HDCP_PROFILE_METHOD_GET_HDCP_STATUS "getHDCPStatus"
#define HDCP_PROFILE_METHOD_GET_SETTOP_HDCP_SUPPORT "getSettopHDCPSupport"
#define HDCP_PROFILE_METHOD_SET_HDCPPROFILE "setHDCPProfile"
#define HDCP_PROFILE_METHOD_GET_HDCPPROFILE "getHDCPProfile"

#define HDCP_PROFILE_EVT_ON_DISPLAY_CONNECTION_CHANGED "onDisplayConnectionChanged"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdcpProfile, 1, 0);

        HdcpProfile* HdcpProfile::_instance = nullptr;

        HdcpProfile::HdcpProfile()
        : PluginHost::JSONRPC()
        {
            HdcpProfile::_instance = this;

            InitializeIARM();
            device::Manager::Initialize();

            Register(HDCP_PROFILE_METHOD_GET_HDCP_STATUS, &HdcpProfile::getHDCPStatusWrapper, this);
            Register(HDCP_PROFILE_METHOD_GET_SETTOP_HDCP_SUPPORT, &HdcpProfile::getSettopHDCPSupportWrapper, this);
        }

        HdcpProfile::~HdcpProfile()
        {
        }

        void HdcpProfile::Deinitialize(PluginHost::IShell* /* service */)
        {
            HdcpProfile::_instance = nullptr;
            //No need to run device::Manager::DeInitialize for individual plugin. As it is a singleton instance
            //and shared among all wpeframework plugins
            DeinitializeIARM();
        }

        void HdcpProfile::InitializeIARM()
        {
            Utils::IARM::init();

            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS, dsHdmiEventHandler) );
        }

        void HdcpProfile::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS) );
            }
        }

        uint32_t HdcpProfile::getHDCPStatusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["HDCPStatus"] = getHDCPStatus();
            returnResponse(true);
        }

        uint32_t HdcpProfile::getSettopHDCPSupportWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            dsHdcpProtocolVersion_t hdcpProtocol = dsHDCP_VERSION_MAX;

            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort.c_str());
                hdcpProtocol = (dsHdcpProtocolVersion_t)vPort.getHDCPProtocol();
            }
            catch (const std::exception e)
            {
                LOGWARN("DS exception caught from %s\r\n", __FUNCTION__);
            }

            if(hdcpProtocol == dsHDCP_VERSION_2X)
            {
                response["supportedHDCPVersion"] = "2.2";
                LOGWARN("supportedHDCPVersion :2.2");
            }
            else
            {
                response["supportedHDCPVersion"] = "1.4";
                LOGWARN("supportedHDCPVersion :1.4");
            }

            response["isHDCPSupported"] = true;

            returnResponse(true);
        }

        JsonObject HdcpProfile::getHDCPStatus()
        {
            JsonObject hdcpStatus;

            bool isConnected     = false;
            bool isHDCPCompliant = false;
            bool isHDCPEnabled   = true;
            int eHDCPEnabledStatus   = dsHDCP_STATUS_UNPOWERED;
            dsHdcpProtocolVersion_t hdcpProtocol = dsHDCP_VERSION_MAX;
            dsHdcpProtocolVersion_t hdcpReceiverProtocol = dsHDCP_VERSION_MAX;
            dsHdcpProtocolVersion_t hdcpCurrentProtocol = dsHDCP_VERSION_MAX;

            try
            {
                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort(strVideoPort.c_str());
                isConnected        = vPort.isDisplayConnected();
                hdcpProtocol       = (dsHdcpProtocolVersion_t)vPort.getHDCPProtocol();
                eHDCPEnabledStatus = vPort.getHDCPStatus();
                if(isConnected)
                {
                    isHDCPCompliant    = (eHDCPEnabledStatus == dsHDCP_STATUS_AUTHENTICATED);
                    isHDCPEnabled      = vPort.isContentProtected();
                    hdcpReceiverProtocol = (dsHdcpProtocolVersion_t)vPort.getHDCPReceiverProtocol();
                    hdcpCurrentProtocol  = (dsHdcpProtocolVersion_t)vPort.getHDCPCurrentProtocol();
                }
                else
                {
                    isHDCPCompliant = false;
                    isHDCPEnabled = false;
                }
            }
            catch (const std::exception e)
            {
                LOGWARN("DS exception caught from %s\r\n", __FUNCTION__);
            }

            hdcpStatus["isConnected"] = isConnected;
            hdcpStatus["isHDCPCompliant"] = isHDCPCompliant;
            hdcpStatus["isHDCPEnabled"] = isHDCPEnabled;
            hdcpStatus["hdcpReason"] = eHDCPEnabledStatus;

            if(hdcpProtocol == dsHDCP_VERSION_2X)
            {
                hdcpStatus["supportedHDCPVersion"] = "2.2";
            }
            else
            {
                hdcpStatus["supportedHDCPVersion"] = "1.4";
            }

            if(hdcpReceiverProtocol == dsHDCP_VERSION_2X)
            {
                hdcpStatus["receiverHDCPVersion"] = "2.2";
            }
            else
            {
                hdcpStatus["receiverHDCPVersion"] = "1.4";
            }

            if(hdcpCurrentProtocol == dsHDCP_VERSION_2X)
            {
                hdcpStatus["currentHDCPVersion"] = "2.2";
            }
            else
            {
                hdcpStatus["currentHDCPVersion"] = "1.4";
            }

            logHdcpStatus("Request", hdcpStatus);
            return hdcpStatus;
        }

        void HdcpProfile::onHdmiOutputHotPlug(int connectStatus)
        {
            if (HDMI_HOT_PLUG_EVENT_CONNECTED == connectStatus)
                LOGWARN(" %s   Status : %d \n",__FUNCTION__, connectStatus);

            JsonObject status = getHDCPStatus();
            JsonObject params;
            params["HDCPStatus"] = status;
            sendNotify(HDCP_PROFILE_EVT_ON_DISPLAY_CONNECTION_CHANGED, params);

            logHdcpStatus("Hotplug", status);
            return;
        }

        void HdcpProfile::logHdcpStatus (const char *trigger, const JsonObject& status)
        {
            LOGWARN("[%s]-HDCPStatus::isConnected : %s", trigger, status["isConnected"].Boolean() ? "true" : "false");
            LOGWARN("[%s]-HDCPStatus::isHDCPEnabled: %s", trigger, status["isHDCPEnabled"].Boolean() ? "true" : "false");
            LOGWARN("[%s]-HDCPStatus::isHDCPCompliant: %s", trigger, status["isHDCPCompliant"].Boolean() ? "true" : "false");
            LOGWARN("[%s]-HDCPStatus::supportedHDCPVersion: %s", trigger, status["supportedHDCPVersion"].String().c_str());
            LOGWARN("[%s]-HDCPStatus::receiverHDCPVersion: %s", trigger, status["receiverHDCPVersion"].String().c_str());
            LOGWARN("[%s]-HDCPStatus::currentHDCPVersion %s", trigger, status["currentHDCPVersion"].String().c_str());
            LOGWARN("[%s]-HDCPStatus::hdcpReason %s", trigger, status["hdcpReason"].String().c_str());
        }

        void HdcpProfile::onHdmiOutputHDCPStatusEvent(int hdcpStatus)
        {
            JsonObject status = getHDCPStatus();
            JsonObject params;
            params["HDCPStatus"] = status;
            sendNotify(HDCP_PROFILE_EVT_ON_DISPLAY_CONNECTION_CHANGED, params);

            logHdcpStatus("AuthRslt", status);
            return;
        }

        void HdcpProfile::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdcpProfile::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);

                HdcpProfile::_instance->onHdmiOutputHotPlug(hdmi_hotplug_event);
            }
            else if (IARM_BUS_DSMGR_EVENT_HDCP_STATUS == eventId)
            {
                IARM_Bus_PWRMgr_GetPowerState_Param_t param;
                IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState, (void *)&param, sizeof(param));
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdcpStatus = eventData->data.hdmi_hdcp.hdcpStatus;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDCP_STATUS  event data:%d  param.curState: %d \r\n", hdcpStatus,param.curState);
                HdcpProfile::_instance->onHdmiOutputHDCPStatusEvent(hdcpStatus);

            }
        }

    } // namespace Plugin
} // namespace WPEFramework

