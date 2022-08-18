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

#include "HdmiCecSink.h"

#include "ccec/Connection.hpp"
#include "ccec/CECFrame.hpp"
#include "ccec/MessageEncoder.hpp"
#include "host.hpp"
#include "ccec/host/RDK.hpp"

#include "ccec/drivers/iarmbus/CecIARMBusMgr.h"

#include "pwrMgr.h"
#include "dsMgr.h"
#include "dsRpc.h"
#include "dsDisplay.h"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#include "websocket/URL.h"

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilssyncPersistFile.h"

#define HDMICECSINK_METHOD_SET_ENABLED 			"setEnabled"
#define HDMICECSINK_METHOD_GET_ENABLED 			"getEnabled"
#define HDMICECSINK_METHOD_OTP_SET_ENABLED 		"setOTPEnabled"
#define HDMICECSINK_METHOD_OTP_GET_ENABLED 		"getOTPEnabled"
#define HDMICECSINK_METHOD_SET_OSD_NAME 		"setOSDName"
#define HDMICECSINK_METHOD_GET_OSD_NAME 		"getOSDName"
#define HDMICECSINK_METHOD_SET_VENDOR_ID 		"setVendorId"
#define HDMICECSINK_METHOD_GET_VENDOR_ID 		"getVendorId"
#define HDMICECSINK_METHOD_PRINT_DEVICE_LIST 	"printDeviceList"
#define HDMICECSINK_METHOD_SET_ACTIVE_PATH 		"setActivePath"
#define HDMICECSINK_METHOD_SET_ROUTING_CHANGE 	"setRoutingChange"
#define HDMICECSINK_METHOD_GET_DEVICE_LIST 		"getDeviceList"
#define HDMICECSINK_METHOD_GET_ACTIVE_SOURCE 	"getActiveSource"
#define HDMICECSINK_METHOD_SET_ACTIVE_SOURCE 	"setActiveSource"
#define HDMICECSINK_METHOD_GET_ACTIVE_ROUTE   	"getActiveRoute"
#define HDMICECSINK_METHOD_SET_MENU_LANGUAGE  	"setMenuLanguage"
#define HDMICECSINK_METHOD_REQUEST_ACTIVE_SOURCE "requestActiveSource"
#define HDMICECSINK_METHOD_SETUP_ARC              "setupARCRouting"
#define HDMICECSINK_METHOD_REQUEST_SHORT_AUDIO_DESCRIPTOR  "requestShortAudioDescriptor"
#define HDMICECSINK_METHOD_SEND_STANDBY_MESSAGE            "sendStandbyMessage"
#define HDMICECSINK_METHOD_SEND_AUDIO_DEVICE_POWER_ON "sendAudioDevicePowerOnMessage"
#define HDMICECSINK_METHOD_SEND_KEY_PRESS                          "sendKeyPressEvent"
#define HDMICECSINK_METHOD_SEND_GIVE_AUDIO_STATUS          "sendGetAudioStatusMessage"
#define HDMICECSINK_METHOD_GET_AUDIO_DEVICE_CONNECTED_STATUS   "getAudioDeviceConnectedStatus"
#define HDMICECSINK_METHOD_REQUEST_AUDIO_DEVICE_POWER_STATUS   "requestAudioDevicePowerStatus"

#define TEST_ADD 0
#define HDMICECSINK_REQUEST_MAX_RETRY 				3
#define HDMICECSINK_REQUEST_MAX_WAIT_TIME_MS 		2000
#define HDMICECSINK_PING_INTERVAL_MS 				10000
#define HDMICECSINK_WAIT_FOR_HDMI_IN_MS 			1000
#define HDMICECSINK_REQUEST_INTERVAL_TIME_MS 		200
#define HDMICECSINK_NUMBER_TV_ADDR 					2
#define HDMICECSINK_UPDATE_POWER_STATUS_INTERVA_MS    (60 * 1000)
#define HDMISINK_ARC_START_STOP_MAX_WAIT_MS           4000


#define SAD_FMT_CODE_AC3 2
#define SAD_FMT_CODE_ENHANCED_AC3 10

enum {
	DEVICE_POWER_STATE_ON = 0,
	DEVICE_POWER_STATE_OFF = 1
};


enum {
	HDMICECSINK_EVENT_ACTIVE_SOURCE_CHANGE = 1,
	HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY,
	HDMICECSINK_EVENT_TEXT_VIEW_ON_MSG,
	HDMICECSINK_EVENT_IMAGE_VIEW_ON_MSG,
	HDMICECSINK_EVENT_DEVICE_ADDED,
	HDMICECSINK_EVENT_DEVICE_REMOVED,
	HDMICECSINK_EVENT_DEVICE_INFO_UPDATED,
	HDMICECSINK_EVENT_INACTIVE_SOURCE,
        HDMICECSINK_EVENT_ARC_INITIATION_EVENT,
	HDMICECSINK_EVENT_ARC_TERMINATION_EVENT,
        HDMICECSINK_EVENT_SHORT_AUDIODESCRIPTOR_EVENT,
        HDMICECSINK_EVENT_STANDBY_MSG_EVENT,
	HDMICECSINK_EVENT_SYSTEM_AUDIO_MODE,
	HDMICECSINK_EVENT_REPORT_AUDIO_STATUS,
	HDMICECSINK_EVENT_AUDIO_DEVICE_CONNECTED_STATUS,
	HDMICECSINK_EVENT_CEC_ENABLED,
        HDMICECSINK_EVENT_AUDIO_DEVICE_POWER_STATUS,
};

static char *eventString[] = {
	"None",
	"onActiveSourceChange",
	"onWakeupFromStandby",
	"onTextViewOnMsg",
	"onImageViewOnMsg",
	"onDeviceAdded",
	"onDeviceRemoved",
	"onDeviceInfoUpdated",
	"onInActiveSource",
        "arcInitiationEvent",
        "arcTerminationEvent",
        "shortAudiodesciptorEvent",
        "standbyMessageReceived",
        "setSystemAudioModeEvent",
        "reportAudioStatusEvent",
	"reportAudioDeviceConnectedStatus",
	"reportCecEnabledEvent",
        "reportAudioDevicePowerStatus"
};
	

#define CEC_SETTING_ENABLED_FILE "/opt/persistent/ds/cecData_2.json"
#define CEC_SETTING_OTP_ENABLED "cecOTPEnabled"
#define CEC_SETTING_ENABLED "cecEnabled"
#define CEC_SETTING_OSD_NAME "cecOSDName"
#define CEC_SETTING_VENDOR_ID "cecVendorId"

static vector<uint8_t> defaultVendorId = {0x00,0x19,0xFB};
static VendorID appVendorId = {defaultVendorId.at(0),defaultVendorId.at(1),defaultVendorId.at(2)};
static VendorID lgVendorId = {0x00,0xE0,0x91};
static PhysicalAddress physical_addr = {0x0F,0x0F,0x0F,0x0F};
static LogicalAddress logicalAddress = 0xF;
static Language defaultLanguage = "eng";
static OSDName osdName = "TV Box";
static int32_t powerState = DEVICE_POWER_STATE_OFF;
static vector<uint8_t> formatid = {0,0};
static vector<uint8_t> audioFormatCode = { SAD_FMT_CODE_ENHANCED_AC3,SAD_FMT_CODE_AC3 };
static uint8_t numberofdescriptor = 2;
static int32_t HdmiArcPortID = -1;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 1

namespace WPEFramework
{
	namespace {

        static Plugin::Metadata<Plugin::HdmiCecSink> metadata(
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
        SERVICE_REGISTRATION(HdmiCecSink, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        HdmiCecSink* HdmiCecSink::_instance = nullptr;
        static int libcecInitStatus = 0;

//=========================================== HdmiCecSinkFrameListener =========================================
        void HdmiCecSinkFrameListener::notify(const CECFrame &in) const {
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

//=========================================== HdmiCecSinkProcessor =========================================
       void HdmiCecSinkProcessor::process (const ActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ActiveSource %s : %s  : %s \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());

			 HdmiCecSink::_instance->addDevice(header.from.toInt());	
			 HdmiCecSink::_instance->updateActiveSource(header.from.toInt(), msg);
      }
       void HdmiCecSinkProcessor::process (const InActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: InActiveSource %s : %s : %s  \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());

			 HdmiCecSink::_instance->updateInActiveSource(header.from.toInt(), msg);
	   }
	   
       void HdmiCecSinkProcessor::process (const ImageViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ImageViewOn from %s\n", header.from.toString().c_str());
			 HdmiCecSink::_instance->addDevice(header.from.toInt());
			 HdmiCecSink::_instance->updateImageViewOn(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const TextViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: TextViewOn\n");
			 HdmiCecSink::_instance->addDevice(header.from.toInt());
			 HdmiCecSink::_instance->updateImageViewOn(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const RequestActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RequestActiveSource\n");

			 HdmiCecSink::_instance->setActiveSource(true);
       }
       void HdmiCecSinkProcessor::process (const Standby &msg, const Header &header)
       {
             printHeader(header);
			 LOGINFO("Command: Standby from %s\n", header.from.toString().c_str());
             HdmiCecSink::_instance->SendStandbyMsgEvent(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const GetCECVersion &msg, const Header &header)
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
       void HdmiCecSinkProcessor::process (const CECVersion &msg, const Header &header)
       {
	     bool updateStatus;
	     printHeader(header);
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());

	     HdmiCecSink::_instance->addDevice(header.from.toInt());
	     updateStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_isVersionUpdated;
             LOGINFO("updateStatus %d\n",updateStatus);
	     HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.version);
	     if(!updateStatus)
	     HdmiCecSink::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const SetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetMenuLanguage Language : %s \n",msg.language.toString().c_str());
       }
       void HdmiCecSinkProcessor::process (const GiveOSDName &msg, const Header &header)
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
       void HdmiCecSinkProcessor::process (const GivePhysicalAddress &msg, const Header &header)
       {
             LOGINFO("Command: GivePhysicalAddress\n");
             if (!(header.to == LogicalAddress(LogicalAddress::BROADCAST)))
             {
                 try
                 { 
                     LOGINFO(" sending ReportPhysicalAddress response physical_addr :%s logicalAddress :%x \n",physical_addr.toString().c_str(), logicalAddress.toInt());
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ReportPhysicalAddress(physical_addr,logicalAddress.toInt())),500);
                 } 
                 catch(...)
                 {
                    LOGWARN("Exception while sending ReportPhysicalAddress ");
                 }
             }
       }
       void HdmiCecSinkProcessor::process (const GiveDeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             try
             {
                 LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n",appVendorId.toString().c_str());
                 conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));
             }
             catch(...)
             {
                 LOGWARN("Exception while sending DeviceVendorID");
             }

       }
       void HdmiCecSinkProcessor::process (const SetOSDString &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetOSDString OSDString : %s\n",msg.osdString.toString().c_str());
       }
       void HdmiCecSinkProcessor::process (const SetOSDName &msg, const Header &header)
       {
             printHeader(header);
	     bool updateStatus ;
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());

	     HdmiCecSink::_instance->addDevice(header.from.toInt());
	     updateStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_isOSDNameUpdated;
	     LOGINFO("updateStatus %d\n",updateStatus);
	     HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.osdName);
             if(!updateStatus)
	     HdmiCecSink::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const RoutingChange &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingChange From : %s To: %s \n",msg.from.toString().c_str(),msg.to.toString().c_str());
       }
       void HdmiCecSinkProcessor::process (const RoutingInformation &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RoutingInformation Routing Information to Sink : %s\n",msg.toSink.toString().c_str());
       }
       void HdmiCecSinkProcessor::process (const SetStreamPath &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetStreamPath Set Stream Path to Sink : %s\n",msg.toSink.toString().c_str());
       }
       void HdmiCecSinkProcessor::process (const GetMenuLanguage &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: GetMenuLanguage\n");
			 HdmiCecSink::_instance->sendMenuLanguage();  
       }
       void HdmiCecSinkProcessor::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             printHeader(header);
	     bool updateDeviceTypeStatus;
	     bool updatePAStatus;
             LOGINFO("Command: ReportPhysicalAddress\n");

	     if(!HdmiCecSink::_instance)
	        return;
             HdmiCecSink::_instance->addDevice(header.from.toInt());
	     updateDeviceTypeStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_isDeviceTypeUpdated;
             updatePAStatus   = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_isPAUpdated;
	     LOGINFO("updateDeviceTypeStatus %d updatePAStatus %d \n",updateDeviceTypeStatus,updatePAStatus);
	     HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.physicalAddress);
	     HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.deviceType);
	     HdmiCecSink::_instance->updateDeviceChain(header.from, msg.physicalAddress);
	     if (!updateDeviceTypeStatus || !updatePAStatus)
             HdmiCecSink::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const DeviceVendorID &msg, const Header &header)
       {
	     bool updateStatus ;
	     printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());

	     HdmiCecSink::_instance->addDevice(header.from.toInt());
	     updateStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_isVendorIDUpdated;
             LOGINFO("updateStatus %d\n",updateStatus);
	     HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
	     if (!updateStatus)
             HdmiCecSink::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCecSinkProcessor::process (const GiveDevicePowerStatus &msg, const Header &header)
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
       void HdmiCecSinkProcessor::process (const ReportPowerStatus &msg, const Header &header)
       {
	   uint32_t  oldPowerStatus,newPowerStatus;
	   printHeader(header);
	   LOGINFO("Command: ReportPowerStatus Power Status from:%s status : %s \n",header.from.toString().c_str(),msg.status.toString().c_str());
           oldPowerStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_powerStatus.toInt();
	   HdmiCecSink::_instance->addDevice(header.from.toInt());
	   HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.status);
	   newPowerStatus = HdmiCecSink::_instance->deviceList[header.from.toInt()].m_powerStatus.toInt();
	   LOGINFO(" oldPowerStatus %d newpower status %d \n",oldPowerStatus,newPowerStatus);
           if ((oldPowerStatus != newPowerStatus) )
	   {
	       HdmiCecSink::_instance->sendDeviceUpdateInfo(header.from.toInt());
	   }

           if((header.from.toInt() == LogicalAddress::AUDIO_SYSTEM) && (HdmiCecSink::_instance->m_audioDevicePowerStatusRequested)) {
               HdmiCecSink::_instance->reportAudioDevicePowerStatusInfo(header.from.toInt(), newPowerStatus);
           }

       }
       void HdmiCecSinkProcessor::process (const FeatureAbort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: FeatureAbort opcode=%s, Reason = %s\n", msg.feature.toString().c_str(), msg.reason.toString().c_str());

			 if(header.from.toInt() < LogicalAddress::UNREGISTERED &&    
			 		msg.reason.toInt()   == AbortReason::UNRECOGNIZED_OPCODE)
			 {
			 	switch(msg.feature.opCode())
			 	{
			 		case GET_CEC_VERSION :
			 		{
			 			/* If we get a Feature abort for CEC Version then default to 1.4b */
			 			HdmiCecSink::_instance->deviceList[header.from.toInt()].update(Version(Version::V_1_4));
			 		}
						break;
					case GIVE_DEVICE_VENDOR_ID :
			 		{
			 			/* If we get a Feature abort for CEC Version then default to 1.4b */
			 			HdmiCecSink::_instance->deviceList[header.from.toInt()].update(VendorID((uint8_t *)"FA", 2));
			 		}
						break;
					
					case GIVE_OSD_NAME :
			 		{
			 			HdmiCecSink::_instance->deviceList[header.from.toInt()].update(OSDName("FA"));
			 		}
						break;

					case GIVE_DEVICE_POWER_STATUS :
					{
						HdmiCecSink::_instance->deviceList[header.from.toInt()].update(PowerStatus(PowerStatus::POWER_STATUS_FEATURE_ABORT));
					}
						break;
			 	}

				HdmiCecSink::_instance->deviceList[header.from.toInt()].m_featureAborts.push_back(msg);
			 }

                         if(msg.feature.opCode() == REQUEST_SHORT_AUDIO_DESCRIPTOR)
		         {
                            JsonArray audiodescriptor;
                            audiodescriptor.Add(0);
			    HdmiCecSink::_instance->Send_ShortAudioDescriptor_Event(audiodescriptor);
                        }
			
       }
       void HdmiCecSinkProcessor::process (const Abort &msg, const Header &header)
       {
              printHeader(header);
             LOGINFO("Command: Abort\n");
	      if (!(header.to == LogicalAddress(LogicalAddress::BROADCAST)))
             {
                AbortReason reason = AbortReason::UNRECOGNIZED_OPCODE;
                LogicalAddress logicaladdress =header.from.toInt();
                OpCode feature = msg.opCode();
                HdmiCecSink::_instance->sendFeatureAbort(logicaladdress, feature,reason);
	     }
	     else
	     {
		LOGINFO("Command: Abort broadcast msg so ignore\n");
	     }
       }
       void HdmiCecSinkProcessor::process (const Polling &msg, const Header &header)                                 {
             printHeader(header);
             LOGINFO("Command: Polling\n");
       }

       void HdmiCecSinkProcessor::process (const InitiateArc &msg, const Header &header)
       {
            printHeader(header);
            PhysicalAddress physical_addr_invalid = {0x0F,0x0F,0x0F,0x0F};
            PhysicalAddress physical_addr_arc_port = {0x0F,0x0F,0x0F,0x0F};

            LOGINFO("Command: INITIATE_ARC \n");
            if(!HdmiCecSink::_instance || HdmiArcPortID == -1)
	    return;

            if (HdmiArcPortID == 0 )
               physical_addr_arc_port = {0x01,0x00,0x00,0x00};
            if (HdmiArcPortID == 1 )
               physical_addr_arc_port = {0x02,0x00,0x00,0x00};
            if (HdmiArcPortID == 2 )
               physical_addr_arc_port = {0x03,0x00,0x00,0x00};

            if( (HdmiCecSink::_instance->deviceList[0x5].m_physicalAddr.toString() == physical_addr_arc_port.toString()) || (HdmiCecSink::_instance->deviceList[0x5].m_physicalAddr.toString() == physical_addr_invalid.toString()) ) {
                LOGINFO("Command: INITIATE_ARC InitiateArc success %s \n",HdmiCecSink::_instance->deviceList[0x5].m_physicalAddr.toString().c_str());
                HdmiCecSink::_instance->Process_InitiateArc();
            } else {
                LOGINFO("Command: INITIATE_ARC InitiateArc ignore %s \n",HdmiCecSink::_instance->deviceList[0x5].m_physicalAddr.toString().c_str());
            }
       }
       void HdmiCecSinkProcessor::process (const TerminateArc &msg, const Header &header)
       {
           printHeader(header);
           if(!HdmiCecSink::_instance)
	     return;
           HdmiCecSink::_instance->Process_TerminateArc();
       }
       void HdmiCecSinkProcessor::process (const ReportShortAudioDescriptor  &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ReportShortAudioDescriptor %s : %d \n",GetOpName(msg.opCode()),numberofdescriptor);
            HdmiCecSink::_instance->Process_ShortAudioDescriptor_msg(msg);
       }

       void HdmiCecSinkProcessor::process (const SetSystemAudioMode &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: SetSystemAudioMode  %s audio status %d audio status is  %s \n",GetOpName(msg.opCode()),msg.status.toInt(),msg.status.toString().c_str());
          HdmiCecSink::_instance->Process_SetSystemAudioMode_msg(msg);
       }
      void HdmiCecSinkProcessor::process (const ReportAudioStatus &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ReportAudioStatus  %s audio Mute status %d  means %s  and current Volume level is %d \n",GetOpName(msg.opCode()),msg.status.getAudioMuteStatus(),msg.status.toString().c_str(),msg.status.getAudioVolume());
             HdmiCecSink::_instance->Process_ReportAudioStatus_msg(msg);
       }
//=========================================== HdmiCecSink =========================================

       HdmiCecSink::HdmiCecSink()
       : PluginHost::JSONRPC()
       {
       	   int err;
           LOGWARN("Initlaizing HdmiCecSink");
           HdmiCecSink::_instance = this;
           smConnection=NULL;
		   cecEnableStatus = false;
                   HdmiCecSink::_instance->m_numberOfDevices = 0;
		   m_logicalAddressAllocated = LogicalAddress::UNREGISTERED;
		   m_currentActiveSource = -1;
		   m_isHdmiInConnected = false;
		   hdmiCecAudioDeviceConnected = false;
                   m_audioDevicePowerStatusRequested = false;
		   m_pollNextState = POLL_THREAD_STATE_NONE;
		   m_pollThreadState = POLL_THREAD_STATE_NONE;
		   dsHdmiInGetNumberOfInputsParam_t hdmiInput;

           InitializeIARM();

           Register(HDMICECSINK_METHOD_SET_ENABLED, &HdmiCecSink::setEnabledWrapper, this);
           Register(HDMICECSINK_METHOD_GET_ENABLED, &HdmiCecSink::getEnabledWrapper, this);
           Register(HDMICECSINK_METHOD_SET_OSD_NAME, &HdmiCecSink::setOSDNameWrapper, this);
           Register(HDMICECSINK_METHOD_GET_OSD_NAME, &HdmiCecSink::getOSDNameWrapper, this);
           Register(HDMICECSINK_METHOD_SET_VENDOR_ID, &HdmiCecSink::setVendorIdWrapper, this);
           Register(HDMICECSINK_METHOD_GET_VENDOR_ID, &HdmiCecSink::getVendorIdWrapper, this);
		   Register(HDMICECSINK_METHOD_PRINT_DEVICE_LIST, &HdmiCecSink::printDeviceListWrapper, this);
		   Register(HDMICECSINK_METHOD_SET_ACTIVE_PATH, &HdmiCecSink::setActivePathWrapper, this);
		   Register(HDMICECSINK_METHOD_SET_ROUTING_CHANGE, &HdmiCecSink::setRoutingChangeWrapper, this); 
		   Register(HDMICECSINK_METHOD_GET_DEVICE_LIST, &HdmiCecSink::getDeviceListWrapper, this);
		   Register(HDMICECSINK_METHOD_GET_ACTIVE_SOURCE, &HdmiCecSink::getActiveSourceWrapper, this);
		   Register(HDMICECSINK_METHOD_SET_ACTIVE_SOURCE, &HdmiCecSink::setActiveSourceWrapper, this);
	       Register(HDMICECSINK_METHOD_GET_ACTIVE_ROUTE, &HdmiCecSink::getActiveRouteWrapper, this);
		  Register(HDMICECSINK_METHOD_REQUEST_ACTIVE_SOURCE, &HdmiCecSink::requestActiveSourceWrapper, this);
                   Register(HDMICECSINK_METHOD_SETUP_ARC, &HdmiCecSink::setArcEnableDisableWrapper, this);
		   Register(HDMICECSINK_METHOD_SET_MENU_LANGUAGE, &HdmiCecSink::setMenuLanguageWrapper, this);
                   Register(HDMICECSINK_METHOD_REQUEST_SHORT_AUDIO_DESCRIPTOR, &HdmiCecSink::requestShortAudioDescriptorWrapper, this);
                   Register(HDMICECSINK_METHOD_SEND_STANDBY_MESSAGE, &HdmiCecSink::sendStandbyMessageWrapper, this);
		   Register(HDMICECSINK_METHOD_SEND_AUDIO_DEVICE_POWER_ON, &HdmiCecSink::sendAudioDevicePowerOnMsgWrapper, this);
		   Register(HDMICECSINK_METHOD_SEND_KEY_PRESS,&HdmiCecSink::sendRemoteKeyPressWrapper,this);
		   Register(HDMICECSINK_METHOD_SEND_GIVE_AUDIO_STATUS,&HdmiCecSink::sendGiveAudioStatusWrapper,this);
		   Register(HDMICECSINK_METHOD_GET_AUDIO_DEVICE_CONNECTED_STATUS,&HdmiCecSink::getAudioDeviceConnectedStatusWrapper,this);
                   Register(HDMICECSINK_METHOD_REQUEST_AUDIO_DEVICE_POWER_STATUS,&HdmiCecSink::requestAudioDevicePowerStatusWrapper,this);
           logicalAddressDeviceType = "None";
           logicalAddress = 0xFF;
           m_sendKeyEventThreadExit = false;
           m_sendKeyEventThread = std::thread(threadSendKeyEvent);
           
           m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
           m_semSignaltoArcRoutingThread.acquire();
           m_arcRoutingThread = std::thread(threadArcRouting);


           m_arcStartStopTimer.connect( std::bind( &HdmiCecSink::arcStartStopTimerFunction, this ) );
           m_arcStartStopTimer.setSingleShot(true);
           // load persistence setting
           loadSettings();

            // get power state:
            IARM_Bus_PWRMgr_GetPowerState_Param_t param;
            err = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                            IARM_BUS_PWRMGR_API_GetPowerState,
                            (void *)&param,
                            sizeof(param));
            if(err == IARM_RESULT_SUCCESS)
            {
                powerState = (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)? DEVICE_POWER_STATE_ON :  DEVICE_POWER_STATE_OFF;
                LOGINFO("Current state is IARM: (%d) powerState :%d \n",param.curState,powerState);
            }

			err = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            IARM_BUS_DSMGR_API_dsHdmiInGetNumberOfInputs,
                            (void *)&hdmiInput,
                            sizeof(hdmiInput));
			
            if(err == IARM_RESULT_SUCCESS && hdmiInput.result == dsERR_NONE )          {
				LOGINFO("Number of Inputs [%d] \n", hdmiInput.numHdmiInputs );
            	m_numofHdmiInput = hdmiInput.numHdmiInputs;
            }
			else
			{
				LOGINFO("Not able to get Numebr of inputs so defaulting to 3 \n");
				m_numofHdmiInput = 3;
			}

			LOGINFO("initalize inputs \n");

			for ( int i=0; i<m_numofHdmiInput; i++ )
			{
				HdmiPortMap hdmiPort((uint8_t)i);
				LOGINFO(" Add to vector [%d] \n", i);
				hdmiInputs.push_back(hdmiPort);
			}

			LOGINFO("Check the HDMI State \n");

			CheckHdmiInState();

            int cecMgrIsAvailableParam;
            err = IARM_Bus_Call(IARM_BUS_CECMGR_NAME,
                            IARM_BUS_CECMGR_API_isAvailable,
                            (void *)&cecMgrIsAvailableParam,
                            sizeof(cecMgrIsAvailableParam));

	    if(err == IARM_RESULT_SUCCESS) {
                LOGINFO("RDK CECDaemon up and running. IARM Call: IARM_BUS_CECMGR_API_isAvailable successful... \n");
            }
	    else {
                LOGINFO("RDK CECDaemon not up yet. IARM Call: IARM_BUS_CECMGR_API_isAvailable failed !!! \n");
            }
            if (cecSettingEnabled && (err == IARM_RESULT_SUCCESS))
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
            getHdmiArcPortID();
            
       }

       HdmiCecSink::~HdmiCecSink()
       {
       }

       void HdmiCecSink::Deinitialize(PluginHost::IShell* /* service */)
       {
	    CECDisable();
	    m_currentArcRoutingState = ARC_STATE_ARC_EXIT;

            m_semSignaltoArcRoutingThread.release();

            try
	    {
		if (m_arcRoutingThread.joinable())
                	m_arcRoutingThread.join();
	    }
	    catch(const std::system_error& e)
	    {
		LOGERR("system_error exception in thread join %s", e.what());
	    }
	    catch(const std::exception& e)
	    {
		LOGERR("exception in thread join %s", e.what());
	    }

	    {
	        m_sendKeyEventThreadExit = true;
                std::unique_lock<std::mutex> lk(m_sendKeyEventMutex);
                m_sendKeyEventThreadRun = true;
                m_sendKeyCV.notify_one();
            }

	    try
	    {
            if (m_sendKeyEventThread.joinable())
                m_sendKeyEventThread.join();
	    }
	    catch(const std::system_error& e)
	    {
		    LOGERR("system_error exception in thread join %s", e.what());
	    }
	    catch(const std::exception& e)
	    {
		    LOGERR("exception in thread join %s", e.what());
	    }

            HdmiCecSink::_instance = nullptr;
            DeinitializeIARM();
	    LOGWARN(" HdmiCecSink Deinitialize() Done");
       }

       const void HdmiCecSink::InitializeIARM()
       {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG, dsHdmiEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED, pwrMgrModeChangeEventHandler) );
           }
       }

       void HdmiCecSink::DeinitializeIARM()
       {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,IARM_BUS_PWRMGR_EVENT_MODECHANGED) );
            }
       }

       void HdmiCecSink::cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSink::_instance)
                return;

            if( !strcmp(owner, IARM_BUS_CECMGR_NAME))
            {
                switch (eventId)
                {
                    case IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED:
                    {
			LOGINFO("Received IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED event \r\n");
                        HdmiCecSink::_instance->onCECDaemonInit();
                    }
                    break;
                    case IARM_BUS_CECMGR_EVENT_STATUS_UPDATED:
                    {
                        IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = new IARM_Bus_CECMgr_Status_Updated_Param_t;
                        if(evtData)
                        {
                            memcpy(evtData,data,sizeof(IARM_Bus_CECMgr_Status_Updated_Param_t));
                            HdmiCecSink::_instance->cecStatusUpdated(evtData);
                        }
                    }
                    break;
                    default:
                    /*Do nothing*/
                    break;
                }
            }
       }

       void HdmiCecSink::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSink::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                bool isHdmiConnected = eventData->data.hdmi_in_connect.isPortConnected;
                dsHdmiInPort_t portId = eventData->data.hdmi_in_connect.port;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG event port: %d data:%d \r\n",portId,  isHdmiConnected);
                HdmiCecSink::_instance->onHdmiHotPlug(portId,isHdmiConnected);
            }
       }

       void HdmiCecSink::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            if(!HdmiCecSink::_instance)
                return;

            if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
                if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
                    IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
                    LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                            param->data.state.curState, param->data.state.newState);
		           LOGWARN(" m_logicalAddressAllocated 0x%x CEC enable status %d \n",_instance->m_logicalAddressAllocated,_instance->cecEnableStatus);
                    if(param->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                    {
                        powerState = DEVICE_POWER_STATE_ON; 
					}
                    else
                   	{
                            powerState = DEVICE_POWER_STATE_OFF;
                            if((_instance->m_currentArcRoutingState == ARC_STATE_REQUEST_ARC_INITIATION) || (_instance->m_currentArcRoutingState == ARC_STATE_ARC_INITIATED))
                            {
                                LOGINFO("%s: Stop ARC \n",__FUNCTION__);
                                _instance->stopArc();
			    }

                   	}
                        if (_instance->cecEnableStatus)
		        {
					if ( _instance->m_logicalAddressAllocated != LogicalAddress::UNREGISTERED )
					{
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_powerStatus = PowerStatus(powerState);

						if ( powerState != DEVICE_POWER_STATE_ON )
						{
						   /*  set the current active source to TV on going to standby */
                                                   HdmiCecSink::_instance->m_currentActiveSource = _instance->m_logicalAddressAllocated;
						}
                                                /* Initiate a ping straight away */
                                                HdmiCecSink::_instance->m_pollNextState = POLL_THREAD_STATE_PING;
                                                HdmiCecSink::_instance->m_ThreadExitCV.notify_one();
					}
			}
			else
			{
				LOGWARN("CEC not Enabled\n");
			}
                }
           }
       }


	  void HdmiCecSink::sendStandbyMessage()
      {
      		if(!HdmiCecSink::_instance)
				return;
          if(!(HdmiCecSink::_instance->smConnection))
              return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(Standby()), 1000);
       } 

	   void HdmiCecSink::wakeupFromStandby()
	   {
		   if ( powerState == DEVICE_POWER_STATE_OFF )
		   {
		   		IARM_Bus_PWRMgr_SetPowerState_Param_t param;
				IARM_Result_t ret;

				LOGINFO("Wakeup Device From standby ");
				param.newState =  IARM_BUS_PWRMGR_POWERSTATE_ON;
				ret = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_SetPowerState, (void *)&param, sizeof(param));

				if ( ret != IARM_RESULT_SUCCESS )
				{
					LOGERR("Failed to wakeup from standby");
				}
		   	}
	  }

       void HdmiCecSink::onCECDaemonInit()
       {
           if(cecSettingEnabled) {
                if(true == getEnabled())
                {
		    LOGINFO("CEC getEnabled() already TRUE. Disable and enable CEC again\n ");
                    setEnabled(false);
                    setEnabled(true);
                }
                else
                {
		    LOGINFO("CEC getEnabled() FALSE. Enable CEC\n ");
                    setEnabled(true);
                }
           }
           else {
               LOGINFO("cecSettingEnabled FALSE. Do Nothing....\n ");
           }
       }

       void HdmiCecSink::cecStatusUpdated(void *evtStatus)
       {
            IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = (IARM_Bus_CECMgr_Status_Updated_Param_t *)evtStatus;
            if(evtData)
            {
                delete evtData;
            }
           return;
       }

       void HdmiCecSink::onHdmiHotPlug(int portId , int connectStatus)
       {
        	bool previousHdmiState = m_isHdmiInConnected;
			int i = 0;
			LOGINFO("onHdmiHotPlug Status : %d ", connectStatus);
                        if(!connectStatus)
                        {
                            LOGINFO(" removeDevice port: %d Logical address :%d  \r\n",portId,hdmiInputs[portId].m_logicalAddr.toInt() );
                            _instance->removeDevice(hdmiInputs[portId].m_logicalAddr.toInt());
                        }
			CheckHdmiInState();

          if(cecEnableStatus) {
              LOGINFO("cecEnableStatus : %d Trigger CEC Ping !!! \n", cecEnableStatus);
              m_pollNextState = POLL_THREAD_STATE_PING;
              m_ThreadExitCV.notify_one();
          }
          if( HdmiArcPortID >= 0 ) {
              updateArcState();  
          }
          return;
       }
       void HdmiCecSink::updateArcState()
       {
           if ( m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED )
           {
        	if (!(hdmiInputs[HdmiArcPortID].m_isConnected))
		{
                   std::lock_guard<std::mutex> lock(_instance->m_arcRoutingStateMutex);
		   m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
		}
                else
                {
                   LOGINFO("updateArcState :not updating ARC state current arc state %d ",m_currentArcRoutingState);
                }
           } 
      }
      void HdmiCecSink::arcStartStopTimerFunction()
      {
           JsonObject params;

	   if (m_arcstarting)
	   {
               LOGINFO("arcStartStopTimerFunction ARC start timer expired");
	       LOGINFO("notify_device setting that Initiate ARC failed to get the ARC_STATE_ARC_INITIATED state\n");
               params["status"] = string("failure");
               sendNotify(eventString[HDMICECSINK_EVENT_ARC_INITIATION_EVENT], params); 
	   }
	   else
	   {
	      LOGINFO("arcStartStopTimerFunction ARC stop timer expired");
	      LOGINFO("notify_device setting that Terminate  ARC failed to get the ARC_STATE_ARC_TERMINATED state\n");
              params["status"] = string("failure");
              sendNotify(eventString[HDMICECSINK_EVENT_ARC_TERMINATION_EVENT], params);
 
		       
	   }
             /* bring the state machine to the clean state for a new start */ 
           std::lock_guard<std::mutex> lock(_instance->m_arcRoutingStateMutex);
           m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
       }
       void  HdmiCecSink::Send_ShortAudioDescriptor_Event(JsonArray audiodescriptor)
       {
           JsonObject params;

	   LOGINFO("Notify the DS ");
           params["ShortAudioDescriptor"]= JsonValue(audiodescriptor);
	   sendNotify(eventString[HDMICECSINK_EVENT_SHORT_AUDIODESCRIPTOR_EVENT], params);
       }

       void HdmiCecSink::Process_ShortAudioDescriptor_msg(const ReportShortAudioDescriptor  &msg)
       {
	    uint8_t numberofdescriptor = msg.numberofdescriptor;
            uint8_t AudioformatCode;
            uint8_t  atmos;
	    uint32 descriptor =0;
	    JsonArray audiodescriptor;

	    if (numberofdescriptor)
            {
	     for( uint8_t  i=0; i < numberofdescriptor; i++)
            {
               descriptor = msg.shortAudioDescriptor[i].getAudiodescriptor();

	       LOGINFO("descriptor%d 0x%x\n",i,descriptor);
	       audiodescriptor.Add(descriptor);

	    }
	    }
	    else
	    {
		    audiodescriptor.Add(descriptor);
	    }
	   HdmiCecSink::_instance->Send_ShortAudioDescriptor_Event(audiodescriptor);
        }

        void HdmiCecSink::Process_SetSystemAudioMode_msg(const SetSystemAudioMode &msg)
        {
            JsonObject params;
            if(!HdmiCecSink::_instance)
               return;

             //DD: Check cecSettingEnabled to prevent race conditions which gives immediate UI setting status
            //SetSystemAudioMode message may come from AVR/Soundbar while CEC disable is in-progress
            if ( cecSettingEnabled != true  )
            {
                 LOGINFO("Process SetSystemAudioMode from Audio device: Cec is disabled-> EnableCEC first");
                 return;
            }

	    if ( (msg.status.toInt() == 0x00) && (m_currentArcRoutingState == ARC_STATE_ARC_INITIATED))
            {
		/* ie system audio mode off -> amplifier goign to standby but still ARC is in initiated state,stop ARC and 
		 bring the ARC state machine to terminated state*/
                 LOGINFO("system audio mode off message but arc is not in terminated state so stopping ARC");
		 stopArc();

            }

            params["audioMode"] = msg.status.toString().c_str();
            sendNotify(eventString[HDMICECSINK_EVENT_SYSTEM_AUDIO_MODE], params);
         }
         void HdmiCecSink::Process_ReportAudioStatus_msg(const ReportAudioStatus msg)
         {
            JsonObject params;
            if(!HdmiCecSink::_instance)
               return;
			LOGINFO("Command: ReportAudioStatus  %s audio Mute status %d  means %s  and current Volume level is %d \n",GetOpName(msg.opCode()),msg.status.getAudioMuteStatus(),msg.status.toString().c_str(),msg.status.getAudioVolume());
            params["muteStatus"]  = msg.status.getAudioMuteStatus();
            params["volumeLevel"] = msg.status.getAudioVolume();
            sendNotify(eventString[HDMICECSINK_EVENT_REPORT_AUDIO_STATUS], params);

         }
		 void HdmiCecSink::sendKeyPressEvent(const int logicalAddress, int keyCode)
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
		 void HdmiCecSink::sendKeyReleaseEvent(const int logicalAddress)
		 {
                    if(!(_instance->smConnection))
                        return;
		 _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlReleased()), 100);

		 }
         void  HdmiCecSink::sendDeviceUpdateInfo(const int logicalAddress)
         {
            JsonObject params;
            params["logicalAddress"] = JsonValue(logicalAddress);
            sendNotify(eventString[HDMICECSINK_EVENT_DEVICE_INFO_UPDATED], params);
        }
	void HdmiCecSink::systemAudioModeRequest()
        {
	     if ( cecEnableStatus != true  )
	     {
               LOGINFO("systemAudioModeRequest: Cec is disabled-> EnableCEC first");
              return;
             } 

            if(!HdmiCecSink::_instance)
             return;
            if(!(_instance->smConnection))
                return;
             LOGINFO(" Send systemAudioModeRequest ");
           _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(SystemAudioModeRequest(physical_addr)), 1000);

        }
         void HdmiCecSink::sendGiveAudioStatusMsg()
        {
            if(!HdmiCecSink::_instance)
             return;
            if(!(_instance->smConnection))
                return;
             LOGINFO(" Send GiveAudioStatus ");
	      _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(GiveAudioStatus()), 100);

        }
        void  HdmiCecSink::reportAudioDevicePowerStatusInfo(const int logicalAddress, const int powerStatus)
        {
            JsonObject params;
            params["powerStatus"] = JsonValue(powerStatus);
            LOGINFO("Notify DS!!! logicalAddress = %d , Audio device power status = %d \n", logicalAddress, powerStatus);
            m_audioDevicePowerStatusRequested = false;
            sendNotify(eventString[HDMICECSINK_EVENT_AUDIO_DEVICE_POWER_STATUS], params);
        }

        void HdmiCecSink::SendStandbyMsgEvent(const int logicalAddress)
        {
            JsonObject params;
	    if(!HdmiCecSink::_instance)
		return;
	    params["logicalAddress"] = JsonValue(logicalAddress);
            sendNotify(eventString[HDMICECSINK_EVENT_STANDBY_MSG_EVENT], params);
       }
       uint32_t HdmiCecSink::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

            setEnabled(enabled);
            returnResponse(true);
       }

       uint32_t HdmiCecSink::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getEnabled();
            returnResponse(true);
       }

       uint32_t HdmiCecSink::getAudioDeviceConnectedStatusWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["connected"] = getAudioDeviceConnectedStatus();
            returnResponse(true);
       }

       uint32_t HdmiCecSink::requestAudioDevicePowerStatusWrapper(const JsonObject& parameters, JsonObject& response)
       {
            requestAudioDevicePowerStatus();
            returnResponse(true);
       }

	  uint32_t HdmiCecSink::getActiveSourceWrapper(const JsonObject& parameters, JsonObject& response)
       {
       		char routeString[1024] = {'\0'};
			int length = 0;
			std::stringstream temp;
			
       		if ( HdmiCecSink::_instance->m_currentActiveSource != -1 )
			{
				int n = HdmiCecSink::_instance->m_currentActiveSource;
				response["available"] = true;
				response["logicalAddress"] = HdmiCecSink::_instance->deviceList[n].m_logicalAddress.toInt();
				response["physicalAddress"] = HdmiCecSink::_instance->deviceList[n].m_physicalAddr.toString().c_str();
				response["deviceType"] = HdmiCecSink::_instance->deviceList[n].m_deviceType.toString().c_str();
				response["cecVersion"] = HdmiCecSink::_instance->deviceList[n].m_cecVersion.toString().c_str();
				response["osdName"] = HdmiCecSink::_instance->deviceList[n].m_osdName.toString().c_str();
				response["vendorID"] = HdmiCecSink::_instance->deviceList[n].m_vendorID.toString().c_str();
				response["powerStatus"] = HdmiCecSink::_instance->deviceList[n].m_powerStatus.toString().c_str();

				if ( HdmiCecSink::_instance->deviceList[n].m_physicalAddr.getByteValue(0) != 0 )
				{
					sprintf(&routeString[length], "%s%d", "HDMI",(HdmiCecSink::_instance->deviceList[n].m_physicalAddr.getByteValue(0) - 1));
				}
				else if ( HdmiCecSink::_instance->deviceList[n].m_physicalAddr.getByteValue(0) == 0 )
				{
					sprintf(&routeString[length], "%s", "TV");
				}
				
				temp << (char *)routeString;
				response["port"] = temp.str();
				
			}
			else
			{
				response["available"] = false;
			}
			
            returnResponse(true);
       }

       uint32_t HdmiCecSink::getDeviceListWrapper(const JsonObject& parameters, JsonObject& response)
       {
           LOGINFOMETHOD();

                        response["numberofdevices"] = HdmiCecSink::_instance->m_numberOfDevices;
                        LOGINFO("getDeviceListWrapper  m_numberOfDevices :%d \n", HdmiCecSink::_instance->m_numberOfDevices);
			JsonArray deviceList;
			
			for (unsigned int n = 0; n < LogicalAddress::UNREGISTERED; n++)
			{

				if ( n != HdmiCecSink::_instance->m_logicalAddressAllocated && 
						HdmiCecSink::_instance->deviceList[n].m_isDevicePresent )
				{
					JsonObject device;
			
					device["logicalAddress"] = HdmiCecSink::_instance->deviceList[n].m_logicalAddress.toInt();
					device["physicalAddress"] = HdmiCecSink::_instance->deviceList[n].m_physicalAddr.toString().c_str();
					device["deviceType"] = HdmiCecSink::_instance->deviceList[n].m_deviceType.toString().c_str();
					device["cecVersion"] = HdmiCecSink::_instance->deviceList[n].m_cecVersion.toString().c_str();
					device["osdName"] = HdmiCecSink::_instance->deviceList[n].m_osdName.toString().c_str();
					device["vendorID"] = HdmiCecSink::_instance->deviceList[n].m_vendorID.toString().c_str();
					device["powerStatus"] = HdmiCecSink::_instance->deviceList[n].m_powerStatus.toString().c_str();
                                        int hdmiPortNumber = -1;
                                        LOGINFO("getDeviceListWrapper  m_numofHdmiInput:%d looking for Logical Address :%d \n", m_numofHdmiInput, HdmiCecSink::_instance->deviceList[n].m_logicalAddress.toInt());
                                        for (int i=0; i < m_numofHdmiInput; i++)
                                        {
                                             LOGINFO("getDeviceListWrapper  connected : %d, portid:%d LA: %d  \n", hdmiInputs[i].m_isConnected, hdmiInputs[i].m_portID, hdmiInputs[i].m_logicalAddr.toInt());
                                             if(hdmiInputs[i].m_isConnected  && hdmiInputs[i].m_logicalAddr.toInt() == HdmiCecSink::_instance->deviceList[n].m_logicalAddress.toInt())
                                             {
                                                 hdmiPortNumber = hdmiInputs[i].m_portID;
                                                 LOGINFO("got portid :%d break \n", hdmiPortNumber);
                                                 break;
                                             }
                                        }
                                        device["portNumber"] = hdmiPortNumber;
                                        deviceList.Add(device);
				}
			}

			response["deviceList"] = deviceList;

            returnResponse(true);
       }


       uint32_t HdmiCecSink::setOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t HdmiCecSink::getOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
        {
            response["name"] = osdName.toString(); 
            LOGINFO("getOSDNameWrapper osdName : %s \n",osdName.toString().c_str());
            returnResponse(true);
        }

		uint32_t HdmiCecSink::printDeviceListWrapper(const JsonObject& parameters, JsonObject& response)
        {
        	printDeviceList();
            response["printed"] = true;
            returnResponse(true);
        }

		uint32_t HdmiCecSink::setActiveSourceWrapper(const JsonObject& parameters, JsonObject& response)
        {
        	setActiveSource(false);
        	returnResponse(true);
        }

		uint32_t HdmiCecSink::setActivePathWrapper(const JsonObject& parameters, JsonObject& response)
        {
         	if (parameters.HasLabel("activePath"))
            {
                std::string id = parameters["activePath"].String();
				PhysicalAddress phy_addr = PhysicalAddress(id);

				LOGINFO("Addr = %s, length = %d", id.c_str(), id.length());

				setStreamPath(phy_addr);
				returnResponse(true);
            }
            else
            {
                returnResponse(false);
            }
        }

		uint32_t HdmiCecSink::getActiveRouteWrapper(const JsonObject& parameters, JsonObject& response)
        {
      	    std::vector<uint8_t> route;	
			char routeString[1024] = {'\0'};
			int length = 0;
			JsonArray pathList;
			std::stringstream temp;
			
         	if (HdmiCecSink::_instance->m_currentActiveSource != -1 &&
					HdmiCecSink::_instance->m_currentActiveSource != HdmiCecSink::_instance->m_logicalAddressAllocated )
            {
           		HdmiCecSink::_instance->getActiveRoute(LogicalAddress(HdmiCecSink::_instance->m_currentActiveSource), route);

				if (route.size())
				{
					response["available"] = true;
					response["length"] = route.size();

					for (int i=0; i < route.size(); i++) 
					{
						if ( route[i] != LogicalAddress::UNREGISTERED )
						{
							JsonObject device;
							
							device["logicalAddress"] = HdmiCecSink::_instance->deviceList[route[i]].m_logicalAddress.toInt();
							device["physicalAddress"] = HdmiCecSink::_instance->deviceList[route[i]].m_physicalAddr.toString().c_str();
							device["deviceType"] = HdmiCecSink::_instance->deviceList[route[i]].m_deviceType.toString().c_str();
							device["osdName"] = HdmiCecSink::_instance->deviceList[route[i]].m_osdName.toString().c_str();
							device["vendorID"] = HdmiCecSink::_instance->deviceList[route[i]].m_vendorID.toString().c_str();
										
							pathList.Add(device);
							
							sprintf(&routeString[length], "%s", _instance->deviceList[route[i]].m_logicalAddress.toString().c_str());
							length += _instance->deviceList[route[i]].m_logicalAddress.toString().length();
							sprintf(&routeString[length], "(%s", _instance->deviceList[route[i]].m_osdName.toString().c_str());
							length += _instance->deviceList[route[i]].m_osdName.toString().length();
							sprintf(&routeString[length], "%s", ")-->");
							length += strlen(")-->");
							if( i + 1 ==  route.size() )
							{
								sprintf(&routeString[length], "%s%d", "HDMI",(HdmiCecSink::_instance->deviceList[route[i]].m_physicalAddr.getByteValue(0) - 1));
							}
						}
					}

					response["pathList"] = pathList;
					temp << (char *)routeString;
					response["ActiveRoute"] = temp.str(); 
					LOGINFO("ActiveRoute = [%s]", routeString);
				}
				
            }
			else if ( HdmiCecSink::_instance->m_currentActiveSource == HdmiCecSink::_instance->m_logicalAddressAllocated )
			{
				response["available"] = true;
				response["ActiveRoute"] = "TV";
			}
            else
            {
                response["available"] = false;
            }

			returnResponse(true);
        }

		uint32_t HdmiCecSink::requestActiveSourceWrapper(const JsonObject& parameters, JsonObject& response)
		{
			requestActiveSource();
			returnResponse(true);
		}

		uint32_t HdmiCecSink::setRoutingChangeWrapper(const JsonObject& parameters, JsonObject& response)
        {
        	std::string oldPortID;
			std::string newPortID;

			returnIfParamNotFound(parameters, "oldPort");
			returnIfParamNotFound(parameters, "newPort");

			oldPortID = parameters["oldPort"].String();
			newPortID = parameters["newPort"].String();
			

			if ((oldPortID.find("HDMI",0) != std::string::npos ||
					oldPortID.find("TV",0) != std::string::npos ) &&
					( newPortID.find("HDMI", 0) != std::string::npos ||
						newPortID.find("TV", 0) != std::string::npos ))
			{
				setRoutingChange(oldPortID, newPortID);
				returnResponse(true);
			}
			else
			{
				returnResponse(false);
			}
	   }
		

	   uint32_t HdmiCecSink::setMenuLanguageWrapper(const JsonObject& parameters, JsonObject& response)
       {
        	std::string lang;

			returnIfParamNotFound(parameters, "language");

			lang = parameters["language"].String();

			setCurrentLanguage(Language(lang.data()));
			sendMenuLanguage();
			returnResponse(true);
	   }
	   

        uint32_t HdmiCecSink::setVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
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
        uint32_t HdmiCecSink::setArcEnableDisableWrapper(const JsonObject& parameters, JsonObject& response)
       {
           
            bool enabled = false;

            if (parameters.HasLabel("enabled"))
            {
                getBoolParameter("enabled", enabled);
            }
            else
            {
                returnResponse(false);
            }
            if(enabled)
	    {
	         startArc();
	    }
	    else
	    {
		 stopArc();
			
	    }
            
            returnResponse(true);
       }
        uint32_t HdmiCecSink::getVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO("getVendorIdWrapper  appVendorId : %s  \n",appVendorId.toString().c_str());
            response["vendorid"] = appVendorId.toString() ;
            returnResponse(true);
        }

        uint32_t HdmiCecSink::requestShortAudioDescriptorWrapper(const JsonObject& parameters, JsonObject& response)
	{
			requestShortaudioDescriptor();
			returnResponse(true);
	}
        uint32_t HdmiCecSink::sendStandbyMessageWrapper(const JsonObject& parameters, JsonObject& response)
        {
          sendStandbyMessage();
	  returnResponse(true);
        }

        uint32_t HdmiCecSink::sendAudioDevicePowerOnMsgWrapper(const JsonObject& parameters, JsonObject& response)
        {
	    LOGINFO("%s invoked. \n",__FUNCTION__);
            systemAudioModeRequest();
	    returnResponse(true);
        }
		uint32_t HdmiCecSink::sendRemoteKeyPressWrapper(const JsonObject& parameters, JsonObject& response)
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
			LOGINFO("Post send key press event to queue size:%d \n",m_SendKeyQueue.size());
			returnResponse(true);
		}
	   uint32_t HdmiCecSink::sendGiveAudioStatusWrapper(const JsonObject& parameters, JsonObject& response)
           {
	      sendGiveAudioStatusMsg();
	      returnResponse(true);
	   }
        bool HdmiCecSink::loadSettings()
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
                    LOGINFO("CEC_SETTING_OSD_NAME present osd_name :%s",osdName.toString().c_str());
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
                parameters[CEC_SETTING_OSD_NAME] = osdName.toString();
                parameters[CEC_SETTING_VENDOR_ID] = vendorId;

                cecSettingEnabled = true;
                cecOTPSettingEnabled = true;
                parameters.IElement::ToFile(file);

                file.Close();

            }

            return cecSettingEnabled;
        }

        void HdmiCecSink::setEnabled(bool enabled)
        {
           LOGINFO("Entered setEnabled: %d  cecSettingEnabled :%d ",enabled, cecSettingEnabled);

           if (cecSettingEnabled != enabled)
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

		void HdmiCecSink::updateImageViewOn(const int logicalAddress)
		{
			JsonObject params;
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ||
					logicalAddress == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if (_instance->deviceList[logicalAddress].m_isDevicePresent &&
					_instance->deviceList[_instance->m_logicalAddressAllocated].m_powerStatus.toInt() == PowerStatus::STANDBY)
			
			{
	                       /* Bringing TV out of standby is handled by application.notify UI to bring the TV out of standby */
				sendNotify(eventString[HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY], params);
			}

			sendNotify(eventString[HDMICECSINK_EVENT_IMAGE_VIEW_ON_MSG], params);
		}

		void HdmiCecSink::updateTextViewOn(const int logicalAddress)
		{
			JsonObject params;
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ||
					logicalAddress == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if (_instance->deviceList[logicalAddress].m_isDevicePresent &&
					_instance->deviceList[_instance->m_logicalAddressAllocated].m_powerStatus.toInt() == PowerStatus::STANDBY)
			{
			        /* Bringing TV out of standby is handled by application.notify UI to bring the TV out of standby */
				sendNotify(eventString[HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY], params);
			}

			sendNotify(eventString[HDMICECSINK_EVENT_TEXT_VIEW_ON_MSG], params);
		}

		
		void HdmiCecSink::updateDeviceChain(const LogicalAddress &logicalAddress, const PhysicalAddress &phy_addr)
		{
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ||
					logicalAddress.toInt() == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if (_instance->deviceList[logicalAddress.toInt()].m_isDevicePresent &&
					logicalAddress.toInt() != _instance->m_logicalAddressAllocated)
			{
				for (int i=0; i < m_numofHdmiInput; i++) 
				{
					LOGINFO(" addr = %d, portID = %d", phy_addr.getByteValue(0), hdmiInputs[i].m_portID);
					if (phy_addr.getByteValue(0) == (hdmiInputs[i].m_portID + 1)) {
						hdmiInputs[i].addChild(logicalAddress, phy_addr);
					}
				}
			}
		}

		void HdmiCecSink::getActiveRoute(const LogicalAddress &logicalAddress, std::vector<uint8_t> &route)
		{
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ||
					logicalAddress.toInt() == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if (_instance->deviceList[logicalAddress.toInt()].m_isDevicePresent &&
					logicalAddress.toInt() != _instance->m_logicalAddressAllocated &&
						_instance->deviceList[logicalAddress.toInt()].m_isActiveSource )
			{
				route.clear();
				for (int i=0; i < m_numofHdmiInput; i++) 
				{
					LOGINFO("physicalAddress = [%d], portID = %d", _instance->deviceList[logicalAddress.toInt()].m_physicalAddr.getByteValue(0), hdmiInputs[i].m_portID);
					if (_instance->deviceList[logicalAddress.toInt()].m_physicalAddr.getByteValue(0) == (hdmiInputs[i].m_portID + 1)) {
						hdmiInputs[i].getRoute(_instance->deviceList[logicalAddress.toInt()].m_physicalAddr, route);
					}
				}
			}
			else {
				LOGERR("Not in correct state to Find Route");
			}
		}


		void HdmiCecSink::CheckHdmiInState()
		{
			int err;
			bool isAnyPortConnected = false;
			
			dsHdmiInGetStatusParam_t params;
            err = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            IARM_BUS_DSMGR_API_dsHdmiInGetStatus,
                            (void *)&params,
                            sizeof(params));
			
            if(err == IARM_RESULT_SUCCESS && params.result == dsERR_NONE )
            {
           		for( int i = 0; i < m_numofHdmiInput; i++ )
           		{
           		    LOGINFO("Is HDMI In Port [%d] connected [%d] \n",i, params.status.isPortConnected[i]);
					if ( params.status.isPortConnected[i] )
					{
						isAnyPortConnected = true;
					}

					LOGINFO("update Port Status [%d] \n", i);
					hdmiInputs[i].update(params.status.isPortConnected[i]);
           		}
			}

			if ( isAnyPortConnected ) {
				m_isHdmiInConnected = true;
			} else {
				m_isHdmiInConnected = false;
			}
		}

		void HdmiCecSink::requestActiveSource()
		{
			if(!HdmiCecSink::_instance)
				return;

                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			_instance->smConnection->sendTo(LogicalAddress::BROADCAST, 
										MessageEncoder().encode(RequestActiveSource()), 500);
		}
		
		void HdmiCecSink::setActiveSource(bool isResponse)
		{
			if(!HdmiCecSink::_instance)
				return;

                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if (isResponse && (_instance->m_currentActiveSource != _instance->m_logicalAddressAllocated) )
			{
				LOGWARN("TV is not current Active Source");
				return;
			}
		
			_instance->smConnection->sendTo(LogicalAddress::BROADCAST, 
										MessageEncoder().encode(ActiveSource(_instance->deviceList[_instance->m_logicalAddressAllocated].m_physicalAddr)), 500);
			_instance->m_currentActiveSource = _instance->m_logicalAddressAllocated;
		}

		void HdmiCecSink::setCurrentLanguage(const Language &lang)
		{
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			_instance->deviceList[_instance->m_logicalAddressAllocated].m_currentLanguage = lang;
		}
	
		void HdmiCecSink::sendMenuLanguage()
		{
			Language lang = "NA";
			if(!HdmiCecSink::_instance)
				return;

                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			lang = _instance->deviceList[_instance->m_logicalAddressAllocated].m_currentLanguage;

			_instance->smConnection->sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(SetMenuLanguage(lang)), 100);
		}

		void HdmiCecSink::updateInActiveSource(const int logical_address, const InActiveSource &source )
		{
			JsonObject params;
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if( logical_address != _instance->m_logicalAddressAllocated )
			{
				_instance->deviceList[logical_address].m_isActiveSource = false;
				
				if ( _instance->m_currentActiveSource == logical_address )
				{
					_instance->m_currentActiveSource = -1;
				}

				params["logicalAddress"] = JsonValue(logical_address);
				params["phsicalAddress"] = source.physicalAddress.toString().c_str(); 
				sendNotify(eventString[HDMICECSINK_EVENT_INACTIVE_SOURCE], params);
			}
		}

	void HdmiCecSink::updateActiveSource(const int logical_address, const ActiveSource &source )
       	{
       		JsonObject params;
			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

			if( logical_address != _instance->m_logicalAddressAllocated )
			{
				if ( _instance->m_currentActiveSource != -1 )
				{
					_instance->deviceList[_instance->m_currentActiveSource].m_isActiveSource = false;
				}

				_instance->deviceList[logical_address].m_isActiveSource = true;
				_instance->deviceList[logical_address].update(source.physicalAddress);
				_instance->m_currentActiveSource = logical_address;

				if (_instance->deviceList[logical_address].m_isDevicePresent &&
									_instance->deviceList[_instance->m_logicalAddressAllocated].m_powerStatus.toInt() == PowerStatus::STANDBY)
				{
					 /* Bringing TV out of standby is handled by application.notify UI to bring the TV out of standby */
					sendNotify(eventString[HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY], params);
				}

				params["logicalAddress"] = JsonValue(logical_address);
				params["physicalAddress"] = _instance->deviceList[logical_address].m_physicalAddr.toString().c_str(); 
				sendNotify(eventString[HDMICECSINK_EVENT_ACTIVE_SOURCE_CHANGE], params);
			}
       	}

                void HdmiCecSink::requestShortaudioDescriptor()
	        {
		        if ( cecEnableStatus != true  )
                        {
                             LOGINFO("requestShortaudioDescriptor: cec is disabled-> EnableCEC first");
                             return;
                        }

		        if(!HdmiCecSink::_instance)
				return;

                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}

                        LOGINFO(" Send requestShortAudioDescriptor Message ");
                    _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(RequestShortAudioDescriptor(formatid,audioFormatCode,numberofdescriptor)), 1000);

		}

                void HdmiCecSink::requestAudioDevicePowerStatus()
                {
                        if ( cecEnableStatus != true  )
                        {
                             LOGWARN("cec is disabled-> EnableCEC first");
                             return;
                        }

                        if(!HdmiCecSink::_instance)
                                return;

                        if(!(_instance->smConnection))
                            return;
                        if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
                                LOGERR("Logical Address NOT Allocated");
                                return;
                        }

                        LOGINFO(" Send GiveDevicePowerStatus Message to Audio system in the network \n");
                        _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM, MessageEncoder().encode(GiveDevicePowerStatus()), 500);

			m_audioDevicePowerStatusRequested = true;
                }

		void HdmiCecSink::sendFeatureAbort(const LogicalAddress logicalAddress, const OpCode feature, const AbortReason reason)
	        {

                       if(!HdmiCecSink::_instance)
                               return;
                       if(!(_instance->smConnection))
                           return;
		       LOGINFO(" Sending FeatureAbort to %s for opcode %s with reason %s ",logicalAddress.toString().c_str(),feature.toString().c_str(),reason.toString().c_str());
                       _instance->smConnection->sendTo(logicalAddress, MessageEncoder().encode(FeatureAbort(feature,reason)), 500);
                 }
	void HdmiCecSink::pingDevices(std::vector<int> &connected , std::vector<int> &disconnected)
        {
        	int i;

		if(!HdmiCecSink::_instance)
                return;
                if(!(_instance->smConnection))
                    return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated");
				return;
			}
			
            for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				if ( i != _instance->m_logicalAddressAllocated )
				{
					//LOGWARN("PING for  0x%x \r\n",i);
					try {
						_instance->smConnection->ping(LogicalAddress(_instance->m_logicalAddressAllocated), LogicalAddress(i), Throw_e());
					}
					catch(CECNoAckException &e)
					{
						if ( _instance->deviceList[i].m_isDevicePresent ) {
							disconnected.push_back(i);
						}
                                                //LOGWARN("Ping device: 0x%x caught %s \r\n", i, e.what());
						usleep(50000);
						continue;
					}
					  catch(Exception &e)
					  {
						LOGWARN("Ping device: 0x%x caught %s \r\n", i, e.what());
                                                usleep(50000);
                                                continue;
					  }
					  
					  /* If we get ACK, then the device is present in the network*/
					  if ( !_instance->deviceList[i].m_isDevicePresent )
					  {
					  	connected.push_back(i);
                                                //LOGWARN("Ping success, added device: 0x%x \r\n", i);
					  }
					  usleep(50000);      
				}
           	}
        }

		int HdmiCecSink::requestType( const int logicalAddress ) {
			int requestType = CECDeviceParams::REQUEST_NONE;
			
			if ( !_instance->deviceList[logicalAddress].m_isPAUpdated || !_instance->deviceList[logicalAddress].m_isDeviceTypeUpdated ) {
				requestType = CECDeviceParams::REQUEST_PHISICAL_ADDRESS; 	
			}else if ( !_instance->deviceList[logicalAddress].m_isOSDNameUpdated ) {
				requestType = CECDeviceParams::REQUEST_OSD_NAME;
			}else if ( !_instance->deviceList[logicalAddress].m_isVersionUpdated ) {
				requestType = CECDeviceParams::REQUEST_CEC_VERSION;
			}else if ( !_instance->deviceList[logicalAddress].m_isVendorIDUpdated ) {
				requestType = CECDeviceParams::REQUEST_DEVICE_VENDOR_ID;
			}else if ( !_instance->deviceList[logicalAddress].m_isPowerStatusUpdated ) {
				requestType = CECDeviceParams::REQUEST_POWER_STATUS;
			}

			return requestType;
		}

		void HdmiCecSink::printDeviceList() {
			int i;

			if(!HdmiCecSink::_instance)
				return;

			for(i=0; i< 16; i++)
			{
				if (HdmiCecSink::_instance->deviceList[i].m_isDevicePresent) {
				LOGWARN("------ Device ID = %d--------", i);
				HdmiCecSink::_instance->deviceList[i].printVariable();
				LOGWARN("-----------------------------");
				}
			}
		}

		int HdmiCecSink::findLogicalAddress( const PhysicalAddress &physical_addr) {

			int i;
			int logicalAddr = LogicalAddress::UNREGISTERED;
			
			if(!HdmiCecSink::_instance)
				return LogicalAddress::UNREGISTERED;
		
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return LogicalAddress::UNREGISTERED;
			}

			for(i=0; i< LogicalAddress::UNREGISTERED; i++)
			{
				if ( _instance->deviceList[i].m_isDevicePresent && ( _instance->deviceList[i].m_physicalAddr.toString() == physical_addr.toString() ))
				{
					logicalAddr = i;
					LOGINFO("Found Logical Address %d for physical addr %s", logicalAddr, physical_addr.toString().c_str());
					break;
				}
			}

			LOGINFO("Logical Address %d for physical addr %s", logicalAddr, physical_addr.toString().c_str());

			return logicalAddr;
		}

		
		void HdmiCecSink::sendPowerOFFCommand( const PhysicalAddress &physical_addr ) 
		{
			if(!HdmiCecSink::_instance)
				return;
		
                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			LOGINFO("sendPowerOFFCommand");

			if ( _instance->m_currentActiveSource != -1 && _instance->m_currentActiveSource != _instance->m_logicalAddressAllocated )
			{
				LOGINFO("Checking the Phy addr ");
				if ( physical_addr.toString() != _instance->deviceList[_instance->m_currentActiveSource].m_physicalAddr.toString() )
				{
					LOGINFO("Sending Power OFF ");
					/* send Power OFF Function to turn OFF */
					_instance->smConnection->sendTo(LogicalAddress(_instance->m_currentActiveSource), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_POWER_OFF_FUNCTION)), 100);

					_instance->smConnection->sendTo(LogicalAddress(_instance->m_currentActiveSource), MessageEncoder().encode(UserControlReleased()), 100);
				}
			}
		}

		
		void HdmiCecSink::sendPowerONCommand( const PhysicalAddress &physical_addr ) 
		{
			int logicalAddr = LogicalAddress::UNREGISTERED;
			
			if(!HdmiCecSink::_instance)
				return;
		
                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			LOGINFO("sendPowerONCommand");

			logicalAddr = findLogicalAddress(physical_addr);

			if ( logicalAddr != LogicalAddress::UNREGISTERED && 
					logicalAddr != _instance->m_logicalAddressAllocated )
			{
				LOGINFO("checking the power status");

				if ( _instance->deviceList[logicalAddr].m_powerStatus.toInt() == PowerStatus::STANDBY ||
						_instance->deviceList[logicalAddr].m_powerStatus.toInt() == PowerStatus::POWER_STATUS_NOT_KNOWN )
				{
					LOGINFO("Sending Power ON");
					/* send Power ON Function to turn ON */
					_instance->smConnection->sendTo(LogicalAddress(logicalAddr), MessageEncoder().encode(UserControlPressed(UICommand::UI_COMMAND_POWER_ON_FUNCTION)), 100);
					_instance->smConnection->sendTo(LogicalAddress(logicalAddr), MessageEncoder().encode(UserControlReleased()), 100);
				}
			}
		}

		void HdmiCecSink::setStreamPath( const PhysicalAddress &physical_addr) {

			if(!HdmiCecSink::_instance)
				return;

                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(SetStreamPath(physical_addr)), 500);
		}

		void HdmiCecSink::setRoutingChange(const std::string &from, const std::string &to) {
			PhysicalAddress oldPhyAddr = {0xF,0xF,0xF,0xF};
			PhysicalAddress newPhyAddr = {0xF,0xF,0xF,0xF};
			int oldPortID = -1;
			int newPortID = -1;

			if(!HdmiCecSink::_instance)
				return;

			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			if( from.find("TV",0) != std::string::npos )
			{
				oldPhyAddr = _instance->deviceList[_instance->m_logicalAddressAllocated].m_physicalAddr;				
				_instance->m_currentActiveSource = -1;
			}
			else
			{
				oldPortID = stoi(from.substr(4,1),NULL,16);
				if ( oldPortID < _instance->m_numofHdmiInput )
				{
					oldPhyAddr = _instance->hdmiInputs[oldPortID].m_physicalAddr;
				}
				else
				{
					LOGERR("Invalid HDMI Old Port ID");
					return;
				}
			}

			if( to.find("TV",0) != std::string::npos )
			{
				newPhyAddr = _instance->deviceList[_instance->m_logicalAddressAllocated].m_physicalAddr;
				/*set active source as TV */
				_instance->m_currentActiveSource = _instance->m_logicalAddressAllocated;
			}
			else
			{
				newPortID = stoi(to.substr(4,1),NULL,16);

				if ( newPortID < _instance->m_numofHdmiInput )
				{
					newPhyAddr = _instance->hdmiInputs[newPortID].m_physicalAddr;
				}
				else
				{
					LOGERR("Invalid HDMI New Port ID");
					return;
				}
			}
			
                        if(!(_instance->smConnection))
                            return;
			_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(RoutingChange(oldPhyAddr, newPhyAddr)), 500);
		}

		void HdmiCecSink::addDevice(const int logicalAddress) {
			JsonObject params;

			if(!HdmiCecSink::_instance)
				return;
			
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}
			
			if ( !HdmiCecSink::_instance->deviceList[logicalAddress].m_isDevicePresent )
			 {
			 	HdmiCecSink::_instance->deviceList[logicalAddress].m_isDevicePresent = true;
				HdmiCecSink::_instance->deviceList[logicalAddress].m_logicalAddress = LogicalAddress(logicalAddress);
				HdmiCecSink::_instance->m_numberOfDevices++;
				HdmiCecSink::_instance->m_pollNextState = POLL_THREAD_STATE_INFO;

				if(logicalAddress == 0x5)
				{
					LOGINFO(" logicalAddress =%d , Audio device detected, Notify Device Settings", logicalAddress );
					params["status"] = string("success");
					params["audioDeviceConnected"] = string("true");
					hdmiCecAudioDeviceConnected = true;
					sendNotify(eventString[HDMICECSINK_EVENT_AUDIO_DEVICE_CONNECTED_STATUS], params)
				}

				sendNotify(eventString[HDMICECSINK_EVENT_DEVICE_ADDED], JsonObject())
			 }
		}

		void HdmiCecSink::removeDevice(const int logicalAddress) {
			JsonObject params;

			if(!HdmiCecSink::_instance)
				return;
			
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			if (_instance->deviceList[logicalAddress].m_isDevicePresent)
			{
				_instance->m_numberOfDevices--;

				for (int i=0; i < m_numofHdmiInput; i++) 
				{
					if (_instance->deviceList[logicalAddress].m_physicalAddr.getByteValue(0) == (hdmiInputs[i].m_portID + 1)) {
						hdmiInputs[i].removeChild(_instance->deviceList[logicalAddress].m_physicalAddr);
                                                hdmiInputs[i].update(LogicalAddress(LogicalAddress::UNREGISTERED));
					}
				}

                                if(logicalAddress == 0x5)
                                {
                                        LOGINFO(" logicalAddress =%d , Audio device removed, Notify Device Settings", logicalAddress );
                                        params["status"] = string("success");
                                        params["audioDeviceConnected"] = string("false");
					hdmiCecAudioDeviceConnected = false;
                                        sendNotify(eventString[HDMICECSINK_EVENT_AUDIO_DEVICE_CONNECTED_STATUS], params)
                                }

				_instance->deviceList[logicalAddress].clear();
				sendNotify(eventString[HDMICECSINK_EVENT_DEVICE_REMOVED], JsonObject());
			}
		}

		void HdmiCecSink::requestPowerStatus(const int logicalAddress) {
			int i;
			int requestType;
			
			if(!HdmiCecSink::_instance)
				return;
                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED + TEST_ADD ){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}
			_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDevicePowerStatus()), 100);
		}

		void HdmiCecSink::request(const int logicalAddress) {
			int i;
			int requestType;
			
			if(!HdmiCecSink::_instance)
				return;
                        if(!(_instance->smConnection))
                            return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED + TEST_ADD ){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			requestType = _instance->requestType(logicalAddress);
			_instance->deviceList[logicalAddress].m_isRequested = requestType;
			
			switch (requestType)
			{
				case CECDeviceParams::REQUEST_PHISICAL_ADDRESS :
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GivePhysicalAddress()), 200);
				}
					break;

				case CECDeviceParams::REQUEST_CEC_VERSION :
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GetCECVersion()), 100);
				}
					break;

				case CECDeviceParams::REQUEST_DEVICE_VENDOR_ID :
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDeviceVendorID()), 100);
				}
					break;

				case CECDeviceParams::REQUEST_OSD_NAME :	
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveOSDName()), 200);
				}
					break;

				case CECDeviceParams::REQUEST_POWER_STATUS :	
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDevicePowerStatus()), 100);
				}
					break;
				default:
				{
					_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
				}
					break;
			}

			_instance->deviceList[logicalAddress].m_requestTime = std::chrono::system_clock::now();
			LOGINFO("request type %d", _instance->deviceList[logicalAddress].m_isRequested);
		}

		int HdmiCecSink::requestStatus(const int logicalAddress) {
			std::chrono::duration<double,std::milli> elapsed;
			bool isElapsed = false;
			
			if(!HdmiCecSink::_instance)
				return -1;


			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED + TEST_ADD ) {
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return -1;
			}

			switch ( _instance->deviceList[logicalAddress].m_isRequested ) {
				case CECDeviceParams::REQUEST_PHISICAL_ADDRESS :
				{
					if( _instance->deviceList[logicalAddress].m_isPAUpdated &&
							_instance->deviceList[logicalAddress].m_isDeviceTypeUpdated )
					{
						_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
					}
				}
					break;

				case CECDeviceParams::REQUEST_CEC_VERSION :
				{
					if( _instance->deviceList[logicalAddress].m_isVersionUpdated )
					{
						_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
					}
				}
					break;

				case CECDeviceParams::REQUEST_DEVICE_VENDOR_ID :
				{
					if( _instance->deviceList[logicalAddress].m_isVendorIDUpdated )
					{
						_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
					}
				}
					break;

				case CECDeviceParams::REQUEST_OSD_NAME :	
				{
					if( _instance->deviceList[logicalAddress].m_isOSDNameUpdated )
					{
						_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
					}
				}
					break;

				case CECDeviceParams::REQUEST_POWER_STATUS :	
				{
					if( _instance->deviceList[logicalAddress].m_isPowerStatusUpdated )
					{
						_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;
					}
				}
					break;
				default:
					break;	
			}

			if ( _instance->deviceList[logicalAddress].m_isRequested != CECDeviceParams::REQUEST_NONE )
			{
				elapsed = std::chrono::system_clock::now() - _instance->deviceList[logicalAddress].m_requestTime;

				if ( elapsed.count() > HDMICECSINK_REQUEST_MAX_WAIT_TIME_MS )
				{
					LOGINFO("request elapsed ");
					isElapsed = true;	
				}
			}

			if (isElapsed)
			{
				/* For some request it should be retry, like report physical address etc for other we can have default values */
				switch( _instance->deviceList[logicalAddress].m_isRequested )
				{
					case CECDeviceParams::REQUEST_PHISICAL_ADDRESS :
					{
						LOGINFO("Retry for REQUEST_PHISICAL_ADDRESS = %d", _instance->deviceList[logicalAddress].m_isRequestRetry);
						/* Update with Invalid Physical Address */
						if ( _instance->deviceList[logicalAddress].m_isRequestRetry++ >= HDMICECSINK_REQUEST_MAX_RETRY )
						{
							LOGINFO("Max retry for REQUEST_PHISICAL_ADDRESS = %d", _instance->deviceList[logicalAddress].m_isRequestRetry);
							_instance->deviceList[logicalAddress].update(PhysicalAddress(0xF,0xF,0xF,0xF));
							_instance->deviceList[logicalAddress].update(DeviceType(DeviceType::RESERVED));
							_instance->deviceList[logicalAddress].m_isRequestRetry = 0;
						}
					}
						break;
					
					case CECDeviceParams::REQUEST_CEC_VERSION :
					{
						/*Defaulting to 1.4*/
						_instance->deviceList[logicalAddress].update(Version(Version::V_1_4));
					}
						break;

					case CECDeviceParams::REQUEST_DEVICE_VENDOR_ID :
					{
						_instance->deviceList[logicalAddress].update(VendorID(0,0,0));
					}
						break;

					case CECDeviceParams::REQUEST_OSD_NAME :	
					{
						_instance->deviceList[logicalAddress].update(OSDName("NA"));
					}
						break;

					case CECDeviceParams::REQUEST_POWER_STATUS :	
					{
						_instance->deviceList[logicalAddress].update(PowerStatus(PowerStatus::POWER_STATUS_NOT_KNOWN));
					}
						break;
					default:
						break;	
				}


				_instance->deviceList[logicalAddress].m_isRequested = CECDeviceParams::REQUEST_NONE;	
			}
			
			if( _instance->deviceList[logicalAddress].m_isRequested == CECDeviceParams::REQUEST_NONE)
			{
				LOGINFO("Request Done");
				return CECDeviceParams::REQUEST_DONE;
			}

			//LOGINFO("Request NOT Done");
			return CECDeviceParams::REQUEST_NOT_DONE;
		}
		
		void HdmiCecSink::threadRun()
        {
        	int i;
			std::vector <int> connected;
			std::vector <int> disconnected;
			int logicalAddressRequested = LogicalAddress::UNREGISTERED + TEST_ADD;
			bool isExit = false;

			if(!HdmiCecSink::_instance)
                return;

                if(!(_instance->smConnection))
                    return;
               LOGINFO("Entering ThreadRun: _instance->m_pollThreadExit %d isExit %d _instance->m_pollThreadState %d  _instance->m_pollNextState %d",_instance->m_pollThreadExit,isExit,_instance->m_pollThreadState,_instance->m_pollNextState );
			_instance->m_sleepTime = HDMICECSINK_PING_INTERVAL_MS;

        	while(1)
        	{

			       if (_instance->m_pollThreadExit || isExit ){
					LOGWARN("Thread Exits _instance->m_pollThreadExit %d isExit %d _instance->m_pollThreadState %d  _instance->m_pollNextState %d",_instance->m_pollThreadExit,isExit,_instance->m_pollThreadState,_instance->m_pollNextState );
					break;
				}

				if ( _instance->m_pollNextState != POLL_THREAD_STATE_NONE )
				{
					_instance->m_pollThreadState = _instance->m_pollNextState;
					_instance->m_pollNextState = POLL_THREAD_STATE_NONE;
				}
				
				switch (_instance->m_pollThreadState)  {

				case POLL_THREAD_STATE_POLL :
				{
					//LOGINFO("POLL_THREAD_STATE_POLL");
					_instance->allocateLogicalAddress(DeviceType::TV);
					if ( _instance->m_logicalAddressAllocated != LogicalAddress::UNREGISTERED)
					{
						logicalAddress = LogicalAddress(_instance->m_logicalAddressAllocated);
						LibCCEC::getInstance().addLogicalAddress(logicalAddress);
						_instance->smConnection->setSource(logicalAddress);
						_instance->m_numberOfDevices = 0;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_deviceType = DeviceType::TV;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_isDevicePresent = true;
                                                _instance->deviceList[_instance->m_logicalAddressAllocated].update(physical_addr);
                                                _instance->m_currentActiveSource = _instance->m_logicalAddressAllocated;
                                                _instance->deviceList[_instance->m_logicalAddressAllocated].m_isActiveSource = true;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_cecVersion = Version::V_1_4;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_vendorID = appVendorId;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_powerStatus = PowerStatus(powerState);
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_currentLanguage = defaultLanguage;
						_instance->smConnection->addFrameListener(_instance->msgFrameListener);
						_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), 
								MessageEncoder().encode(ReportPhysicalAddress(physical_addr, _instance->deviceList[_instance->m_logicalAddressAllocated].m_deviceType)), 100);

						_instance->m_sleepTime = 0;
						_instance->m_pollThreadState = POLL_THREAD_STATE_PING;
					}
					else
					{
						LOGINFO("Not able allocate Logical Address for TV");	
						_instance->m_pollThreadState = POLL_THREAD_STATE_EXIT;
					}
				}
				break;
				
				case POLL_THREAD_STATE_PING :
				{
					//LOGINFO("POLL_THREAD_STATE_PING");
					_instance->m_pollThreadState = POLL_THREAD_STATE_INFO;
					connected.clear();
					disconnected.clear();
					_instance->pingDevices(connected, disconnected);

					if ( disconnected.size() ){
						for( i=0; i< disconnected.size(); i++ )
						{
							LOGWARN("Disconnected Devices [%d]", disconnected.size());
							_instance->removeDevice(disconnected[i]);
						}
					}

					if (connected.size()) {
						LOGWARN("Connected Devices [%d]", connected.size());
						for( i=0; i< connected.size(); i++ )
						{
							_instance->addDevice(connected[i]);
							/* If new device is connected, then try to aquire the information */
							_instance->m_pollThreadState = POLL_THREAD_STATE_INFO;
							_instance->m_sleepTime = 0;
						}
					}
					else
					{
						/* Check for any update required */
						_instance->m_pollThreadState = POLL_THREAD_STATE_UPDATE;
						_instance->m_sleepTime = 0;
					}
				}
				break;

				case POLL_THREAD_STATE_INFO :
				{
					//LOGINFO("POLL_THREAD_STATE_INFO");

					if ( logicalAddressRequested == LogicalAddress::UNREGISTERED + TEST_ADD )
					{
						for(i=0;i<LogicalAddress::UNREGISTERED + TEST_ADD;i++)
						{
							if( i != _instance->m_logicalAddressAllocated &&
								_instance->deviceList[i].m_isDevicePresent &&
								!_instance->deviceList[i].isAllUpdated() )
							{
								//LOGINFO("POLL_THREAD_STATE_INFO -> request for %d", i);
								logicalAddressRequested = i;
								_instance->request(logicalAddressRequested);
								_instance->m_sleepTime = HDMICECSINK_REQUEST_INTERVAL_TIME_MS;
								break;
							}
						}

						if ( i ==  LogicalAddress::UNREGISTERED)
						{
							/*So there is no update required, try to ping after some seconds*/
							_instance->m_pollThreadState = POLL_THREAD_STATE_IDLE;		
							_instance->m_sleepTime = 0;
							//LOGINFO("POLL_THREAD_STATE_INFO -> state change to Ping", i);
						}
					}
					else
					{
						/*So there is request sent for logical address, so wait and check the status */
						if ( _instance->requestStatus(logicalAddressRequested) == CECDeviceParams::REQUEST_DONE )
						{
							logicalAddressRequested = LogicalAddress::UNREGISTERED;
						}
						else
						{
							_instance->m_sleepTime = HDMICECSINK_REQUEST_INTERVAL_TIME_MS;							
						}
					}
				}
				break;

				/* updating the power status and if required we can add other information later*/
				case POLL_THREAD_STATE_UPDATE :
				{
					//LOGINFO("POLL_THREAD_STATE_UPDATE");

					for(i=0;i<LogicalAddress::UNREGISTERED + TEST_ADD;i++)
					{
						if( i != _instance->m_logicalAddressAllocated &&
							_instance->deviceList[i].m_isDevicePresent &&
							_instance->deviceList[i].m_isPowerStatusUpdated )
						{
							std::chrono::duration<double,std::milli> elapsed = std::chrono::system_clock::now() - _instance->deviceList[i].m_lastPowerUpdateTime;

							if ( elapsed.count() > HDMICECSINK_UPDATE_POWER_STATUS_INTERVA_MS )
							{
								_instance->deviceList[i].m_isPowerStatusUpdated = false;
								_instance->m_pollNextState = POLL_THREAD_STATE_INFO;		
								_instance->m_sleepTime = 0;
							}
						}
					}

					_instance->m_pollThreadState = POLL_THREAD_STATE_IDLE;		
					_instance->m_sleepTime = 0;
				}
				break;

				case POLL_THREAD_STATE_IDLE :
				{
					//LOGINFO("POLL_THREAD_STATE_IDLE");
					_instance->m_sleepTime = HDMICECSINK_PING_INTERVAL_MS;
					_instance->m_pollThreadState = POLL_THREAD_STATE_PING;
				}
				break;

				case POLL_THREAD_STATE_WAIT :
				{
					/* Wait for Hdmi is connected, in case it disconnected */
					//LOGINFO("19Aug2020-[01] -> POLL_THREAD_STATE_WAIT");
					_instance->m_sleepTime = HDMICECSINK_WAIT_FOR_HDMI_IN_MS;

					if ( _instance->m_isHdmiInConnected == true )
					{
						_instance->m_pollThreadState = POLL_THREAD_STATE_POLL;
					}
				}
				break;

				case POLL_THREAD_STATE_EXIT :
				{
					isExit = true;
					_instance->m_sleepTime = 0;
				}
				break;
				}

				std::unique_lock<std::mutex> lk(_instance->m_pollExitMutex);
				if ( _instance->m_ThreadExitCV.wait_for(lk, std::chrono::milliseconds(_instance->m_sleepTime)) == std::cv_status::timeout )
					continue;
				else
					LOGINFO("Thread is going to Exit m_pollThreadExit %d\n", _instance->m_pollThreadExit );

			}
        }

        void HdmiCecSink::allocateLAforTV()
        {
            bool gotLogicalAddress = false;
            int addr = LogicalAddress::TV;
            int i, j;
            if (!(_instance->smConnection))
                return;

            for (i = 0; i< HDMICECSINK_NUMBER_TV_ADDR; i++)
            {
                /* poll for TV logical address - retry 5 times*/
                for (j = 0; j < 5; j++)
                {
                    try {
                        smConnection->poll(LogicalAddress(addr), Throw_e());
                    }
                    catch(CECNoAckException &e )
                    {
                        LOGWARN("Poll caught %s \r\n",e.what());
                        gotLogicalAddress = true;
                        break;
                    }
                    catch(Exception &e)
                    {
                        LOGWARN("Poll caught %s \r\n",e.what());
                        usleep(250000);
                    }
                }
                if (gotLogicalAddress)
                {
                    break;
                }
                addr = LogicalAddress::SPECIFIC_USE;
            }

            if ( gotLogicalAddress )
            {
                m_logicalAddressAllocated = addr;
            }
            else
            {
                m_logicalAddressAllocated = LogicalAddress::UNREGISTERED;
            }

            LOGWARN("Logical Address for TV 0x%x \r\n",m_logicalAddressAllocated);
        }

        void HdmiCecSink::allocateLogicalAddress(int deviceType)
        {
        	if( deviceType == DeviceType::TV )
        	{
				allocateLAforTV();        		
        	}
        }

        void HdmiCecSink::CECEnable(void)
        {
            std::lock_guard<std::mutex> lock(m_enableMutex);
	    JsonObject params;
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

			smConnection = new Connection(LogicalAddress::UNREGISTERED,false,"ServiceManager::Connection::");
            smConnection->open();
            allocateLogicalAddress(DeviceType::TV);
            LOGINFO("logical address allocalted: %x  \n",m_logicalAddressAllocated);
            if ( m_logicalAddressAllocated != LogicalAddress::UNREGISTERED && smConnection)
            {
                logicalAddress = LogicalAddress(m_logicalAddressAllocated);
                LOGINFO(" add logical address  %x  \n",m_logicalAddressAllocated);
                LibCCEC::getInstance().addLogicalAddress(logicalAddress);
                smConnection->setSource(logicalAddress);
            }
            msgProcessor = new HdmiCecSinkProcessor(*smConnection);
            msgFrameListener = new HdmiCecSinkFrameListener(*msgProcessor);
            if(smConnection)
            {
           		LOGWARN("Start Thread %p", smConnection );
			    m_pollThreadState = POLL_THREAD_STATE_POLL;
                            m_pollNextState = POLL_THREAD_STATE_NONE;
                            m_pollThreadExit = false;
				m_pollThread = std::thread(threadRun);
            }
            cecEnableStatus = true;

	    params["cecEnable"] = string("true");
            sendNotify(eventString[HDMICECSINK_EVENT_CEC_ENABLED], params);
 
            return;
        }

        void HdmiCecSink::CECDisable(void)
        {
            std::lock_guard<std::mutex> lock(m_enableMutex);
	    JsonObject params;
            LOGINFO("Entered CECDisable ");
            if(!cecEnableStatus)
            {
                LOGWARN("CEC Already Disabled ");
                return;
            }
            
            if(m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED)
            {
                stopArc();
	      while(m_currentArcRoutingState != ARC_STATE_ARC_TERMINATED)	
              {
                     usleep(500000);
               }
            }

             LOGINFO(" CECDisable ARC stopped ");
           cecEnableStatus = false;
            if (smConnection != NULL)
            {
		LOGWARN("Stop Thread %p", smConnection );
		m_pollThreadExit = true;
		m_ThreadExitCV.notify_one();

		try
		{
			if (m_pollThread.joinable())
			{
				LOGWARN("Join Thread %p", smConnection );
				m_pollThread.join();
			}
		}
		catch(const std::system_error& e)
		{
			LOGERR("system_error exception in thread join %s", e.what());
		}
		catch(const std::exception& e)
		{
			LOGERR("exception in thread join %s", e.what());
		}

                m_pollThreadState = POLL_THREAD_STATE_NONE;
                m_pollNextState = POLL_THREAD_STATE_NONE;

		LOGWARN("Deleted Thread %p", smConnection );

                smConnection->close();
                delete smConnection;
                smConnection = NULL;
            }
            
	    m_logicalAddressAllocated = LogicalAddress::UNREGISTERED;
            m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;

	    for(int i=0; i< 16; i++)
            {
		 if (_instance->deviceList[i].m_isDevicePresent)
	         {
	 		_instance->deviceList[i].clear();
	         }
            }

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
            LOGWARN("CEC Disabled %d",libcecInitStatus); 

	   params["cecEnable"] = string("false");
           sendNotify(eventString[HDMICECSINK_EVENT_CEC_ENABLED], params);

            return;
        }


        void HdmiCecSink::getPhysicalAddress()
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

        bool HdmiCecSink::getEnabled()
        {

            
            LOGINFO("getEnabled :%d ",cecEnableStatus);
            if(true == cecEnableStatus)
                return true;
            else
                return false;
        }

        bool HdmiCecSink::getAudioDeviceConnectedStatus()
        {
            LOGINFO("getAudioDeviceConnectedStatus :%d ", hdmiCecAudioDeviceConnected);
            if(true == hdmiCecAudioDeviceConnected)
                return true;
            else
                return false;
        }
        //Arc Routing related  functions
        void HdmiCecSink::startArc()
        {
           if ( cecEnableStatus != true  )
           {
              LOGINFO("Initiate_Arc Cec is disabled-> EnableCEC first");
	      return;
           }
           if(!HdmiCecSink::_instance)
            return;

             LOGINFO("Current ARC State : %d\n", m_currentArcRoutingState);

	    _instance->requestArcInitiation();
 
          // start initiate ARC timer 3 sec
            if (m_arcStartStopTimer.isActive())
            {
                m_arcStartStopTimer.stop();
            }
            m_arcstarting = true;
            m_arcStartStopTimer.start((HDMISINK_ARC_START_STOP_MAX_WAIT_MS)); 

        }
        void  HdmiCecSink::requestArcInitiation()
        {
	  {
           std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
           m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_INITIATION;
	  }
          LOGINFO("requestArcInitiation release sem");
          _instance->m_semSignaltoArcRoutingThread.release();

        }
        void HdmiCecSink::stopArc()
        {
            if ( cecEnableStatus != true  )
            {
              LOGINFO("Initiate_Arc Cec is disabled-> EnableCEC first");
	      return;
            }
            if(!HdmiCecSink::_instance)
                return;
	    if(m_currentArcRoutingState == ARC_STATE_REQUEST_ARC_TERMINATION || m_currentArcRoutingState == ARC_STATE_ARC_TERMINATED)
            {
               LOGINFO("ARC is either Termination  in progress or already Terminated");
               return;
            }
		
           _instance->requestArcTermination();
           /* start a timer for 3 sec to get the desired ARC_STATE_ARC_TERMINATED */
           if (m_arcStartStopTimer.isActive())
            {
                m_arcStartStopTimer.stop();
            }
            /* m_arcstarting = true means starting the ARC start timer ,false means ARC stopping timer*/
            m_arcstarting = false; 
            m_arcStartStopTimer.start((HDMISINK_ARC_START_STOP_MAX_WAIT_MS));

  				
        }
        void HdmiCecSink::requestArcTermination()
        {  
	  {
           std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
           m_currentArcRoutingState = ARC_STATE_REQUEST_ARC_TERMINATION;
	  }
           LOGINFO("requestArcTermination release sem");
           _instance->m_semSignaltoArcRoutingThread.release();

       }  	
       
	void  HdmiCecSink::Process_InitiateArc()
        {
            JsonObject params;
             
            LOGINFO("Command: INITIATE_ARC \n");
              
            if(!HdmiCecSink::_instance)
	    return;

	    //DD: Check cecSettingEnabled to prevent race conditions which gives immediate UI setting status
	    //Initiate ARC message may come from AVR/Soundbar while CEC disable is in-progress
            if ( cecSettingEnabled != true  )
            {
              LOGINFO("Process InitiateArc from Audio device: Cec is disabled-> EnableCEC first");
              return;
            }

            LOGINFO("Got : INITIATE_ARC  and current Arcstate is %d\n",_instance->m_currentArcRoutingState);

            if (m_arcStartStopTimer.isActive())
            {
               m_arcStartStopTimer.stop();
            }
		{
            	  std::lock_guard<std::mutex> lock(_instance->m_arcRoutingStateMutex);
	          _instance->m_currentArcRoutingState = ARC_STATE_ARC_INITIATED;
		}
                  _instance->m_semSignaltoArcRoutingThread.release();
                  LOGINFO("Got : ARC_INITIATED  and notify Device setting");
                  params["status"] = string("success");
                  sendNotify(eventString[HDMICECSINK_EVENT_ARC_INITIATION_EVENT], params); 
	  

       }
       void HdmiCecSink::Process_TerminateArc()
       {
            JsonObject params;

            LOGINFO("Command: TERMINATE_ARC current arc state %d \n",HdmiCecSink::_instance->m_currentArcRoutingState);
                if (m_arcStartStopTimer.isActive())
                {
                      m_arcStartStopTimer.stop();
                }
		{
            		std::lock_guard<std::mutex> lock(m_arcRoutingStateMutex);
                	HdmiCecSink::_instance->m_currentArcRoutingState = ARC_STATE_ARC_TERMINATED;
		}
                _instance->m_semSignaltoArcRoutingThread.release();
            	  
                // trigger callback to Device setting informing to TERMINATE_ARC
                LOGINFO("Got : ARC_TERMINATED  and notify Device setting");
                params["status"] = string("success");
                sendNotify(eventString[HDMICECSINK_EVENT_ARC_TERMINATION_EVENT], params);
        }

        void HdmiCecSink::threadSendKeyEvent()
        {
            int i;

            if(!HdmiCecSink::_instance)
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

                LOGINFO("sendRemoteKeyThread : logical addr:0x%x keyCode: 0x%x  queue size :%d \n",keyInfo.logicalAddr,keyInfo.keyCode,_instance->m_SendKeyQueue.size());
			    _instance->sendKeyPressEvent(keyInfo.logicalAddr,keyInfo.keyCode);
			    _instance->sendKeyReleaseEvent(keyInfo.logicalAddr);
			    if((_instance->m_SendKeyQueue.size()<=1 || (_instance->m_SendKeyQueue.size() % 2 == 0)) && ((keyInfo.keyCode == VOLUME_UP) || (keyInfo.keyCode == VOLUME_DOWN) || (keyInfo.keyCode == MUTE)) )
			    {
			        _instance->sendGiveAudioStatusMsg();
			    }

            }//while(!_instance->m_sendKeyEventThreadExit)
        }//threadSendKeyEvent


        void HdmiCecSink::threadArcRouting()
        {
        	int i;
		bool isExit = false;
		uint32_t currentArcRoutingState;

		if(!HdmiCecSink::_instance)
                return;

		LOGINFO("Running threadArcRouting");


        	while(1)
        	{
			
		    _instance->m_semSignaltoArcRoutingThread.acquire();
			   
			   
		   
		{ 
                   LOGINFO(" threadArcRouting Got semaphore"); 
 		   std::lock_guard<std::mutex> lock(_instance->m_arcRoutingStateMutex);
			   
		   currentArcRoutingState = _instance->m_currentArcRoutingState;
	   
		   LOGINFO(" threadArcRouting  Got Sem arc state %d",currentArcRoutingState);
		}	   
			   
		  switch (currentArcRoutingState) 
		  {   

			     case ARC_STATE_REQUEST_ARC_INITIATION :
                             { 
				 
                                 _instance->systemAudioModeRequest();
				 _instance->Send_Request_Arc_Initiation_Message();
				   
			     }
			          break;
			    case ARC_STATE_ARC_INITIATED :
			    {
			       _instance->Send_Report_Arc_Initiated_Message();
		     	    }
				 break;
			    case ARC_STATE_REQUEST_ARC_TERMINATION :
			    {
				    
			       _instance->Send_Request_Arc_Termination_Message();
				 
			    }
			       break;
			    case ARC_STATE_ARC_TERMINATED :
			    {
				  _instance->Send_Report_Arc_Terminated_Message();
			    }
			       break;
			    case ARC_STATE_ARC_EXIT :
			    {
				isExit = true;
			    }
			    break;
	         }
			 
	         if (isExit == true)
	         {  
		     LOGINFO(" threadArcRouting EXITing"); 
	             break;
	          }
            }//while(1)
        }//threadArcRouting
  
        void HdmiCecSink::Send_Request_Arc_Initiation_Message()
	{
           if(!HdmiCecSink::_instance)
	     return;
           if(!(_instance->smConnection))
               return;
          LOGINFO(" Send_Request_Arc_Initiation_Message ");
           _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(RequestArcInitiation()), 1000);

        }
        void HdmiCecSink::Send_Report_Arc_Initiated_Message()
        {   
            if(!HdmiCecSink::_instance)
	    return;
            if(!(_instance->smConnection))
               return;
            _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(ReportArcInitiation()), 1000);

        }
        void HdmiCecSink::Send_Request_Arc_Termination_Message()
        {

            if(!HdmiCecSink::_instance)
	     return;
            if(!(_instance->smConnection))
               return;
            _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(RequestArcTermination()), 1000);
        }

       void HdmiCecSink::Send_Report_Arc_Terminated_Message()
       {
            if(!HdmiCecSink::_instance)
		return;
            if(!(_instance->smConnection))
               return;
           _instance->smConnection->sendTo(LogicalAddress::AUDIO_SYSTEM,MessageEncoder().encode(ReportArcTermination()), 1000);

       }
       
      void HdmiCecSink::getHdmiArcPortID()
      {
         int err;
         dsGetHDMIARCPortIdParam_t param;
         err = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            (char *)IARM_BUS_DSMGR_API_dsGetHDMIARCPortId,
                            (void *)&param,
                            sizeof(param));
          if (IARM_RESULT_SUCCESS == err)
          {
             LOGINFO("HDMI ARC port ID HdmiArcPortID=[%d] \n", param.portId);
             HdmiArcPortID = param.portId;
          }
      }

    } // namespace Plugin
} // namespace WPEFrameworklk
