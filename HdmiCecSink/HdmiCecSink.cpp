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

#include "utils.h"

#define HDMICECSINK_METHOD_SET_ENABLED "setEnabled"
#define HDMICECSINK_METHOD_GET_ENABLED "getEnabled"
#define HDMICECSINK_METHOD_OTP_SET_ENABLED "setOTPEnabled"
#define HDMICECSINK_METHOD_OTP_GET_ENABLED "getOTPEnabled"
#define HDMICECSINK_METHOD_SET_OSD_NAME "setOSDName"
#define HDMICECSINK_METHOD_GET_OSD_NAME "getOSDName"
#define HDMICECSINK_METHOD_SET_VENDOR_ID "setVendorId"
#define HDMICECSINK_METHOD_GET_VENDOR_ID "getVendorId"
#define HDMICECSINK_METHOD_PRINT_DEVICE_LIST "printDeviceList"
#define HDMICECSINK_METHOD_SET_ACTIVE_PATH "setActivePath"
#define HDMICECSINK_METHOD_GET_DEVICE_LIST "getDeviceList"
#define HDMICECSINK_METHOD_GET_ACTIVE_SOURCE "getActiveSource"



#define TEST_ADD 0
#define HDMICECSINK_REQUEST_MAX_RETRY 				3
#define HDMICECSINK_REQUEST_MAX_WAIT_TIME_MS 		2000
#define HDMICECSINK_PING_INTERVAL_MS 				5000
#define HDMICECSINK_WAIT_FOR_HDMI_IN_MS 			1000
#define HDMICECSINK_REQUEST_INTERVAL_TIME_MS 		200
#define HDMICECSINK_NUMBER_TV_ADDR 					2
#define HDMICECSINK_UPDATE_POWER_STATUS_INTERVA_MS    (60 * 1000)




enum {
	HDMICECSINK_EVENT_ACTIVE_SOURCE_CHANGE = 1,
	HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY,
	HDMICECSINK_EVENT_TEXT_VIEW_ON_MSG,
	HDMICECSINK_EVENT_IMAGE_VIEW_ON_MSG,
	HDMICECSINK_EVENT_DEVICE_ADDED,
	HDMICECSINK_EVENT_DEVICE_REMOVED,
	HDMICECSINK_EVENT_DEVICE_INFO_UPDATED,
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
static OSDName osdName = "TV Box";
static int32_t powerState = 1;


namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiCecSink, 1, 0);

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
       }
       void HdmiCecSinkProcessor::process (const RequestActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: RequestActiveSource\n");
       }
       void HdmiCecSinkProcessor::process (const Standby &msg, const Header &header)
       {
             printHeader(header);
			 LOGINFO("Command: Standby from %s\n", header.from.toString().c_str());
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
             printHeader(header);
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());

			HdmiCecSink::_instance->addDevice(header.from.toInt());
			HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.version);
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
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());

			 HdmiCecSink::_instance->addDevice(header.from.toInt());
			 HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.osdName);
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
       }
       void HdmiCecSinkProcessor::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ReportPhysicalAddress\n");

			 if(!HdmiCecSink::_instance)
			 	return;

			 HdmiCecSink::_instance->addDevice(header.from.toInt()); 	
			 HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.physicalAddress);
			 HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.deviceType);
       }
       void HdmiCecSinkProcessor::process (const DeviceVendorID &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());

			 HdmiCecSink::_instance->addDevice(header.from.toInt());
			 HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
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
			printHeader(header);
			LOGINFO("Command: ReportPowerStatus Power Status from:%s status : %s \n",header.from.toString().c_str(),msg.status.toString().c_str());

			HdmiCecSink::_instance->addDevice(header.from.toInt());
			HdmiCecSink::_instance->deviceList[header.from.toInt()].update(msg.status);
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


			
       }
       void HdmiCecSinkProcessor::process (const Abort &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: Abort\n");
       }
       void HdmiCecSinkProcessor::process (const Polling &msg, const Header &header)                                 {
             printHeader(header);
             LOGINFO("Command: Polling\n");
       }


//=========================================== HdmiCecSink =========================================

       HdmiCecSink::HdmiCecSink()
       : AbstractPlugin()
       {
       	   int err;
           LOGWARN("Initlaizing HdmiCecSink");
           HdmiCecSink::_instance = this;
		   cecEnableStatus = false;
		   m_currentActiveSource = -1;
		   m_isHdmiInConnected = false;
		   m_pollNextState = POLL_THREAD_STATE_NONE;
		   m_pollThreadState = POLL_THREAD_STATE_NONE;

           InitializeIARM();

           registerMethod(HDMICECSINK_METHOD_SET_ENABLED, &HdmiCecSink::setEnabledWrapper, this);
           registerMethod(HDMICECSINK_METHOD_GET_ENABLED, &HdmiCecSink::getEnabledWrapper, this);
           registerMethod(HDMICECSINK_METHOD_SET_OSD_NAME, &HdmiCecSink::setOSDNameWrapper, this);
           registerMethod(HDMICECSINK_METHOD_GET_OSD_NAME, &HdmiCecSink::getOSDNameWrapper, this);
           registerMethod(HDMICECSINK_METHOD_SET_VENDOR_ID, &HdmiCecSink::setVendorIdWrapper, this);
           registerMethod(HDMICECSINK_METHOD_GET_VENDOR_ID, &HdmiCecSink::getVendorIdWrapper, this);
		   registerMethod(HDMICECSINK_METHOD_PRINT_DEVICE_LIST, &HdmiCecSink::printDeviceListWrapper, this);
		   registerMethod(HDMICECSINK_METHOD_SET_ACTIVE_PATH, &HdmiCecSink::setActivePathWrapper, this);
		   registerMethod(HDMICECSINK_METHOD_GET_DEVICE_LIST, &HdmiCecSink::getDeviceListWrapper, this);
		   registerMethod(HDMICECSINK_METHOD_GET_ACTIVE_SOURCE, &HdmiCecSink::getActiveSourceWrapper, this);

           logicalAddressDeviceType = "None";
           logicalAddress = 0xFF;

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
                powerState = (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)?0:1 ;
                LOGINFO("Current state is IARM: (%d) powerState :%d \n",param.curState,powerState);
            }

			CheckHdmiInState();
			    
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

       HdmiCecSink::~HdmiCecSink()
       {
           LOGINFO();
           HdmiCecSink::_instance = nullptr;
           DeinitializeIARM();
       }

       const void HdmiCecSink::InitializeIARM()
       {
            LOGINFO();

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
            LOGINFO();

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
            LOGINFO();

            if(!HdmiCecSink::_instance)
                return;

            if( !strcmp(owner, IARM_BUS_CECMGR_NAME))
            {
                switch (eventId)
                {
                    case IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED:
                    {
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
            LOGINFO();

            if(!HdmiCecSink::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                bool isHdmiConnected = eventData->data.hdmi_in_connect.isPortConnected;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_IN_HOTPLUG event data:%d \r\n", isHdmiConnected);
                HdmiCecSink::_instance->onHdmiHotPlug(isHdmiConnected);
            }
       }

       void HdmiCecSink::pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
       {
            LOGINFO();

            if(!HdmiCecSink::_instance)
                return;

            if (strcmp(owner, IARM_BUS_PWRMGR_NAME)  == 0) {
                if (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED ) {
                    IARM_Bus_PWRMgr_EventData_t *param = (IARM_Bus_PWRMgr_EventData_t *)data;
                    LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                            param->data.state.curState, param->data.state.newState);
                    if(param->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                    {
                        powerState = 0; 
						HdmiCecSink::_instance->onPowerStateON();
                    }
                    else
                        powerState = 1;

                }
           }
       }

	  void HdmiCecSink::onPowerStateON()
       {
            LOGINFO();

       		if ( powerState == 0 )
       		{
       			/*while wakeup From Standby, Ask for Active Source*/
				m_currentActiveSource = -1;
       			_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST),MessageEncoder().encode(RequestActiveSource()), 5000); 
       		}
       }

       void HdmiCecSink::onCECDaemonInit()
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

       void HdmiCecSink::cecStatusUpdated(void *evtStatus)
       {
            LOGINFO();

            IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = (IARM_Bus_CECMgr_Status_Updated_Param_t *)evtStatus;
            if(evtData)
            {
                delete evtData;
            }
           return;
       }

       void HdmiCecSink::onHdmiHotPlug(int connectStatus)
       {
        	bool previousHdmiState = m_isHdmiInConnected;
			int i = 0;
            LOGINFO();
			LOGINFO("onHdmiHotPlug Status : %d ", connectStatus);

			CheckHdmiInState();

			if ( previousHdmiState != m_isHdmiInConnected )
			{
				if ( m_isHdmiInConnected == false )
				{
					m_pollNextState = POLL_THREAD_STATE_PING;
				}
			}
            return;
       }

       uint32_t HdmiCecSink::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

       uint32_t HdmiCecSink::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
       {
            response["enabled"] = getEnabled();
            returnResponse(true);
       }

	  uint32_t HdmiCecSink::getActiveSourceWrapper(const JsonObject& parameters, JsonObject& response)
       {
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
			}
			else
			{
				response["available"] = false;
			}
			
            returnResponse(true);
       }

       uint32_t HdmiCecSink::getDeviceListWrapper(const JsonObject& parameters, JsonObject& response)
       {
            LOGINFO();

			response["numberofdevices"] = HdmiCecSink::_instance->m_numberOfDevices;
			
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
			
					deviceList.Add(device);
				}
			}

			response["deviceList"] = deviceList;

            returnResponse(true);
       }


       uint32_t HdmiCecSink::setOSDNameWrapper(const JsonObject& parameters, JsonObject& response)
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

		uint32_t HdmiCecSink::setActivePathWrapper(const JsonObject& parameters, JsonObject& response)
        {
        	setActivePath();
            response["setActivePath"] = true;
            returnResponse(true);
        }

        uint32_t HdmiCecSink::setVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t HdmiCecSink::getVendorIdWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO("getVendorIdWrapper  appVendorId : %s  \n",appVendorId.toString().c_str());
            response["vendorid"] = appVendorId.toString() ;
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

        void HdmiCecSink::persistSettings(bool enableStatus)
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

        void HdmiCecSink::persistOSDName(const char *name)
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

        void HdmiCecSink::persistVendorId(unsigned int vendorId)
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

        void HdmiCecSink::setEnabled(bool enabled)
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
				sendNotify(eventString[HDMICECSINK_EVENT_WAKEUP_FROM_STANDBY], params);
			}

			sendNotify(eventString[HDMICECSINK_EVENT_TEXT_VIEW_ON_MSG], params);
		}

		void HdmiCecSink::CheckHdmiInState()
		{
			int err;
			
			dsHdmiInGetStatusParam_t params;
            err = IARM_Bus_Call(IARM_BUS_DSMGR_NAME,
                            IARM_BUS_DSMGR_API_dsHdmiInGetStatus,
                            (void *)&params,
                            sizeof(params));
			
            if(err == IARM_RESULT_SUCCESS)
            {
                LOGINFO("Is HDMI In Ports are connected [%d] [%d] [%d] \n",
							params.status.isPortConnected[0], params.status.isPortConnected[1], params.status.isPortConnected[2]);

				if ( params.status.isPortConnected[0] ||
						params.status.isPortConnected[1] ||
							params.status.isPortConnected[2] )
				{
					m_isHdmiInConnected = true;
				}
				else
				{
					m_isHdmiInConnected = false;
				}
            }
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

			if( logical_address != _instance->m_logicalAddressAllocated &&
				_instance->deviceList[logical_address].m_isActiveSource &&
				_instance->m_currentActiveSource == logical_address )
			{
				_instance->deviceList[logical_address].m_isActiveSource = false;
				_instance->m_currentActiveSource = -1;
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
				if ( _instance->m_currentActiveSource !=  _instance->m_logicalAddressAllocated &&
						_instance->m_currentActiveSource != LogicalAddress::UNREGISTERED )
				{
					_instance->deviceList[_instance->m_currentActiveSource].m_isActiveSource = false;
				}

				_instance->deviceList[logical_address].m_isActiveSource = true;
				_instance->deviceList[logical_address].update(source.physicalAddress);
				_instance->m_currentActiveSource = logical_address;
				params["logicalAddress"] = JsonValue(logical_address);
				params["phsicalAddress"] = _instance->deviceList[logical_address].m_physicalAddr.toString().c_str(); 
				sendNotify(eventString[HDMICECSINK_EVENT_ACTIVE_SOURCE_CHANGE], params);
			}
       	}

		void HdmiCecSink::pingDevices(std::vector<int> &connected , std::vector<int> &disconnected)
        {
        	int i;

			if(!HdmiCecSink::_instance)
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
						//LOGWARN("Ping caught %s \r\n",e.what());
						usleep(50000);
						continue;
					}
					  catch(Exception &e)
					  {
						LOGINFO("Ping caught %s \r\n",e.what());
					  }
					  
					  LOGINFO("PING got Device ACK 0x%x \r\n",i);
					  /* If we get ACK, then the device is present in the network*/
					  if ( !_instance->deviceList[i].m_isDevicePresent )
					  {
					  	connected.push_back(i);
					  }
					  usleep(50000);      
				}
           	}
        }

		int HdmiCecSink::requestType( const int logicalAddress ) {
			int requestType = CECDeviceParams::REQUEST_NONE;
			
			if ( !_instance->deviceList[logicalAddress].m_isPAUpdated || !_instance->deviceList[logicalAddress].m_isDeviceTypeUpdated ) {
				requestType = CECDeviceParams::REQUEST_PHISICAL_ADDRESS; 	
			}else if ( !_instance->deviceList[logicalAddress].m_isVersionUpdated ) {
				requestType = CECDeviceParams::REQUEST_CEC_VERSION;
			}else if ( !_instance->deviceList[logicalAddress].m_isVendorIDUpdated ) {
				requestType = CECDeviceParams::REQUEST_DEVICE_VENDOR_ID;
			}else if ( !_instance->deviceList[logicalAddress].m_isOSDNameUpdated ) {
				requestType = CECDeviceParams::REQUEST_OSD_NAME;
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
				LOGWARN("-----19Aug2020-[01] - Device ID = %d--------", i);
				HdmiCecSink::_instance->deviceList[i].printVariable();
				LOGWARN("-----------------------------");
				}
			}
		}

		void HdmiCecSink::setActivePath() {
			int i;

			if(!HdmiCecSink::_instance)
				return;

			_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(SetStreamPath(PhysicalAddress(1,0,0,0))), 5000);	
		}

		void HdmiCecSink::addDevice(const int logicalAddress) {

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
				sendNotify(eventString[HDMICECSINK_EVENT_DEVICE_ADDED], JsonObject())
			 }
		}

		void HdmiCecSink::removeDevice(const int logicalAddress) {
			if(!HdmiCecSink::_instance)
				return;
			
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED ){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}

			if (_instance->deviceList[logicalAddress].m_isDevicePresent)
			{
				_instance->m_numberOfDevices--;
				_instance->deviceList[logicalAddress].clear();
				sendNotify(eventString[HDMICECSINK_EVENT_DEVICE_REMOVED], JsonObject());
			}
		}

		void HdmiCecSink::requestPowerStatus(const int logicalAddress) {
			int i;
			int requestType;
			
			if(!HdmiCecSink::_instance)
				return;
			if ( _instance->m_logicalAddressAllocated == LogicalAddress::UNREGISTERED || logicalAddress >= LogicalAddress::UNREGISTERED + TEST_ADD ){
				LOGERR("Logical Address NOT Allocated Or its not valid");
				return;
			}
			_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDevicePowerStatus()), 5000);	
		}

		void HdmiCecSink::request(const int logicalAddress) {
			int i;
			int requestType;
			
			if(!HdmiCecSink::_instance)
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
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GivePhysicalAddress()), 5000);	
				}
					break;

				case CECDeviceParams::REQUEST_CEC_VERSION :
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GetCECVersion()), 5000);	
				}
					break;

				case CECDeviceParams::REQUEST_DEVICE_VENDOR_ID :
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDeviceVendorID()), 5000);	
				}
					break;

				case CECDeviceParams::REQUEST_OSD_NAME :	
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveOSDName()), 5000);	
				}
					break;

				case CECDeviceParams::REQUEST_POWER_STATUS :	
				{
					_instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(GiveDevicePowerStatus()), 5000);	
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

			LOGINFO("Entering ThreadRun = %d", _instance->m_pollThreadState);

			_instance->m_sleepTime = HDMICECSINK_PING_INTERVAL_MS;

        	while(1)
        	{

				if ( isExit ){
					LOGINFO("Thread Exits");
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
					LOGINFO("POLL_THREAD_STATE_POLL");
					_instance->allocateLogicalAddress(DeviceType::TV);
					if ( _instance->m_logicalAddressAllocated != LogicalAddress::UNREGISTERED)
					{
						logicalAddress = LogicalAddress(_instance->m_logicalAddressAllocated);
						LibCCEC::getInstance().addLogicalAddress(logicalAddress);
						_instance->smConnection->setSource(logicalAddress);
						_instance->m_numberOfDevices = 0;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_deviceType = DeviceType::TV;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_isDevicePresent = true;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_cecVersion = Version::V_1_4;
						_instance->deviceList[_instance->m_logicalAddressAllocated].m_vendorID = appVendorId;
						_instance->smConnection->addFrameListener(_instance->msgFrameListener);
						_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), 
								MessageEncoder().encode(ReportPhysicalAddress(physical_addr, _instance->deviceList[_instance->m_logicalAddressAllocated].m_deviceType)), 5000);	

						 if ( powerState == 0 )
						 {
							_instance->smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST),MessageEncoder().encode(RequestActiveSource()), 5000); 
						 }

						_instance->m_sleepTime = HDMICECSINK_PING_INTERVAL_MS;
						_instance->m_pollThreadState = POLL_THREAD_STATE_IDLE;
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
					LOGINFO("POLL_THREAD_STATE_PING");
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
					LOGINFO("POLL_THREAD_STATE_INFO");

					if ( logicalAddressRequested == LogicalAddress::UNREGISTERED + TEST_ADD )
					{
						for(i=0;i<LogicalAddress::UNREGISTERED + TEST_ADD;i++)
						{
							if( i != _instance->m_logicalAddressAllocated &&
								_instance->deviceList[i].m_isDevicePresent &&
								!_instance->deviceList[i].isAllUpdated() )
							{
								LOGINFO("POLL_THREAD_STATE_INFO -> request for %d", i);
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
					LOGINFO("POLL_THREAD_STATE_UPDATE");

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
					LOGINFO("POLL_THREAD_STATE_IDLE");
					_instance->m_sleepTime = HDMICECSINK_PING_INTERVAL_MS;
					_instance->m_pollThreadState = POLL_THREAD_STATE_PING;
				}
				break;

				case POLL_THREAD_STATE_WAIT :
				{
					/* Wait for Hdmi is connected, in case it disconnected */
					LOGINFO("19Aug2020-[01] -> POLL_THREAD_STATE_WAIT");
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

				if ( _instance->m_sleepTime ) {
					usleep(_instance->m_sleepTime*1000);
				}
			}
        }

		void HdmiCecSink::allocateLAforTV()
        {
        	bool gotLogicalAddress = false;
			int addr = LogicalAddress::TV;
			int i;
			
			for ( i =0; i<HDMICECSINK_NUMBER_TV_ADDR; i++ )
			{
        	/* poll for TV logical address */
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
            msgProcessor = new HdmiCecSinkProcessor(*smConnection);
            msgFrameListener = new HdmiCecSinkFrameListener(*msgProcessor);
            
            cecEnableStatus = true;

            if(smConnection)
            {
           		LOGWARN("Start Thread %p", smConnection );
			    m_pollThreadState = POLL_THREAD_STATE_POLL;

				if (m_pollThread.joinable())
				m_pollThread.join();

				m_pollThread = std::thread(threadRun);
            }

			
            return;
        }

        void HdmiCecSink::CECDisable(void)
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
            if(true == cecEnableStatus)
                return true;
            else
                return false;
            LOGINFO("getEnabled :%d ",cecEnableStatus);
        }
    } // namespace Plugin
} // namespace WPEFramework
