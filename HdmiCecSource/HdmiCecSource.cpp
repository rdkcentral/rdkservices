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

#include "HdmiCecSource.h"


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

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilssyncPersistFile.h"

#define HDMICECSOURCE_METHOD_SET_ENABLED "setEnabled"
#define HDMICECSOURCE_METHOD_GET_ENABLED "getEnabled"
#define HDMICECSOURCE_METHOD_OTP_SET_ENABLED "setOTPEnabled"
#define HDMICECSOURCE_METHOD_OTP_GET_ENABLED "getOTPEnabled"
#define HDMICECSOURCE_METHOD_SET_OSD_NAME "setOSDName"
#define HDMICECSOURCE_METHOD_GET_OSD_NAME "getOSDName"
#define HDMICECSOURCE_METHOD_SET_VENDOR_ID "setVendorId"
#define HDMICECSOURCE_METHOD_GET_VENDOR_ID "getVendorId"
#define HDMICECSOURCE_METHOD_PERFORM_OTP_ACTION "performOTPAction"
#define HDMICECSOURCE_METHOD_SEND_STANDBY_MESSAGE "sendStandbyMessage"
#define HDMICECSOURCE_METHOD_GET_ACTIVE_SOURCE_STATUS "getActiveSourceStatus"
#define HDMICECSOURCE_METHOD_SEND_KEY_PRESS         "sendKeyPressEvent"
#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define HDMICEC_EVENT_ON_STANDBY_MSG_RECEIVED "standbyMessageReceived"
#define DEV_TYPE_TUNER 1
#define HDMI_HOT_PLUG_EVENT_CONNECTED 0
#define ABORT_REASON_ID 4

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 6

enum {
	HDMICECSOURCE_EVENT_DEVICE_ADDED=0,
	HDMICECSOURCE_EVENT_DEVICE_REMOVED,
	HDMICECSOURCE_EVENT_DEVICE_INFO_UPDATED,
    HDMICECSOURCE_EVENT_ACTIVE_SOURCE_STATUS_UPDATED,
};

static const char *eventString[] = {
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

static std::vector<uint8_t> defaultVendorId = {0x00,0x19,0xFB};
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

        static Plugin::Metadata<Plugin::HdmiCecSource> metadata(
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
        SERVICE_REGISTRATION(HdmiCecSource, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        HdmiCecSource* HdmiCecSource::_instance = nullptr;
        static int libcecInitStatus = 0;

//=========================================== HdmiCecSourceFrameListener =========================================
        void HdmiCecSourceFrameListener::notify(const CECFrame &in) const {
                const uint8_t *buf = NULL;
                char strBuffer[512] = {0}; 
                size_t len = 0;

                in.getBuffer(&buf, &len);
                for (unsigned int i = 0; i < len; i++) {
                   snprintf(strBuffer + (i*3) , sizeof(strBuffer) - (i*3), "%02X ",(uint8_t) *(buf + i));
                }
                LOGINFO("   >>>>>    Received CEC Frame: :%s \n",strBuffer);

                MessageDecoder(processor).decode(in);
       }

//=========================================== HdmiCecSourceProcessor =========================================
       void HdmiCecSourceProcessor::process (const ActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ActiveSource %s : %s  : %s \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
             if(msg.physicalAddress.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("ActiveSource isDeviceActiveSource status :%d \n", isDeviceActiveSource);
             HdmiCecSource::_instance->sendActiveSourceEvent();
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const InActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: InActiveSource %s : %s : %s  \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
       }
       void HdmiCecSourceProcessor::process (const ImageViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ImageViewOn \n");
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const TextViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: TextViewOn\n");
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const RequestActiveSource &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const Standby &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: Standby from %s\n", header.from.toString().c_str());
             HdmiCecSource::_instance->SendStandbyMsgEvent(header.from.toInt());

       }
       void HdmiCecSourceProcessor::process (const GetCECVersion &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const CECVersion &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const SetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetMenuLanguage Language : %s \n",msg.language.toString().c_str());
       }
       void HdmiCecSourceProcessor::process (const GiveOSDName &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const GivePhysicalAddress &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const GiveDeviceVendorID &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const SetOSDString &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetOSDString OSDString : %s\n",msg.osdString.toString().c_str());
       }
       void HdmiCecSourceProcessor::process (const SetOSDName &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());
             if (HdmiCecSource::_instance) {
                 bool isOSDNameUpdated = HdmiCecSource::_instance->deviceList[header.from.toInt()].update(msg.osdName);
                 if (isOSDNameUpdated)
                     HdmiCecSource::_instance->sendDeviceUpdateInfo(header.from.toInt());
             } else {
                 LOGWARN("Exception HdmiCecSource::_instance NULL");
             }
       }
       void HdmiCecSourceProcessor::process (const RoutingChange &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingChange From : %s To: %s \n",msg.from.toString().c_str(),msg.to.toString().c_str());
             if(msg.to.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSource::_instance->sendActiveSourceEvent();
       }
       void HdmiCecSourceProcessor::process (const RoutingInformation &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingInformation Routing Information to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSource::_instance->sendActiveSourceEvent();
       }
       void HdmiCecSourceProcessor::process (const SetStreamPath &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetStreamPath Set Stream Path to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSource::_instance->sendActiveSourceEvent();

       }
       void HdmiCecSourceProcessor::process (const GetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GetMenuLanguage\n");
       }
       void HdmiCecSourceProcessor::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ReportPhysicalAddress\n");
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const DeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());
             if (HdmiCecSource::_instance){
                 bool isVendorIdUpdated = HdmiCecSource::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
                 if (isVendorIdUpdated)
                     HdmiCecSource::_instance->sendDeviceUpdateInfo(header.from.toInt());
             }
             else {
                 LOGWARN("Exception HdmiCecSource::_instance NULL");
             }

       }
       void HdmiCecSourceProcessor::process (const GiveDevicePowerStatus &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const ReportPowerStatus &msg, const Header &header)
       {
             printHeader(header);
             if ((header.from == LogicalAddress(LogicalAddress::TV)))
                 tvPowerState = msg.status; 
             LOGINFO("Command: ReportPowerStatus TV Power Status from:%s status : %s \n",header.from.toString().c_str(),msg.status.toString().c_str());
             HdmiCecSource::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const FeatureAbort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: FeatureAbort\n");
       }
       void HdmiCecSourceProcessor::process (const Abort &msg, const Header &header)
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
       void HdmiCecSourceProcessor::process (const Polling &msg, const Header &header)                                 {
             printHeader(header);
             LOGINFO("Command: Polling\n");
       }


//=========================================== HdmiCecSource =========================================

       HdmiCecSource::HdmiCecSource()
       : PluginHost::JSONRPC(),cecEnableStatus(false),smConnection(nullptr), m_sendKeyEventThreadRun(false)
       {
           LOGWARN("ctor");
           smConnection = NULL;
           cecEnableStatus = false;
           IsCecMgrActivated = false;
           Register(HDMICECSOURCE_METHOD_SET_ENABLED, &HdmiCecSource::setEnabledWrapper, this);
           Register(HDMICECSOURCE_METHOD_GET_ENABLED, &HdmiCecSource::getEnabledWrapper, this);
           Register(HDMICECSOURCE_METHOD_OTP_SET_ENABLED, &HdmiCecSource::setOTPEnabledWrapper, this);
           Register(HDMICECSOURCE_METHOD_OTP_GET_ENABLED, &HdmiCecSource::getOTPEnabledWrapper, this);
           Register(HDMICECSOURCE_METHOD_SET_OSD_NAME, &HdmiCecSource::setOSDNameWrapper, this);
           Register(HDMICECSOURCE_METHOD_GET_OSD_NAME, &HdmiCecSource::getOSDNameWrapper, this);
           Register(HDMICECSOURCE_METHOD_SET_VENDOR_ID, &HdmiCecSource::setVendorIdWrapper, this);
           Register(HDMICECSOURCE_METHOD_GET_VENDOR_ID, &HdmiCecSource::getVendorIdWrapper, this);
           Register(HDMICECSOURCE_METHOD_PERFORM_OTP_ACTION, &HdmiCecSource::performOTPActionWrapper, this);
           Register(HDMICECSOURCE_METHOD_SEND_STANDBY_MESSAGE, &HdmiCecSource::sendStandbyMessageWrapper, this);
           Register(HDMICECSOURCE_METHOD_GET_ACTIVE_SOURCE_STATUS, &HdmiCecSource::getActiveSourceStatus, this);
           Register(HDMICECSOURCE_METHOD_SEND_KEY_PRESS,&HdmiCecSource::sendRemoteKeyPressWrapper,this);
           Register("getDeviceList", &HdmiCecSource::getDeviceList, this);

       }

       HdmiCecSource::~HdmiCecSource()
       {
           IsCecMgrActivated = false;
           LOGWARN("dtor");

       }
 
       const string HdmiCecSource::Initialize(PluginHost::IShell* /* service */)
       {
           LOGWARN("Initlaizing CEC_2");
           string msg;
           HdmiCecSource::_instance = this;
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
                   LOGINFO("CECMGR is not available. Failed to activate HdmiCecSource Plugin");
                   return msg;
               } else {
                   LOGINFO("CECMGR is available. Activate HdmiCecSource Plugin. IsCecMgrActivated: %d", IsCecMgrActivated);
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
                       std::vector<uint8_t> edidVec;
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
               LOGERR("IARM bus is not available. Failed to activate HdmiCecSource Plugin");
           }

           // On success return empty, to indicate there is no error text.
           return msg;
       }

       uint32_t HdmiCecSource::getActiveSourceStatus(const JsonObject& parameters, JsonObject& response)
       {
            response["status"] = isDeviceActiveSource;
            returnResponse(true);
       }

       void HdmiCecSource::Deinitialize(PluginHost::IShell* /* service */)
       {
           LOGWARN("Deinitialize CEC_2");
           if(true == getEnabled())
           {
               setEnabled(false,false);
           }
           isDeviceActiveSource = false;
           HdmiCecSource::_instance->sendActiveSourceEvent();
           HdmiCecSource::_instance = nullptr;
           smConnection = NULL;

           DeinitializeIARM();
       }
       
	    void HdmiCecSource::sendKeyPressEvent(const int logicalAddress, int keyCode)
		{
			if(!(_instance->smConnection))
                 return;
		    LOGINFO(" sendKeyPressEvent logicalAddress 0x%x keycode 0x%x\n",logicalAddress,keyCode);
			switch(keyCode)
                   {
                case VOLUME_UP:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_VOLUME_UP)),100);
			   break;
		       case VOLUME_DOWN:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_VOLUME_DOWN)), 100);
               break;
		       case MUTE:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_MUTE)), 100);
			   break;
		       case UP:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_UP)), 100);
			   break;
		       case DOWN:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_DOWN)), 100);
			   break;
		       case LEFT:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_LEFT)), 100);
			   break;
		       case RIGHT:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_RIGHT)), 100);
			   break;
		       case SELECT:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_SELECT)), 100);
			   break;
		       case HOME:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_HOME)), 100);
			   break;
		       case BACK:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_BACK)), 100);
			   break;
		       case NUMBER_0:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_0)), 100);
			   break;
		       case NUMBER_1:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_1)), 100);
			   break;
		       case NUMBER_2:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_2)), 100);
			   break;
		       case NUMBER_3:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_3)), 100);
			   break;
		       case NUMBER_4:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_4)), 100);
			   break;
		       case NUMBER_5:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_5)), 100);
			   break;
		       case NUMBER_6:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_6)), 100);
			   break;
		       case NUMBER_7:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_7)), 100);
			   break;
		       case NUMBER_8:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_8)), 100);
			   break;
		       case NUMBER_9:
			   _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_NUM_9)), 100);
			   break;

                   }
		}
		void HdmiCecSource::sendKeyReleaseEvent(const int logicalAddress)
		 {
	            LOGINFO(" sendKeyReleaseEvent logicalAddress 0x%x \n",logicalAddress);
                    if(!(_instance->smConnection))
                        return;
		 _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlReleased()), 100);

		 }
       void HdmiCecSource::SendStandbyMsgEvent(const int logicalAddress)
       {
           JsonObject params;
           params["logicalAddress"] = JsonValue(logicalAddress);
           sendNotify(HDMICEC_EVENT_ON_STANDBY_MSG_RECEIVED, params);
       }
	   uint32_t HdmiCecSource::sendRemoteKeyPressWrapper(const JsonObject& parameters, JsonObject& response)
		{
            returnIfParamNotFound(parameters, "logicalAddress");
			returnIfParamNotFound(parameters, "keyCode");
			string logicalAddress = parameters["logicalAddress"].String();
			string keyCode = parameters["keyCode"].String();
			SendKeyInfo keyInfo;
			keyInfo.logicalAddr = stoi(logicalAddress);
			keyInfo.keyCode     = stoi(keyCode);
			std::unique_lock<std::mutex> lk(m_sendKeyEventMutex);
			m_SendKeyQueue.push(keyInfo);
            m_sendKeyEventThreadRun = true;
			m_sendKeyCV.notify_one();
			LOGINFO("Post send key press event to queue size:%d \n",(int)m_SendKeyQueue.size());
			returnResponse(true);
		}
	    
       uint32_t HdmiCecSource::sendStandbyMessageWrapper(const JsonObject& parameters, JsonObject& response)
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
 
       bool HdmiCecSource::sendStandbyMessage()
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


       const void HdmiCecSource::InitializeIARM()
       {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
       }

       void HdmiCecSource::DeinitializeIARM()
       {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG,dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED,pwrMgrModeChangeEventHandler) );
            }
       }

       void HdmiCecSource::cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSource::_instance)
                return;

            if( !strcmp(owner, IARM_BUS_CECMGR_NAME))
            {
                switch (eventId)
                {
                    case IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED:
                    {
                        HdmiCecSource::_instance->onCECDaemonInit();
                    }
                    break;
                    case IARM_BUS_CECMGR_EVENT_STATUS_UPDATED:
                    {
                        IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = new IARM_Bus_CECMgr_Status_Updated_Param_t;
                        if(evtData)
                        {
                            memcpy(evtData,data,sizeof(IARM_Bus_CECMgr_Status_Updated_Param_t));
                            HdmiCecSource::_instance->cecStatusUpdated(evtData);
                        }
                    }
                    break;
                    default:
                    /*Do nothing*/
                    break;
                }
            }
       }

       void HdmiCecSource::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSource::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);
                HdmiCecSource::_instance->onHdmiHotPlug(hdmi_hotplug_event);
                //Trigger CEC device poll here
                pthread_cond_signal(&(_instance->m_condSig));
            }
       }

       void HdmiCecSource::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSource::_instance)
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

       void HdmiCecSource::onCECDaemonInit()
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

       void HdmiCecSource::cecStatusUpdated(void *evtStatus)
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

                    int logicalAddr = evtData->logicalAddress;
                    std::string logicalAddrDeviceType = DeviceType(LogicalAddress(evtData->logicalAddress).getType()).toString().c_str();

                    LOGINFO("cecLogicalAddressUpdated: logical address updated: %d , saved : %d ", logicalAddr, logicalAddress.toInt());
                    if (logicalAddr != logicalAddress.toInt() || logicalAddrDeviceType != logicalAddressDeviceType)
                    {
                        logicalAddress = logicalAddr;
                        logicalAddressDeviceType = logicalAddrDeviceType;
                    }
                }
                catch (const std::exception& e)
                {
                    LOGWARN("CEC exception caught from cecStatusUpdated");
                }

                delete evtData;
            }
           return;
       }

       void HdmiCecSource::sendActiveSourceEvent()
       {
           JsonObject params;
           params["status"] = isDeviceActiveSource;
           LOGWARN(" sendActiveSourceEvent isDeviceActiveSource: %d ",isDeviceActiveSource);
           sendNotify(eventString[HDMICECSOURCE_EVENT_ACTIVE_SOURCE_STATUS_UPDATED], params);
       }

       void HdmiCecSource::onHdmiHotPlug(int connectStatus)
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
                     std::vector<uint8_t> edidVec;
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

       uint32_t HdmiCecSource::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

       uint32_t HdmiCecSource::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getEnabled();
            returnResponse(true);
       }
       uint32_t HdmiCecSource::setOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

       uint32_t HdmiCecSource::getOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getOTPEnabled();
            returnResponse(true);
       }

       uint32_t HdmiCecSource::setOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
       {
           LOGINFOMETHOD();

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

        uint32_t HdmiCecSource::getOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
        {
            response["name"] = osdName.toString(); 
            LOGINFO("getOSDNameWrapper osdName : %s \n",osdName.toString().c_str());
            returnResponse(true);
        }

        uint32_t HdmiCecSource::setVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

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

        uint32_t HdmiCecSource::getVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO("getVendorIdWrapper  appVendorId : %s  \n",appVendorId.toString().c_str());
            response["vendorid"] = appVendorId.toString() ;
            returnResponse(true);
        }


        uint32_t HdmiCecSource::performOTPActionWrapper(const JsonObject& parameters, JsonObject& response)
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

        bool HdmiCecSource::loadSettings()
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

        void HdmiCecSource::setEnabled(bool enabled, bool isPersist)
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

        void HdmiCecSource::setOTPEnabled(bool enabled)
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

        void HdmiCecSource::CECEnable(void)
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
                catch (const std::exception& e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().init()");
                }
            }
            libcecInitStatus++;

            m_sendKeyEventThreadExit = false;
            try {
               if (m_sendKeyEventThread.get().joinable()) {
                   m_sendKeyEventThread.get().join();
	       }
               m_sendKeyEventThread = Utils::ThreadRAII(std::thread(threadSendKeyEvent));
            } catch(const std::system_error& e) {
                LOGERR("exception in creating threadSendKeyEvent %s", e.what());
	    }


            //Acquire CEC Addresses
            getPhysicalAddress();
            getLogicalAddress();

            smConnection = new Connection(logicalAddress.toInt(),false,"ServiceManager::Connection::");
            smConnection->open();
            msgProcessor = new HdmiCecSourceProcessor(*smConnection);
            msgFrameListener = new HdmiCecSourceFrameListener(*msgProcessor);
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
                try {
                    if (m_UpdateThread.get().joinable()) {
                       m_UpdateThread.get().join();
	            }
                    m_UpdateThread = Utils::ThreadRAII(std::thread(threadUpdateCheck));
                } catch(const std::system_error& e) {
                    LOGERR("exception in creating threadUpdateCheck %s", e.what());
	        }

                LOGWARN("Start Thread %p", smConnection );
                m_pollThreadExit = false;
                _instance->m_numberOfDevices = 0;
                _instance->m_lock = PTHREAD_MUTEX_INITIALIZER;
                _instance->m_condSig = PTHREAD_COND_INITIALIZER;
                try {
                    if (m_pollThread.get().joinable()) {
                       m_pollThread.get().join();
	            }
                    m_pollThread = Utils::ThreadRAII(std::thread(threadRun));
                } catch(const std::system_error& e) {
                    LOGERR("exception in creating threadRun %s", e.what());
	        }

            }
            return;
        }

        void HdmiCecSource::CECDisable(void)
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

            {
                m_sendKeyEventThreadExit = true;
                std::unique_lock<std::mutex> lk(m_sendKeyEventMutex);
                m_sendKeyEventThreadRun = true;
                m_sendKeyCV.notify_one();
            }
            try
	        {
                if (m_sendKeyEventThread.get().joinable())
                    m_sendKeyEventThread.get().join();
	        }
	        catch(const std::system_error& e)
	        {
		        LOGERR("system_error exception in thread join %s", e.what());
	        }
	        catch(const std::exception& e)
	        {
		        LOGERR("exception in thread join %s", e.what());
	        }

            if (smConnection != NULL)
            {
                LOGWARN("Stop Thread %p", smConnection );

                m_updateThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_mutex_lock(&(_instance->m_lockUpdate)); //Join mutex lock to wait until thread is in its wait condition
                pthread_cond_signal(&(_instance->m_condSigUpdate));
                pthread_mutex_unlock(&(_instance->m_lockUpdate));
                if (m_UpdateThread.get().joinable()) {//Join thread to make sure it's deleted before moving on.
                    m_UpdateThread.get().join();
                }
                LOGWARN("Deleted update Thread %p", smConnection );

                m_pollThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_mutex_lock(&(_instance->m_lock)); //Join mutex lock to wait until thread is in its wait condition
                pthread_cond_signal(&(_instance->m_condSig));
                pthread_mutex_unlock(&(_instance->m_lock));
                if (m_pollThread.get().joinable()) {//Join thread to make sure it's deleted before moving on.
                    m_pollThread.get().join();
                }
                LOGWARN("Deleted Thread %p", smConnection );
                //Clear cec device cache.
                removeAllCecDevices();

                smConnection->close();
                delete smConnection;
                delete msgProcessor;
                delete msgFrameListener;
                msgProcessor = NULL;
                msgFrameListener = NULL;
                smConnection = NULL;
            }
            cecEnableStatus = false;

            if(1 == libcecInitStatus)
            {
                try
                {
                   LibCCEC::getInstance().term();
                }
                catch (const std::exception& e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().term() ");
                }
            }

            libcecInitStatus--;

            return;
        }


        void HdmiCecSource::getPhysicalAddress()
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
            catch (const std::exception& e)
            {
                LOGWARN("exception caught from getPhysicalAddress");
            }
            return;
        }

        void HdmiCecSource::getLogicalAddress()
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
            catch (const std::exception& e)
            {
                LOGWARN("CEC exception caught from getLogicalAddress ");
            }
            return;
        }

        bool HdmiCecSource::getEnabled()
        {
            LOGINFO("getEnabled :%d ",cecEnableStatus);
            return cecEnableStatus;
        }

        bool HdmiCecSource::getOTPEnabled()
        {
            if(true == cecOTPSettingEnabled)
                return true;
            else
                return false;
            LOGINFO("getOTPEnabled :%d ",cecOTPSettingEnabled);
        }

        bool HdmiCecSource::performOTPAction()
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

        uint32_t HdmiCecSource::getDeviceList (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
		LOGINFOMETHOD();
		//Trigger CEC device poll here
		pthread_cond_signal(&(_instance->m_condSig));

		bool success = true;
		response["numberofdevices"] = HdmiCecSource::_instance->m_numberOfDevices;
		LOGINFO("getDeviceListWrapper  m_numberOfDevices :%d \n", HdmiCecSource::_instance->m_numberOfDevices);
		JsonArray deviceListArg;
		try
		{
			int i = 0;
			for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				if (BIT_CHECK(deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
					JsonObject device;
					device["logicalAddress"] = HdmiCecSource::_instance->deviceList[i].m_logicalAddress.toInt();
					device["osdName"] = HdmiCecSource::_instance->deviceList[i].m_osdName.toString().c_str();
					device["vendorID"] = HdmiCecSource::_instance->deviceList[i].m_vendorID.toString().c_str();
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

	bool HdmiCecSource::pingDeviceUpdateList (int idev)
	{
		bool isConnected = false;
		//self ping is not required
		if (idev == logicalAddress.toInt()){
		        return isConnected;
		}
		if(!HdmiCecSource::_instance)
		{
			LOGERR("HdmiCecSource::_instance not existing");
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

	void  HdmiCecSource::sendDeviceUpdateInfo(const int logicalAddress)
	{
		JsonObject params;
		params["logicalAddress"] = JsonValue(logicalAddress);
		LOGINFO("Device info updated notification send: for logical address:%d\r\n", logicalAddress);
		sendNotify(eventString[HDMICECSOURCE_EVENT_DEVICE_INFO_UPDATED], params);
	}

	void HdmiCecSource::addDevice(const int logicalAddress) {

		if(!HdmiCecSource::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if ( !(BIT_CHECK(HdmiCecSource::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		 {
			BIT_SET(HdmiCecSource::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT);
			HdmiCecSource::_instance->deviceList[logicalAddress].m_logicalAddress = LogicalAddress(logicalAddress);
			HdmiCecSource::_instance->m_numberOfDevices++;
			LOGINFO("New cec ligical address add notification send:  \r\n");
			sendNotify(eventString[HDMICECSOURCE_EVENT_DEVICE_ADDED], JsonObject());
		 }
		//Two source devices can have same logical address.
		requestCecDevDetails(logicalAddress);
	}

	void HdmiCecSource::removeAllCecDevices() {
		int i = 0;
		for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
			removeDevice (i);
		}
	}
	void HdmiCecSource::removeDevice(const int logicalAddress) {
		if(!HdmiCecSource::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED ){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if (BIT_CHECK(HdmiCecSource::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
		{
			_instance->m_numberOfDevices--;
			_instance->deviceList[logicalAddress].clear();
			LOGINFO("Cec ligical address remove notification send:  \r\n");
			sendNotify(eventString[HDMICECSOURCE_EVENT_DEVICE_REMOVED], JsonObject());
		}
	}

	void HdmiCecSource::sendUnencryptMsg(unsigned char* msg, int size)
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

	void HdmiCecSource::requestVendorID(const int newDevlogicalAddress)
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

	void HdmiCecSource::requestOsdName(const int newDevlogicalAddress)
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

	void HdmiCecSource::requestCecDevDetails(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		requestVendorID (newDevlogicalAddress);
		requestOsdName (newDevlogicalAddress);
	}

	void HdmiCecSource::threadRun()
	{
		if(!HdmiCecSource::_instance)
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
	        LOGINFO("%s: Thread exited", __FUNCTION__);
	}
	void HdmiCecSource::threadSendKeyEvent()
        {
            if(!HdmiCecSource::_instance)
                return;

	    SendKeyInfo keyInfo = {-1,-1};

            while(!_instance->m_sendKeyEventThreadExit)
            {
                keyInfo.logicalAddr = -1;
                keyInfo.keyCode = -1;
                {
                    // Wait for a message to be added to the queue
                    std::unique_lock<std::mutex> lk(_instance->m_sendKeyEventMutex);
                    _instance->m_sendKeyCV.wait(lk, []{return (_instance->m_sendKeyEventThreadRun == true);});
                }

                if (_instance->m_sendKeyEventThreadExit == true)
                {
                    LOGINFO(" threadSendKeyEvent Exiting");
                    _instance->m_sendKeyEventThreadRun = false;
                    break;
                }

                if (_instance->m_SendKeyQueue.empty()) {
                    _instance->m_sendKeyEventThreadRun = false;
                    continue;
                }

                    keyInfo = _instance->m_SendKeyQueue.front();
                    _instance->m_SendKeyQueue.pop();

                LOGINFO("sendRemoteKeyThread : logical addr:0x%x keyCode: 0x%x  queue size :%d \n",keyInfo.logicalAddr,keyInfo.keyCode,(int)_instance->m_SendKeyQueue.size());
			    _instance->sendKeyPressEvent(keyInfo.logicalAddr,keyInfo.keyCode);
			    _instance->sendKeyReleaseEvent(keyInfo.logicalAddr);
            }
	    LOGINFO("%s: Thread exited", __FUNCTION__);
        }
	void HdmiCecSource::threadUpdateCheck()
	{
		if(!HdmiCecSource::_instance)
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
				if (BIT_CHECK(HdmiCecSource::_instance->deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
				{
					int itr = 0;
					bool retry = true;
					int iCounter = 0;
					for (itr = 0; ((itr<5)&&(retry)); itr++){

						if (!HdmiCecSource::_instance->deviceList[i].m_isOSDNameUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 2sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCecSource::_instance->requestOsdName (i);
							retry = true;
						}
						else {
							retry = false;
						}

						if (!HdmiCecSource::_instance->deviceList[i].m_isVendorIDUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 1sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCecSource::_instance->requestVendorID (i);
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
	        LOGINFO("%s: Thread exited", __FUNCTION__);
	}

    } // namespace Plugin
} // namespace WPEFramework
