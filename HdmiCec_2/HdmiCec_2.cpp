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
#include "UtilsIarm.h"

#define HDMICEC2_METHOD_SET_ENABLED "setEnabled"
#define HDMICEC2_METHOD_GET_ENABLED "getEnabled"
#define HDMICEC2_METHOD_OTP_SET_ENABLED "setOTPEnabled"
#define HDMICEC2_METHOD_OTP_GET_ENABLED "getOTPEnabled"
#define HDMICEC2_METHOD_SET_OSD_NAME "setOSDName"
#define HDMICEC2_METHOD_GET_OSD_NAME "getOSDName"
#define HDMICEC2_METHOD_SET_VENDOR_ID "setVendorId"
#define HDMICEC2_METHOD_GET_VENDOR_ID "getVendorId"
#define HDMICEC2_METHOD_PERFORM_OTP_ACTION "performOTPAction"
#define HDMICEC2_METHOD_SEND_STANDBY_MESSAGE "sendStandbyMessage"
#define HDMICEC2_METHOD_GET_ACTIVE_SOURCE_STATUS "getActiveSourceStatus"

#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define HDMICEC_EVENT_ON_STANDBY_MSG_RECEIVED "standbyMessageReceived"
#define DEV_TYPE_TUNER 1
#define HDMI_HOT_PLUG_EVENT_CONNECTED 0
#define ABORT_REASON_ID 4

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

enum {
	HDMICEC2_EVENT_DEVICE_ADDED=0,
	HDMICEC2_EVENT_DEVICE_REMOVED,
	HDMICEC2_EVENT_DEVICE_INFO_UPDATED,
        HDMICEC2_EVENT_ACTIVE_SOURCE_STATUS_UPDATED,
};

static char *eventString[] = {
	"onDeviceAdded",
	"onDeviceRemoved",
	"onDeviceInfoUpdated",
        "onActiveSourceStatusUpdated"
};

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
    namespace {

        static Plugin::Metadata<Plugin::HdmiCec_2> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiCec_2, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

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
             HdmiCec_2::_instance->sendActiveSourceEvent();
             HdmiCec_2::_instance->addDevice(header.from.toInt());
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
             HdmiCec_2::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec_2Processor::process (const TextViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: TextViewOn\n");
             HdmiCec_2::_instance->addDevice(header.from.toInt());
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
             LOGINFO("Command: Standby from %s\n", header.from.toString().c_str());
             HdmiCec_2::_instance->SendStandbyMsgEvent(header.from.toInt());

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
             HdmiCec_2::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec_2Processor::process (const SetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetMenuLanguage Language : %s \n",msg.language.toString().c_str());
       }
       void HdmiCec_2Processor::process (const GiveOSDName &msg, const Header &header)
       {
             printHeader(header);
             if (!(header.from == LogicalAddress(LogicalAddress::UNREGISTERED)))
             {
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
       }
       void HdmiCec_2Processor::process (const GivePhysicalAddress &msg, const Header &header)
       {
             LOGINFO("Command: GivePhysicalAddress\n");
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
             if (HdmiCec_2::_instance) {
                 bool isOSDNameUpdated = HdmiCec_2::_instance->deviceList[header.from.toInt()].update(msg.osdName);
                 if (isOSDNameUpdated)
                     HdmiCec_2::_instance->sendDeviceUpdateInfo(header.from.toInt());
             } else {
                 LOGWARN("Exception HdmiCec_2::_instance NULL");
             }
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
             HdmiCec_2::_instance->sendActiveSourceEvent();
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
             HdmiCec_2::_instance->sendActiveSourceEvent();
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
             HdmiCec_2::_instance->sendActiveSourceEvent();

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
             HdmiCec_2::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec_2Processor::process (const DeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());
             if (HdmiCec_2::_instance){
                 bool isVendorIdUpdated = HdmiCec_2::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
                 if (isVendorIdUpdated)
                     HdmiCec_2::_instance->sendDeviceUpdateInfo(header.from.toInt());
             }
             else {
                 LOGWARN("Exception HdmiCec_2::_instance NULL");
             }

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
             HdmiCec_2::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec_2Processor::process (const FeatureAbort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: FeatureAbort\n");
       }
       void HdmiCec_2Processor::process (const Abort &msg, const Header &header)
       {
             printHeader(header);
             if (!(header.from == LogicalAddress(LogicalAddress::BROADCAST)))
             {
		 LOGINFO("Command: Abort, sending FeatureAbort");
		 try
		 { 
		     conn.sendTo(header.from, MessageEncoder().encode(FeatureAbort(OpCode(msg.opCode()),AbortReason(ABORT_REASON_ID))));
		 } 
		 catch(...)
		 {
		     LOGWARN("Exception while sending FeatureAbort command");
		 }

             }
             LOGINFO("Command: Abort\n");
       }
       void HdmiCec_2Processor::process (const Polling &msg, const Header &header)                                 {
             printHeader(header);
             LOGINFO("Command: Polling\n");
       }


//=========================================== HdmiCec_2 =========================================

       HdmiCec_2::HdmiCec_2()
       : PluginHost::JSONRPC()
       {
           LOGWARN("ctor");
           IsCecMgrActivated = false;
           Register(HDMICEC2_METHOD_SET_ENABLED, &HdmiCec_2::setEnabledWrapper, this);
           Register(HDMICEC2_METHOD_GET_ENABLED, &HdmiCec_2::getEnabledWrapper, this);
           Register(HDMICEC2_METHOD_OTP_SET_ENABLED, &HdmiCec_2::setOTPEnabledWrapper, this);
           Register(HDMICEC2_METHOD_OTP_GET_ENABLED, &HdmiCec_2::getOTPEnabledWrapper, this);
           Register(HDMICEC2_METHOD_SET_OSD_NAME, &HdmiCec_2::setOSDNameWrapper, this);
           Register(HDMICEC2_METHOD_GET_OSD_NAME, &HdmiCec_2::getOSDNameWrapper, this);
           Register(HDMICEC2_METHOD_SET_VENDOR_ID, &HdmiCec_2::setVendorIdWrapper, this);
           Register(HDMICEC2_METHOD_GET_VENDOR_ID, &HdmiCec_2::getVendorIdWrapper, this);
           Register(HDMICEC2_METHOD_PERFORM_OTP_ACTION, &HdmiCec_2::performOTPActionWrapper, this);
           Register(HDMICEC2_METHOD_SEND_STANDBY_MESSAGE, &HdmiCec_2::sendStandbyMessageWrapper, this);
           Register(HDMICEC2_METHOD_GET_ACTIVE_SOURCE_STATUS, &HdmiCec_2::getActiveSourceStatus, this);
           Register("getDeviceList", &HdmiCec_2::getDeviceList, this);

       }

       HdmiCec_2::~HdmiCec_2()
       {
           IsCecMgrActivated = false;
           LOGWARN("dtor");
       }
 
       const string HdmiCec_2::Initialize(PluginHost::IShell* /* service */)
       {
           LOGWARN("Initlaizing CEC_2");
           string msg;
           HdmiCec_2::_instance = this;
           smConnection = NULL;
           IsCecMgrActivated = false;
           if (Utils::IARM::init()) {


               //Initialize cecEnableStatus to false in ctor
               cecEnableStatus = false;

               logicalAddressDeviceType = "None";
               logicalAddress = 0xFF;


               char c;
               IARM_Result_t retVal = IARM_RESULT_SUCCESS;
               retVal = IARM_Bus_Call_with_IPCTimeout(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_API_isAvailable, (void *)&c, sizeof(c), 1000);
               if(retVal != IARM_RESULT_SUCCESS) {
                   msg = "IARM_BUS_CECMGR is not available";
                   LOGINFO("CECMGR is not available. Failed to activate HdmiCec_2 Plugin");
                   return msg;
               } else {
                   LOGINFO("CECMGR is available. Activate HdmiCec_2 Plugin. IsCecMgrActivated: %d", IsCecMgrActivated);
                   IsCecMgrActivated = true;
               }

               //CEC plugin functionalities will only work if CECmgr is available. If plugin Initialize failure upper layer will call dtor directly.
               InitializeIARM();

               // load persistence setting
               loadSettings();

               try
               {
                   //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                   device::Manager::Initialize();
                   std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                   device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
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
           } else {
               msg = "IARM bus is not available";
               LOGERR("IARM bus is not available. Failed to activate HdmiCec_2 Plugin");
           }

           // On success return empty, to indicate there is no error text.
           return msg;
       }

       uint32_t HdmiCec_2::getActiveSourceStatus(const JsonObject& parameters, JsonObject& response)
       {
            response["status"] = isDeviceActiveSource;
            returnResponse(true);
       }

       void HdmiCec_2::Deinitialize(PluginHost::IShell* /* service */)
       {
           LOGWARN("Deinitialize CEC_2");
           if(true == getEnabled())
           {
               setEnabled(false,false);
           }
           isDeviceActiveSource = false;
           HdmiCec_2::_instance->sendActiveSourceEvent();
           HdmiCec_2::_instance = nullptr;
           smConnection = NULL;
           DeinitializeIARM();
       }

       void HdmiCec_2::SendStandbyMsgEvent(const int logicalAddress)
       {
           JsonObject params;
           params["logicalAddress"] = JsonValue(logicalAddress);
           sendNotify(HDMICEC_EVENT_ON_STANDBY_MSG_RECEIVED, params);
       }
 
       uint32_t HdmiCec_2::sendStandbyMessageWrapper(const JsonObject& parameters, JsonObject& response)
       {
	   if(sendStandbyMessage())
	   { 
               returnResponse(true);
	   }  
	   else
	   {
	       returnResponse(false);
	   } 
       }
 
       bool HdmiCec_2::sendStandbyMessage()
       {
            bool ret = false;
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return ret;
            }
            if(true == cecEnableStatus)
            {
                if (smConnection){
                   try
                   {
                       smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(Standby()));
		       ret = true;
                   }
                   catch(...)
                   {
                       LOGWARN("Exception while sending CEC StandBy Message");
                   }
                }
                else {
                    LOGWARN("smConnection is NULL");
                }
            }
            else
                LOGWARN("cecEnableStatus=false");
	    return ret;
       }


       const void HdmiCec_2::InitializeIARM()
       {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
       }

       void HdmiCec_2::DeinitializeIARM()
       {
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
            if(!HdmiCec_2::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);
                HdmiCec_2::_instance->onHdmiHotPlug(hdmi_hotplug_event);
                //Trigger CEC device poll here
                pthread_cond_signal(&(_instance->m_condSig));
            }
       }

       void HdmiCec_2::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
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
                    }
                    else
                        powerState = 1;

                }
           }
       }

       void HdmiCec_2::onCECDaemonInit()
       {
            if(true == getEnabled())
            {
                LOGINFO("%s %d. Calling setEnabled false", __func__, __LINE__);
                setEnabled(false,false);
                LOGINFO("%s %d. Calling setEnabled true", __func__, __LINE__);
                setEnabled(true,false);
            }
            else
            {
                /*Do nothing as CEC is not already enabled*/
            }
       }

       void HdmiCec_2::cecStatusUpdated(void *evtStatus)
       {
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            }
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

       void HdmiCec_2::sendActiveSourceEvent()
       {
           JsonObject params;
           params["status"] = isDeviceActiveSource;
           LOGWARN(" sendActiveSourceEvent isDeviceActiveSource: %d ",isDeviceActiveSource);
           sendNotify(eventString[HDMICEC2_EVENT_ACTIVE_SOURCE_STATUS_UPDATED], params);
       }

       void HdmiCec_2::onHdmiHotPlug(int connectStatus)
       {
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            }
            if (HDMI_HOT_PLUG_EVENT_CONNECTED == connectStatus)
            {
                LOGINFO ("onHdmiHotPlug Status : %d ", connectStatus);
                getPhysicalAddress();
                getLogicalAddress();
                try
                {
                   std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                   device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
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
                 if(smConnection)
                 {
                     try
                     {
                         LOGINFO(" sending ReportPhysicalAddress response physical_addr :%s logicalAddress :%x \n",physical_addr.toString().c_str(), logicalAddress.toInt());
                         smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ReportPhysicalAddress(physical_addr,logicalAddress.toInt()))); 

                         LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n", \
                             (isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                         if(isLGTvConnected)
                             smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                         else 
                             smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));
                     } 
                     catch(...)
                     {
                         LOGWARN("Exception while sending Messages onHdmiHotPlug\n");
                     }
                 }
            }
            return;
       }

       uint32_t HdmiCec_2::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
           LOGINFOMETHOD();

            bool enabled = false;

            if (parameters.HasLabel("enabled"))
            {
                getBoolParameter("enabled", enabled);
            }
            else
            {
                returnResponse(false);
            }

            setEnabled(enabled,true);
            returnResponse(true);
       }

       uint32_t HdmiCec_2::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getEnabled();
            returnResponse(true);
       }
       uint32_t HdmiCec_2::setOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
           LOGINFOMETHOD();

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
           LOGINFOMETHOD();
            bool enabled = false;

            if (parameters.HasLabel("name"))
            {
                std::string osd = parameters["name"].String();
                LOGINFO("setOSDNameWrapper osdName: %s",osd.c_str());
                osdName = osd.c_str();
                Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_OSD_NAME, JsonValue(osd.c_str()));
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
            LOGINFOMETHOD();

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

                Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_VENDOR_ID, JsonValue(vendorID));
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

        void HdmiCec_2::setEnabled(bool enabled, bool isPersist)
        {
           LOGINFO("Entered setEnabled ");

           if (!IsCecMgrActivated) {
               LOGWARN("CEC Mgr not activated CEC communication is not possible");
               return;
           } else {
               LOGWARN("CEC Mgr activated. proceeding with %s", __func__);
           }
           if (isPersist && (cecSettingEnabled != enabled))
           {
               Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_ENABLED, JsonValue(enabled));
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
           if (!IsCecMgrActivated) {
               LOGWARN("CEC Mgr not activated CEC communication is not possible");
               return;
           }
           if (cecOTPSettingEnabled != enabled)
           {
               LOGINFO("persist setOTPEnabled ");
               Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_OTP_ENABLED, JsonValue(enabled));
               cecOTPSettingEnabled = enabled;
           }
           return;
        }

        void HdmiCec_2::CECEnable(void)
        {
            LOGINFO("Entered CECEnable");
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            }
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
                smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(GiveDevicePowerStatus()));
                LOGINFO("Command: sending request active Source isDeviceActiveSource is set to false\r\n");
                smConnection->sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(RequestActiveSource()));
                isDeviceActiveSource = false;
                LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n", \
                                                 (isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                if(isLGTvConnected)
                    smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                else 
                    smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));

                LOGWARN("Start Update thread %p", smConnection );
                m_updateThreadExit = false;
                _instance->m_lockUpdate = PTHREAD_MUTEX_INITIALIZER;
                _instance->m_condSigUpdate = PTHREAD_COND_INITIALIZER;
                m_UpdateThread = std::thread(threadUpdateCheck);

                LOGWARN("Start Thread %p", smConnection );
                m_pollThreadExit = false;
                _instance->m_numberOfDevices = 0;
                _instance->m_lock = PTHREAD_MUTEX_INITIALIZER;
                _instance->m_condSig = PTHREAD_COND_INITIALIZER;
                m_pollThread = std::thread(threadRun);

            }
            return;
        }

        void HdmiCec_2::CECDisable(void)
        {
            LOGINFO("Entered CECDisable ");

            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            } else {
                LOGWARN("CEC Mgr activated. proceeding with %s", __func__);
            }
            if(!cecEnableStatus)
            {
                LOGWARN("CEC Already Disabled ");
                return;
            }

            if (smConnection != NULL)
            {
                LOGWARN("Stop Thread %p", smConnection );

                m_updateThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_cond_signal(&(_instance->m_condSigUpdate));
                try {
                    if (m_UpdateThread.joinable()) {
                       LOGWARN("Join update Thread %p", smConnection );
                       m_UpdateThread.join();
                    }
                }
                catch(const std::system_error& e) {
                    LOGERR("system_error exception in thread join %s", e.what());
                }
                catch(const std::exception& e) {
                    LOGERR("exception in thread join %s", e.what());
                }
                LOGWARN("Deleted update Thread %p", smConnection );

                m_pollThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_cond_signal(&(_instance->m_condSig));
                try {
                    if (m_pollThread.joinable()) {
                       LOGWARN("Join Thread %p", smConnection );
                       m_pollThread.join();
                    }
                }
                catch(const std::system_error& e) {
                    LOGERR("system_error exception in thread join %s", e.what());
                }
                catch(const std::exception& e) {
                    LOGERR("exception in thread join %s", e.what());
                }
                LOGWARN("Deleted Thread %p", smConnection );
                //Clear cec device cache.
                removeAllCecDevices();

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
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            }

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

            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return;
            }
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
            LOGINFO("getEnabled :%d ",cecEnableStatus);
            return cecEnableStatus;
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
            if (!IsCecMgrActivated) {
                LOGWARN("CEC Mgr not activated CEC communication is not possible");
                return ret;
            }
            if((true == cecEnableStatus) && (cecOTPSettingEnabled == true))
            {
                if (smConnection)  {
                    try
                    {
                        LOGINFO("Command: sending ImageViewOn TV \r\n");
                        smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(ImageViewOn()));
                        usleep(10000);
                        LOGINFO("Command: sending ActiveSource  physical_addr :%s \r\n",physical_addr.toString().c_str());
                        smConnection->sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(ActiveSource(physical_addr)));
                        usleep(10000);
                        isDeviceActiveSource = true;
                        LOGINFO("Command: sending GiveDevicePowerStatus \r\n");
                        smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(GiveDevicePowerStatus()));
                        ret = true;
                    }
                    catch(...)
                    {
                        LOGWARN("Exception while processing performOTPAction");
                    }
                }
                else {
                    LOGWARN("smConnection is NULL");
                }
            }
            else
                LOGWARN("cecEnableStatus=false");
            return ret;
        }

        uint32_t HdmiCec_2::getDeviceList (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
		LOGINFOMETHOD();
		//Trigger CEC device poll here
		pthread_cond_signal(&(_instance->m_condSig));

		bool success = true;
		response["numberofdevices"] = HdmiCec_2::_instance->m_numberOfDevices;
		LOGINFO("getDeviceListWrapper  m_numberOfDevices :%d \n", HdmiCec_2::_instance->m_numberOfDevices);
		JsonArray deviceListArg;
		try
		{
			int i = 0;
			for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				if (BIT_CHECK(deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
					JsonObject device;
					device["logicalAddress"] = HdmiCec_2::_instance->deviceList[i].m_logicalAddress.toInt();
					device["osdName"] = HdmiCec_2::_instance->deviceList[i].m_osdName.toString().c_str();
					device["vendorID"] = HdmiCec_2::_instance->deviceList[i].m_vendorID.toString().c_str();
					deviceListArg.Add(device);
				}
			}
		}
		catch (...)
		{
			LOGERR("Exception in api");
			success = false;
		}
		response["deviceList"] = deviceListArg;
		returnResponse(success);
	}

	bool HdmiCec_2::pingDeviceUpdateList (int idev)
	{
		bool isConnected = false;
		if(!HdmiCec_2::_instance)
		{
			LOGERR("HdmiCec_2::_instance not existing");
			return isConnected;
		}
		if ( !(_instance->smConnection) || logicalAddress.toInt() == LogicalAddress::UNREGISTERED || (false==cecEnableStatus)){
			LOGERR("Exiting from pingDeviceUpdateList _instance->smConnection:%p, logicalAddress:%d, cecEnableStatus=%d",
					_instance->smConnection, logicalAddress.toInt(), cecEnableStatus);
			return isConnected;
		}

		LOGWARN("PING for  0x%x \r\n",idev);
		try {
			_instance->smConnection->ping(logicalAddress, LogicalAddress(idev), Throw_e());
		}
		catch(CECNoAckException &e)
		{
			if (BIT_CHECK(_instance->deviceList[idev].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
				LOGINFO("Device disconnected: %d \r\n",idev);
				removeDevice (idev);
			} else {
				LOGINFO("Device is not connected: %d. Ping caught %s\r\n",idev, e.what());
			}
			isConnected = false;
			return isConnected;;
		}
		catch(IOException &e)
		{
			LOGINFO("Device is not reachable: %d. Ping caught %s\r\n",idev, e.what());
			isConnected = false;
			return isConnected;;
		}
		catch(Exception &e)
		{
			LOGINFO("Ping caught %s \r\n",e.what());
		}

		/* If we get ACK, then the device is present in the network*/
		isConnected = true;
		if ( !(BIT_CHECK(_instance->deviceList[idev].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		{
			LOGINFO("Device connected: %d \r\n",idev);
			addDevice (idev);
		}
		return isConnected;
	}

	void  HdmiCec_2::sendDeviceUpdateInfo(const int logicalAddress)
	{
		JsonObject params;
		params["logicalAddress"] = JsonValue(logicalAddress);
		LOGINFO("Device info updated notification send: for logical address:%d\r\n", logicalAddress);
		sendNotify(eventString[HDMICEC2_EVENT_DEVICE_INFO_UPDATED], params);
	}

	void HdmiCec_2::addDevice(const int logicalAddress) {

		if(!HdmiCec_2::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if ( !(BIT_CHECK(HdmiCec_2::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		 {
			BIT_SET(HdmiCec_2::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT);
			HdmiCec_2::_instance->deviceList[logicalAddress].m_logicalAddress = LogicalAddress(logicalAddress);
			HdmiCec_2::_instance->m_numberOfDevices++;
			LOGINFO("New cec ligical address add notification send:  \r\n");
			sendNotify(eventString[HDMICEC2_EVENT_DEVICE_ADDED], JsonObject());
		 }
		//Two source devices can have same logical address.
		requestCecDevDetails(logicalAddress);
	}

	void HdmiCec_2::removeAllCecDevices() {
		int i = 0;
		for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
			removeDevice (i);
		}
	}
	void HdmiCec_2::removeDevice(const int logicalAddress) {
		if(!HdmiCec_2::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED ){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if (BIT_CHECK(HdmiCec_2::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
		{
			_instance->m_numberOfDevices--;
			_instance->deviceList[logicalAddress].clear();
			LOGINFO("Cec ligical address remove notification send:  \r\n");
			sendNotify(eventString[HDMICEC2_EVENT_DEVICE_REMOVED], JsonObject());
		}
	}

	void HdmiCec_2::sendUnencryptMsg(unsigned char* msg, int size)
	{
		LOGINFO("sendMessage ");

		if(true == cecEnableStatus)
		{
			std::vector <unsigned char> buf;
			buf.resize(size);

			int itr = 0;
			for (itr= 0; itr<size; itr++)
				buf [itr] = msg [itr];

			CECFrame frame = CECFrame((const uint8_t *)buf.data(), size);
			//      SVCLOG_WARN("Frame to be sent from servicemanager in %s \n",__FUNCTION__);
			//      frame.hexDump();
			smConnection->sendAsync(frame);
		}
		else
			LOGWARN("cecEnableStatus=false");
		return;
	}

	void HdmiCec_2::requestVendorID(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned int logicalAddr = logicalAddress.toInt();
		unsigned char sender = (unsigned char)(logicalAddr & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request vendor id
		msg [1] = 0x8c;
		LOGINFO("Sending msg request vendor id %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCec_2::requestOsdName(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned int logicalAddr = logicalAddress.toInt();
		unsigned char sender = (unsigned char)(logicalAddr & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request OSD  name
		msg [1] = 0x46;
		LOGINFO("Sending msg request osd name %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCec_2::requestCecDevDetails(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		requestVendorID (newDevlogicalAddress);
		requestOsdName (newDevlogicalAddress);
	}

	void HdmiCec_2::threadRun()
	{
		if(!HdmiCec_2::_instance)
			return;
		if(!(_instance->smConnection))
			return;
		LOGINFO("Entering ThreadRun: _instance->m_pollThreadExit %d",_instance->m_pollThreadExit);
		int i = 0;
		pthread_mutex_lock(&(_instance->m_lock));//pthread_cond_wait should be mutex protected. //pthread_cond_wait will unlock the mutex and perfoms wait for the condition.
		while (!_instance->m_pollThreadExit) {
			bool isActivateUpdateThread = false;
			LOGINFO("Starting cec device polling");
			for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				bool isConnected = _instance->pingDeviceUpdateList(i);
				if (isConnected){
					isActivateUpdateThread = isConnected;
				}

			}
			if (isActivateUpdateThread){
				//i any of devices is connected activate thread update check
				pthread_cond_signal(&(_instance->m_condSigUpdate));
			}
			//Wait for mutex signal here to continue the worker thread again.
			pthread_cond_wait(&(_instance->m_condSig), &(_instance->m_lock));

		}
		pthread_mutex_unlock(&(_instance->m_lock));
	}

	void HdmiCec_2::threadUpdateCheck()
	{
		if(!HdmiCec_2::_instance)
			return;
		if(!(_instance->smConnection))
			return;
		LOGINFO("Entering ThreadUpdate: _instance->m_updateThreadExit %d",_instance->m_updateThreadExit);
		int i = 0;
		pthread_mutex_lock(&(_instance->m_lockUpdate));//pthread_cond_wait should be mutex protected. //pthread_cond_wait will unlock the mutex and perfoms wait for the condition.
		while (!_instance->m_updateThreadExit) {
			//Wait for mutex signal here to continue the worker thread again.
			pthread_cond_wait(&(_instance->m_condSigUpdate), &(_instance->m_lockUpdate));

			LOGINFO("Starting cec device update check");
			for(i=0; ((i< LogicalAddress::UNREGISTERED)&&(!_instance->m_updateThreadExit)); i++ ) {
				//If details are not updated. update now.
				if (BIT_CHECK(HdmiCec_2::_instance->deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
				{
					int itr = 0;
					bool retry = true;
					int iCounter = 0;
					for (itr = 0; ((itr<5)&&(retry)); itr++){

						if (!HdmiCec_2::_instance->deviceList[i].m_isOSDNameUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 2sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCec_2::_instance->requestOsdName (i);
							retry = true;
						}
						else {
							retry = false;
						}

						if (!HdmiCec_2::_instance->deviceList[i].m_isVendorIDUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 1sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCec_2::_instance->requestVendorID (i);
							retry = true;
						}
					}
					if (retry){
						LOGINFO("cec device: %d update time out", i);
					}
				}
			}

		}
		pthread_mutex_unlock(&(_instance->m_lockUpdate));
	}

    } // namespace Plugin
} // namespace WPEFramework
