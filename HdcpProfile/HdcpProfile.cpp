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

#include "HdcpProfile.h"

#include "videoOutputPort.hpp"
#include "videoOutputPortConfig.hpp"
#include "dsMgr.h"
#include "manager.hpp"

#include "utils.h"

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
        : AbstractPlugin()
        {
            LOGINFO();
            HdcpProfile::_instance = this;

            InitializeIARM();
            device::Manager::Initialize();

            registerMethod(HDCP_PROFILE_METHOD_GET_HDCP_STATUS, &HdcpProfile::getHDCPStatusWrapper, this);
            registerMethod(HDCP_PROFILE_METHOD_GET_SETTOP_HDCP_SUPPORT, &HdcpProfile::getSettopHDCPSupportWrapper, this);
        }

        HdcpProfile::~HdcpProfile()
        {
            LOGINFO();
            HdcpProfile::_instance = nullptr;
            device::Manager::DeInitialize();
            DeinitializeIARM();
        }

        void HdcpProfile::InitializeIARM()
        {
            LOGINFO();

            Utils::IARM::init();

            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS, dsHdmiEventHandler) );
        }

        void HdcpProfile::DeinitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS) );
            }
        }

        uint32_t HdcpProfile::getHDCPStatusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            response["HDCPStatus"] = getHDCPStatus();
            returnResponse(true);
        }

        uint32_t HdcpProfile::getSettopHDCPSupportWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            dsHdcpProtocolVersion_t hdcpProtocol = dsHDCP_VERSION_MAX;

            try
            {
            device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort("HDMI0");
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
            LOGINFO();

            JsonObject hdcpStatus;

            bool isConnected     = false;
            bool isHDCPCompliant = false;
            bool isHDCPEnabled   = true;
            dsHdcpProtocolVersion_t hdcpProtocol = dsHDCP_VERSION_MAX;
            dsHdcpProtocolVersion_t hdcpReceiverProtocol = dsHDCP_VERSION_MAX;
            dsHdcpProtocolVersion_t hdcpCurrentProtocol = dsHDCP_VERSION_MAX;

            try
            {
                device::VideoOutputPort vPort = device::VideoOutputPortConfig::getInstance().getPort("HDMI0");
                isConnected        = vPort.isDisplayConnected();
                hdcpProtocol       = (dsHdcpProtocolVersion_t)vPort.getHDCPProtocol();
                if(isConnected)
                {
                    isHDCPCompliant    = (vPort.getHDCPStatus() == dsHDCP_STATUS_AUTHENTICATED);
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
            LOGINFO();

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
        }

        void HdcpProfile::onHdmiOutputHDCPStatusEvent(int hdcpStatus)
        {
            LOGINFO();

            JsonObject status = getHDCPStatus();
            JsonObject params;
            params["HDCPStatus"] = status;
            sendNotify(HDCP_PROFILE_EVT_ON_DISPLAY_CONNECTION_CHANGED, params);

            logHdcpStatus("AuthRslt", status);
            return;
        }

        void HdcpProfile::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO();

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
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdcpStatus = eventData->data.hdmi_hdcp.hdcpStatus;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDCP_STATUS  event data:%d \r\n", hdcpStatus);
                HdcpProfile::_instance->onHdmiOutputHDCPStatusEvent(hdcpStatus);

            }
        }

    } // namespace Plugin
} // namespace WPEFramework

