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

#include "HdmiCec_2.h"


#include "ccec/Connection.hpp"
#include "ccec/CECFrame.hpp"
#include "ccec/MessageEncoder.hpp"
#include "host.hpp"
#include "ccec/host/RDK.hpp"


#include "ccec/drivers/iarmbus/CecIARMBusMgr.h"

#include "pwrMgr.h"
#include "dsMgr.h"
#include "dsDisplay.h"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#include "websocket/URL.h"

#include "utils.h"

#define HDMICEC2_METHOD_SET_ENABLED "setEnabled"
#define HDMICEC2_METHOD_GET_ENABLED "getEnabled"
#define HDMICEC2_METHOD_OTP_SET_ENABLED "setOTPEnabled"
#define HDMICEC2_METHOD_OTP_GET_ENABLED "getOTPEnabled"
#define HDMICEC2_METHOD_SET_OSD_NAME "setOSDName"
#define HDMICEC2_METHOD_GET_OSD_NAME "getOSDName"
#define HDMICEC2_METHOD_SET_VENDOR_ID "setVendorId"
#define HDMICEC2_METHOD_GET_VENDOR_ID "getVendorId"
#define HDMICEC2_METHOD_PERFORM_OTP_ACTION "performOTPAction"

#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define DEV_TYPE_TUNER 1
#define HDMI_HOT_PLUG_EVENT_CONNECTED 0

#define CEC_SETTING_ENABLED_FILE "/opt/persistent/ds/cecData_2.json"
#define CEC_SETTING_ENABLED "cecEnabled"
#define CEC_SETTING_OTP_ENABLED "cecOTPEnabled"
#define CEC_SETTING_OSD_NAME "cecOSDName"
#define CEC_SETTING_VENDOR_ID "cecVendorId"

static vector<uint8_t> defaultVendorId = {0x00,0x19,0xFB};
static VendorID appVendorId = {defaultVendorId.at(0),defaultVendorId.at(1),defaultVendorId.at(2)};
static VendorID lgVendorId = {0x00,0xE0,0x91};
static PhysicalAddress physical_addr = {0x0F,0x0F,0x0F,0x0F};
static LogicalAddress logicalAddress = 0xF;
static OSDName osdName = "TV Box";
static int32_t powerState = 1;
static PowerStatus tvPowerState = 1;
static bool isDeviceActiveSource = false;
static bool isLGTvConnected = false;

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiCec_2, 1, 0);

        HdmiCec_2* HdmiCec_2::_instance = nullptr;
        static int libcecInitStatus = 0;

//=========================================== HdmiCec_2FrameListener =========================================
        void HdmiCec_2FrameListener::notify(const CECFrame &in) const {
                const uint8_t *buf = NULL;
                char strBuffer[512] = {0}; 
                size_t len = 0;

                in.getBuffer(&buf, &len);
                for (int i = 0; i < len; i++) {
                   sprintf(strBuffer + (i*3) , "%02X ",(uint8_t) *(buf + i));
                }
                LOGINFO("   >>>>>    Received CEC Frame: :%s \n",strBuffer);

                MessageDecoder(processor).decode(in);
       }

//=========================================== HdmiCec_2Processor =========================================
       void HdmiCec_2Processor::process (const ActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ActiveSource %s : %s  : %s \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
             if(msg.physicalAddress.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("ActiveSource isDeviceActiveSource status :%d \n", isDeviceActiveSource);
       }
       void HdmiCec_2Processor::process (const InActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: InActiveSource %s : %s : %s  \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
       }
       void HdmiCec_2Processor::process (const ImageViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ImageViewOn \n");
       }
       void HdmiCec_2Processor::process (const TextViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: TextViewOn\n");
       }
       void HdmiCec_2Processor::process (const RequestActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RequestActiveSource\n");
             if(isDeviceActiveSource)
             {
                  LOGINFO("sending  ActiveSource\n");
                  try
                  { 
                      conn.sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(ActiveSource(physical_addr)));
                  } 
                  catch(...)
                  {
                     LOGWARN("Exception while sending ActiveSource");
                  }
             }
       }
       void HdmiCec_2Processor::process (const Standby &msg, const Header &header)
       {
             printHeader(header);
             if(header.from.toInt() == LogicalAddress::TV)
             {
                 tvPowerState = 1; 
                 LOGINFO("Command: Standby  tvPowerState :%s \n",(tvPowerState.toInt())?"OFF":"ON");
             }  
       }
       void HdmiCec_2Processor::process (const GetCECVersion &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GetCECVersion sending CECVersion response \n");
             try
             { 
                 conn.sendTo(header.from, MessageEncoder().encode(CECVersion(Version::V_1_4)));
             } 
             catch(...)
             {
                 LOGWARN("Exception while sending CECVersion ");
             }
       }
       void HdmiCec_2Processor::process (const CECVersion &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());
       }
       void HdmiCec_2Processor::process (const SetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetMenuLanguage Language : %s \n",msg.language.toString().c_str());
       }
       void HdmiCec_2Processor::process (const GiveOSDName &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GiveOSDName sending SetOSDName : %s\n",osdName.toString().c_str());
             try
             { 
                 conn.sendTo(header.from, MessageEncoder().encode(SetOSDName(osdName)));
             } 
             catch(...)
             {
                 LOGWARN("Exception while sending SetOSDName");
             }
       }
       void HdmiCec_2Processor::process (const GivePhysicalAddress &msg, const Header &header)
       {
             LOGINFO("Command: GivePhysicalAddress\n");
             if (!(header.from == LogicalAddress(LogicalAddress::BROADCAST)))
             {
                 try
                 { 
                     LOGINFO(" sending ReportPhysicalAddress response physical_addr :%s logicalAddress :%x \n",physical_addr.toString().c_str(), logicalAddress.toInt());
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ReportPhysicalAddress(physical_addr,logicalAddress.toInt()))); 
                 } 
                 catch(...)
                 {
                    LOGWARN("Exception while sending ReportPhysicalAddress ");
                 }
             }
       }
       void HdmiCec_2Processor::process (const GiveDeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             try
             {
                 LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n",(isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                 if(isLGTvConnected)
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                 else 
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));
             }
             catch(...)
             {
                 LOGWARN("Exception while sending DeviceVendorID");
             }

       }
       void HdmiCec_2Processor::process (const SetOSDString &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetOSDString OSDString : %s\n",msg.osdString.toString().c_str());
       }
       void HdmiCec_2Processor::process (const SetOSDName &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());
       }
       void HdmiCec_2Processor::process (const RoutingChange &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingChange From : %s To: %s \n",msg.from.toString().c_str(),msg.to.toString().c_str());
             if(msg.to.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
       }
       void HdmiCec_2Processor::process (const RoutingInformation &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingInformation Routing Information to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
       }
       void HdmiCec_2Processor::process (const SetStreamPath &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetStreamPath Set Stream Path to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);

       }
       void HdmiCec_2Processor::process (const GetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GetMenuLanguage\n");
       }
       void HdmiCec_2Processor::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ReportPhysicalAddress\n");
       }
       void HdmiCec_2Processor::process (const DeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());
       }
       void HdmiCec_2Processor::process (const GiveDevicePowerStatus &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GiveDevicePowerStatus sending powerState :%d \n",powerState);
             try
             { 
                 conn.sendTo(header.from, MessageEncoder().encode(ReportPowerStatus(PowerStatus(powerState))));
             } 
             catch(...)
             {
                 LOGWARN("Exception while sending ReportPowerStatus");
             }
       }
       void HdmiCec_2Processor::process (const ReportPowerStatus &msg, const Header &header)
       {
             printHeader(header);
             if ((header.from == LogicalAddress(LogicalAddress::TV)))
                 tvPowerState = msg.status; 
             LOGINFO("Command: ReportPowerStatus TV Power Status from:%s status : %s \n",header.from.toString().c_str(),msg.status.toString().c_str());
       }
       void HdmiCec_2Processor::process (const FeatureAbort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: FeatureAbort\n");
       }
       void HdmiCec_2Processor::process (const Abort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: Abort\n");
       }
       void HdmiCec_2Processor::process (const Polling &msg, const Header &header)                                 {
             printHeader(header);
             LOGINFO("Command: Polling\n");
       }


//=========================================== HdmiCec_2 =========================================

       HdmiCec_2::HdmiCec_2()
       : AbstractPlugin()
       {
           LOGWARN("Initlaizing CEC_2");
           HdmiCec_2::_instance = this;
           smConnection = NULL;
           InitializeIARM();

           registerMethod(HDMICEC2_METHOD_SET_ENABLED, &HdmiCec_2::setEnabledWrapper, this);
           registerMethod(HDMICEC2_METHOD_GET_ENABLED, &HdmiCec_2::getEnabledWrapper, this);
           registerMethod(HDMICEC2_METHOD_OTP_SET_ENABLED, &HdmiCec_2::setOTPEnabledWrapper, this);
           registerMethod(HDMICEC2_METHOD_OTP_GET_ENABLED, &HdmiCec_2::getOTPEnabledWrapper, this);
           registerMethod(HDMICEC2_METHOD_SET_OSD_NAME, &HdmiCec_2::setOSDNameWrapper, this);
           registerMethod(HDMICEC2_METHOD_GET_OSD_NAME, &HdmiCec_2::getOSDNameWrapper, this);
           registerMethod(HDMICEC2_METHOD_SET_VENDOR_ID, &HdmiCec_2::setVendorIdWrapper, this);
           registerMethod(HDMICEC2_METHOD_GET_VENDOR_ID, &HdmiCec_2::getVendorIdWrapper, this);
           registerMethod(HDMICEC2_METHOD_PERFORM_OTP_ACTION, &HdmiCec_2::performOTPActionWrapper, this);

           logicalAddressDeviceType = "None";
           logicalAddress = 0xFF;

           // load persistence setting
           loadSettings();

           try
           {
               //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
               device::Manager::Initialize();
               device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
               if (vPort.isDisplayConnected())
               {
                   vector<uint8_t> edidVec;
                   vPort.getDisplay().getEDIDBytes(edidVec);
                   //Set LG vendor id if connected with LG TV
                   if(edidVec.at(8) == 0x1E && edidVec.at(9) == 0x6D)
                   {
                      isLGTvConnected = true;
                   }
                   LOGINFO("manufacturer byte from edid :%x: %x  isLGTvConnected :%d",edidVec.at(8),edidVec.at(9),isLGTvConnected);
               }
            }
            catch(...)
            {
                LOGWARN("Exception in getting edid info .\r\n");
            }

            // get power state:
            IARM_Bus_PWRMgr_GetPowerState_Param_t param;
            int err = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                            IARM_BUS_PWRMGR_API_GetPowerState,
                            (void *)&param,
                            sizeof(param));
            if(err == IARM_RESULT_SUCCESS)
            {
                powerState = (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)?0:1 ;
                LOGINFO("Current state is IARM: (%d) powerState :%d \n",param.curState,powerState);
            }
            
            if (cecSettingEnabled)
            {
               try
               {
                   CECEnable();
               }
               catch(...)
               {
                   LOGWARN("Exception while enabling CEC settings .\r\n");
               }
            }
       }

       HdmiCec_2::~HdmiCec_2()
       {
           LOGINFO();
           HdmiCec_2::_instance = nullptr;
           DeinitializeIARM();
       }

       const void HdmiCec_2::InitializeIARM()
       {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
           }
       }

       void HdmiCec_2::DeinitializeIARM()
       {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
            }
       }

       void HdmiCec_2::cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            LOGINFO();

            if(!HdmiCec_2::_instance)
                return;

            if( !strcmp(owner, IARM_BUS_CECMGR_NAME))
            {
                switch (eventId)
                {
                    case IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED:
                    {
                        HdmiCec_2::_instance->onCECDaemonInit();
                    }
                    break;
                    case IARM_BUS_CECMGR_EVENT_STATUS_UPDATED:
                    {
                        IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = new IARM_Bus_CECMgr_Status_Updated_Param_t;
                        if(evtData)
                        {
                            memcpy(evtData,data,sizeof(IARM_Bus_CECMgr_Status_Updated_Param_t));
                            HdmiCec_2::_instance->cecStatusUpdated(evtData);
                        }
                    }
                    break;
                    default:
                    /*Do nothing*/
                    break;
                }
            }
       }

       void HdmiCec_2::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            LOGINFO();

            if(!HdmiCec_2::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);
                HdmiCec_2::_instance->onHdmiHotPlug(hdmi_hotplug_event);
            }
       }

       void HdmiCec_2::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            LOGINFO();

            if(!HdmiCec_2::_instance)
                return;

            if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
                if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
                    IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
                    LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                            param->data.state.curState, param->data.state.newState);
                    if(param->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                    {
                        powerState = 0; 
                        _instance->performOTPAction();
                    }
                    else
                        powerState = 1;

                }
           }
       }

       void HdmiCec_2::onCECDaemonInit()
       {
            LOGINFO();

            if(true == getEnabled())
            {
                setEnabled(false);
                setEnabled(true);
            }
            else
            {
                /*Do nothing as CEC is not already enabled*/
            }
       }

       void HdmiCec_2::cecStatusUpdated(void *evtStatus)
       {
            LOGINFO();

            IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = (IARM_Bus_CECMgr_Status_Updated_Param_t *)evtStatus;
            if(evtData)
            {
               try{
                    getPhysicalAddress();

                    unsigned int logicalAddr = evtData->logicalAddress;
                    std::string logicalAddrDeviceType = DeviceType(LogicalAddress(evtData->logicalAddress).getType()).toString().c_str();

                    LOGINFO("cecLogicalAddressUpdated: logical address updated: %d , saved : %d ", logicalAddr, logicalAddress.toInt());
                    if (logicalAddr != logicalAddress.toInt() || logicalAddrDeviceType != logicalAddressDeviceType)
                    {
                        logicalAddress = logicalAddr;
                        logicalAddressDeviceType = logicalAddrDeviceType;
                    }
                }
                catch (const std::exception e)
                {
                    LOGWARN("CEC exception caught from cecStatusUpdated");
                }

                delete evtData;
            }
           return;
       }

       void HdmiCec_2::onHdmiHotPlug(int connectStatus)
       {
            LOGINFO();

            if (HDMI_HOT_PLUG_EVENT_CONNECTED == connectStatus)
            {
                LOGINFO ("onHdmiHotPlug Status : %d ", connectStatus);
                getPhysicalAddress();
                getLogicalAddress();
                try
                {
                   device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort("HDMI0");
                   if (vPort.isDisplayConnected())
                   {
                     vector<uint8_t> edidVec;
                     vPort.getDisplay().getEDIDBytes(edidVec);
                     //Set LG vendor id if connected with LG TV
                     if(edidVec.at(8) == 0x1E && edidVec.at(9) == 0x6D)
                     {
                         isLGTvConnected = true;
                     }
                     LOGINFO("manufacturer byte from edid :%x: %x  isLGTvConnected :%d",edidVec.at(8),edidVec.at(9),isLGTvConnected);
                   }
                 }
                 catch(...)
                 {
                    LOGWARN("Exception in getting edid info .\r\n");
                 }
            }
            return;
       }

       uint32_t HdmiCec_2::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            LOGINFO();

            bool enabled = false;

            if (parameters.HasLabel("enabled"))
            {
                getBoolParameter("enabled", enabled);
            }
            else
            {
                returnResponse(false);
            }

            setEnabled(enabled);
            returnResponse(true);
       }

       uint32_t HdmiCec_2::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getEnabled();
            returnResponse(true);
       }
       uint32_t HdmiCec_2::setOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            LOGINFO();

            bool enabled = false;

            if (parameters.HasLabel("enabled"))
            {
                getBoolParameter("enabled", enabled);
            }
            else
            {
                returnResponse(false);
            }

            setOTPEnabled(enabled);
            returnResponse(true);
       }

       uint32_t HdmiCec_2::getOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getOTPEnabled();
            returnResponse(true);
       }

       uint32_t HdmiCec_2::setOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
       {
            LOGINFO();
            bool enabled = false;

            if (parameters.HasLabel("name"))
            {
                std::string osd = parameters["name"].String();
                LOGINFO("setOSDNameWrapper osdName: %s",osd.c_str());
                osdName = osd.c_str();
                persistOSDName(osd.c_str());
            }
            else
            {
                returnResponse(false);
            }
            returnResponse(true);
        }

        uint32_t HdmiCec_2::getOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
        {
            response["name"] = osdName.toString(); 
            LOGINFO("getOSDNameWrapper osdName : %s \n",osdName.toString().c_str());
            returnResponse(true);
        }

        uint32_t HdmiCec_2::setVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool enabled = false;

            if (parameters.HasLabel("vendorid"))
            {
                std::string id = parameters["vendorid"].String();
                unsigned int vendorID = 0x00;
                try
                {
                    vendorID = stoi(id,NULL,16);
                }
                catch (...)
                {
                    LOGWARN("Exception in setVendorIdWrapper set default value\n");
                    vendorID = 0x0019FB;
                }
                appVendorId = {(uint8_t)(vendorID >> 16 & 0xff),(uint8_t)(vendorID>> 8 & 0xff),(uint8_t) (vendorID & 0xff)};
                LOGINFO("appVendorId : %s  vendorID :%x \n",appVendorId.toString().c_str(), vendorID );

                persistVendorId(vendorID);
            }
            else
            {
                returnResponse(false);
            }
            returnResponse(true);
        }

        uint32_t HdmiCec_2::getVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO("getVendorIdWrapper  appVendorId : %s  \n",appVendorId.toString().c_str());
            response["vendorid"] = appVendorId.toString() ;
            returnResponse(true);
        }


        uint32_t HdmiCec_2::performOTPActionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            if(performOTPAction())
            { 
                returnResponse(true);
            }
            else
            {
                returnResponse(false);
            }
        }

        bool HdmiCec_2::loadSettings()
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            if( file.Open())
            {
                JsonObject parameters;
                parameters.IElement::FromFile(file);
                bool isConfigAdded = false;

                if( parameters.HasLabel(CEC_SETTING_ENABLED))
                {
                     getBoolParameter(CEC_SETTING_ENABLED, cecSettingEnabled);
                     LOGINFO("CEC_SETTING_ENABLED present value:%d",cecSettingEnabled);
                }
                else
                {
                    parameters[CEC_SETTING_ENABLED] = true;
                    cecSettingEnabled = true;
                    isConfigAdded = true;
                    LOGINFO("CEC_SETTING_ENABLED not present set dafult true:\n ");
                }

                if( parameters.HasLabel(CEC_SETTING_OTP_ENABLED))
                {
                    getBoolParameter(CEC_SETTING_OTP_ENABLED, cecOTPSettingEnabled);
                    LOGINFO("CEC_SETTING_OTP_ENABLED present value :%d",cecOTPSettingEnabled);
                }
                else
                {
                    parameters[CEC_SETTING_OTP_ENABLED] = true;
                    cecOTPSettingEnabled = true;
                    isConfigAdded = true;
                    LOGINFO("CEC_SETTING_OTP_ENABLED not present set dafult true:\n ");
                }
                if( parameters.HasLabel(CEC_SETTING_OSD_NAME))
                {
                    std::string osd_name;
                    getStringParameter(CEC_SETTING_OSD_NAME, osd_name);
                    osdName = osd_name.c_str();
                    LOGINFO("CEC_SETTING_OTP_ENABLED present osd_name :%s",osdName.toString().c_str());
                }
                else
                {
                    parameters[CEC_SETTING_OSD_NAME] = osdName.toString();
                    LOGINFO("CEC_SETTING_OSD_NMAE not  present set dafult value :%s\n ",osdName.toString().c_str());
                    isConfigAdded = true;
                }
                unsigned int  vendorId = (defaultVendorId.at(0) <<16) | ( defaultVendorId.at(1) << 8 ) | defaultVendorId.at(2);
                if( parameters.HasLabel(CEC_SETTING_VENDOR_ID))
                {
                    getNumberParameter(CEC_SETTING_VENDOR_ID, vendorId);
                    LOGINFO("CEC_SETTING_VENDOR_ID present :%x ",vendorId);
                }
                else
                {
                    LOGINFO("CEC_SETTING_VENDOR_ID not  present set dafult value :%x \n ",vendorId);
                    parameters[CEC_SETTING_VENDOR_ID] = vendorId;
                    isConfigAdded = true;
                }
                
                appVendorId = {(uint8_t)(vendorId >> 16 & 0xff),(uint8_t)(vendorId >> 8 & 0xff),(uint8_t) (vendorId & 0xff)}; 
                LOGINFO("appVendorId : %s  vendorId :%x \n",appVendorId.toString().c_str(), vendorId );

                if(isConfigAdded)
                {
                    LOGINFO("isConfigAdded true so update file:\n ");
                    file.Destroy();
                    file.Create();
                    parameters.IElement::ToFile(file);

                }

                file.Close();
            }
            else
            {
                LOGINFO("CEC_SETTING_ENABLED_FILE file not present create with default settings ");
                file.Open(false);
                if (!file.IsOpen())
                    file.Create();

                JsonObject parameters;
                unsigned int  vendorId = (defaultVendorId.at(0) <<16) | ( defaultVendorId.at(1) << 8 ) | defaultVendorId.at(2);
                parameters[CEC_SETTING_ENABLED] = true;
                parameters[CEC_SETTING_OTP_ENABLED] = true;
                parameters[CEC_SETTING_OSD_NAME] = osdName.toString();
                parameters[CEC_SETTING_VENDOR_ID] = vendorId;

                cecSettingEnabled = true;
                cecOTPSettingEnabled = true;
                parameters.IElement::ToFile(file);

                file.Close();

            }

            return cecSettingEnabled;
        }

        void HdmiCec_2::persistSettings(bool enableStatus)
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            file.Open(false);
            if (!file.IsOpen())
                file.Create();

            JsonObject cecSetting;
            cecSetting.IElement::FromFile(file);
            file.Destroy();
            file.Create();
            cecSetting[CEC_SETTING_ENABLED] = enableStatus;
            cecSetting.IElement::ToFile(file);

            file.Close();

            return;
        }

        void HdmiCec_2::persistOTPSettings(bool enableStatus)
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            file.Open(false);
            if (!file.IsOpen())
                file.Create();

            JsonObject cecSetting;
            cecSetting.IElement::FromFile(file);
            file.Destroy();
            file.Create();
            cecSetting[CEC_SETTING_OTP_ENABLED] = enableStatus;
            cecSetting.IElement::ToFile(file);

            file.Close();

            return;
        }

        void HdmiCec_2::persistOSDName(const char *name)
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            file.Open(false);
            if (!file.IsOpen())
                file.Create();

            JsonObject cecSetting;
            cecSetting.IElement::FromFile(file);
            file.Destroy();
            file.Create();
            cecSetting[CEC_SETTING_OSD_NAME] = name;
            cecSetting.IElement::ToFile(file);

            file.Close();

            return;
        }

        void HdmiCec_2::persistVendorId(unsigned int vendorId)
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            file.Open(false);
            if (!file.IsOpen())
                file.Create();

            JsonObject cecSetting;
            cecSetting.IElement::FromFile(file);
            file.Destroy();
            file.Create();
            cecSetting[CEC_SETTING_VENDOR_ID] = vendorId;
            cecSetting.IElement::ToFile(file);

            file.Close();

            return;
        }

        void HdmiCec_2::setEnabled(bool enabled)
        {
           LOGINFO("Entered setEnabled ");

           if (cecSettingEnabled != enabled)
           {
               persistSettings(enabled);
               cecSettingEnabled = enabled;
           }
           if(true == enabled)
           {
               CECEnable();
           }
           else
           {
               CECDisable();
           }
           return;
        }

        void HdmiCec_2::setOTPEnabled(bool enabled)
        {
           if (cecOTPSettingEnabled != enabled)
           {
               LOGINFO("persist setOTPEnabled ");
               persistOTPSettings(enabled);
               cecOTPSettingEnabled = enabled;
           }
           return;
        }

        void HdmiCec_2::CECEnable(void)
        {
            LOGINFO("Entered CECEnable");
            if (cecEnableStatus)
            {
                LOGWARN("CEC Already Enabled");
                return;
            }

            if(0 == libcecInitStatus)
            {
                try
                {
                    LibCCEC::getInstance().init();
                }
                catch (const std::exception e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().init()");
                }
            }
            libcecInitStatus++;
          

            //Acquire CEC Addresses
            getPhysicalAddress();
            getLogicalAddress();

            smConnection = new Connection(logicalAddress.toInt(),false,"ServiceManager::Connection::");
            smConnection->open();
            msgProcessor = new HdmiCec_2Processor(*smConnection);
            msgFrameListener = new HdmiCec_2FrameListener(*msgProcessor);
            smConnection->addFrameListener(msgFrameListener);

            cecEnableStatus = true;

            if(smConnection)
            {
                LOGINFO("Command: sending GiveDevicePowerStatus \r\n");
                smConnection->sendTo(LogicalAddress(LogicalAddress::TV), MessageEncoder().encode(GiveDevicePowerStatus()), 5000);
                LOGINFO("Command: sending request active Source isDeviceActiveSource is set to false\r\n");
                smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(RequestActiveSource()), 5000);
                isDeviceActiveSource = false;
            }
            return;
        }

        void HdmiCec_2::CECDisable(void)
        {
            LOGINFO("Entered CECDisable ");

            if(!cecEnableStatus)
            {
                LOGWARN("CEC Already Disabled ");
                return;
            }

            if (smConnection != NULL)
            {
                smConnection->close();
                delete smConnection;
                smConnection = NULL;
            }
            cecEnableStatus = false;

            if(1 == libcecInitStatus)
            {
                try
                {
                   LibCCEC::getInstance().term();
                }
                catch (const std::exception e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().term() ");
                }
            }

            libcecInitStatus--;

            return;
        }


        void HdmiCec_2::getPhysicalAddress()
        {
            LOGINFO("Entered getPhysicalAddress ");

            uint32_t physAddress = 0x0F0F0F0F;

            try {
                    LibCCEC::getInstance().getPhysicalAddress(&physAddress);
                    physical_addr = {(uint8_t)((physAddress >> 24) & 0xFF),(uint8_t)((physAddress >> 16) & 0xFF),(uint8_t) ((physAddress >> 8)  & 0xFF),(uint8_t)((physAddress) & 0xFF)};
                    LOGINFO("getPhysicalAddress: physicalAddress: %s ", physical_addr.toString().c_str());
            }
            catch (const std::exception e)
            {
                LOGWARN("exception caught from getPhysicalAddress");
            }
            return;
        }

        void HdmiCec_2::getLogicalAddress()
        {
            LOGINFO("Entered getLogicalAddress ");

            try{
                LogicalAddress addr = LibCCEC::getInstance().getLogicalAddress(DEV_TYPE_TUNER);

                std::string logicalAddrDeviceType = DeviceType(LogicalAddress(addr).getType()).toString().c_str();

                LOGINFO("logical address obtained is %d , saved logical address is %d ", addr.toInt(), logicalAddress.toInt());

                if (logicalAddress.toInt() != addr.toInt() || logicalAddressDeviceType != logicalAddrDeviceType)

                {
                    logicalAddress = addr;
                    logicalAddressDeviceType = logicalAddrDeviceType;
                }
            }
            catch (const std::exception e)
            {
                LOGWARN("CEC exception caught from getLogicalAddress ");
            }
            return;
        }

        bool HdmiCec_2::getEnabled()
        {
            if(true == cecEnableStatus)
                return true;
            else
                return false;
            LOGINFO("getEnabled :%d ",cecEnableStatus);
        }

        bool HdmiCec_2::getOTPEnabled()
        {
            if(true == cecOTPSettingEnabled)
                return true;
            else
                return false;
            LOGINFO("getOTPEnabled :%d ",cecOTPSettingEnabled);
        }

        bool HdmiCec_2::performOTPAction()
        {
            LOGINFO("performOTPAction ");
            bool ret = false; 
            if((true == cecEnableStatus) && (cecOTPSettingEnabled == true))
            {
                try
                {
                    if(tvPowerState.toInt())
                    {
                       LOGINFO("Command: sending ImageViewOn TV \r\n");
                       smConnection->sendTo(LogicalAddress(LogicalAddress::TV), MessageEncoder().encode(ImageViewOn()), 5000);
                       usleep(10000);
                    }
                    if(!isDeviceActiveSource)
                    {
                        LOGINFO("Command: sending ActiveSource  physical_addr :%s \r\n",physical_addr.toString().c_str());
                        smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ActiveSource(physical_addr)), 5000);
                        usleep(10000);
                        isDeviceActiveSource = true;
                    }
                    LOGINFO("Command: sending GiveDevicePowerStatus \r\n");
                    smConnection->sendTo(LogicalAddress(LogicalAddress::TV), MessageEncoder().encode(GiveDevicePowerStatus()), 5000);
                    ret = true;
                }
                catch(...)
                {
                    LOGWARN("Exception while processing performOTPAction");
                }
            }
            else
                LOGWARN("cecEnableStatus=false");
            return ret;
        }
    } // namespace Plugin
} // namespace WPEFramework
