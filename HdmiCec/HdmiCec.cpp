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

#include "HdmiCec.h"


#include "ccec/Connection.hpp"
#include "ccec/CECFrame.hpp"
#include "host.hpp"
#include "ccec/host/RDK.hpp"

#include "ccec/drivers/iarmbus/CecIARMBusMgr.h"


#include "dsMgr.h"
#include "dsDisplay.h"
#include "videoOutputPort.hpp"

#include "websocket/URL.h"

#include "UtilsIarm.h"

#define HDMICEC_METHOD_SET_ENABLED "setEnabled"
#define HDMICEC_METHOD_GET_ENABLED "getEnabled"
#define HDMICEC_METHOD_GET_CEC_ADDRESSES "getCECAddresses"
#define HDMICEC_METHOD_SEND_MESSAGE "sendMessage"
#define HDMICEC_METHOD_GET_ACTIVE_SOURCE_STATUS "getActiveSourceStatus"

#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_MESSAGE "onMessage"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define HDMICEC_EVENT_ON_CEC_ADDRESS_CHANGE "cecAddressesChanged"

#define PHYSICAL_ADDR_CHANGED 1
#define LOGICAL_ADDR_CHANGED 2
#define DEV_TYPE_TUNER 1

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0

enum {
	HDMICEC_EVENT_DEVICE_ADDED=0,
	HDMICEC_EVENT_DEVICE_REMOVED,
	HDMICEC_EVENT_DEVICE_INFO_UPDATED,
        HDMICEC_EVENT_ACTIVE_SOURCE_STATUS_UPDATED,
};

static char *eventString[] = {
	"onDeviceAdded",
	"onDeviceRemoved",
	"onDeviceInfoUpdated",
        "onActiveSourceStatusUpdated"
};

static bool isDeviceActiveSource = false;

#if defined(HAS_PERSISTENT_IN_HDD)
#define CEC_SETTING_ENABLED_FILE "/tmp/mnt/diska3/persistent/ds/cecData.json"
#elif defined(HAS_PERSISTENT_IN_FLASH)
#define CEC_SETTING_ENABLED_FILE "/opt/persistent/ds/cecData.json"
#else
#define CEC_SETTING_ENABLED_FILE "/opt/ds/cecData.json"
#endif

#define CEC_SETTING_ENABLED "cecEnabled"

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiCec, 1, 0);

        HdmiCec* HdmiCec::_instance = nullptr;

        static int libcecInitStatus = 0;

//=========================================== HdmiCec cec msg Processor =========================================
       void HdmiCec::process (const ActiveSource &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ActiveSource %s : %s  : %s \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
             HdmiCec::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec::process (const ImageViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: ImageViewOn from %s\n", header.from.toString().c_str());
             HdmiCec::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec::process (const TextViewOn &msg, const Header &header)
       {
             printHeader(header);
             LOGINFO("Command: TextViewOn\n");
             HdmiCec::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec::process (const CECVersion &msg, const Header &header)
       {
	     bool updateStatus;
	     printHeader(header);
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());

	     HdmiCec::_instance->addDevice(header.from.toInt());
       }

       void HdmiCec::process (const SetOSDName &msg, const Header &header)
       {
             printHeader(header);
	     bool updateStatus ;
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());

	     bool isOSDNameUpdated = HdmiCec::_instance->deviceList[header.from.toInt()].update(msg.osdName);
	     if (isOSDNameUpdated)
	         HdmiCec::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCec::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             printHeader(header);
	     bool updateDeviceTypeStatus;
	     bool updatePAStatus;
             LOGINFO("Command: ReportPhysicalAddress\n");

	     if(!HdmiCec::_instance)
	        return;
             HdmiCec::_instance->addDevice(header.from.toInt());
       }
       void HdmiCec::process (const DeviceVendorID &msg, const Header &header)
       {
	     bool updateStatus ;
	     printHeader(header);
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());

	     bool isVendorIdUpdated = HdmiCec::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
	     if (isVendorIdUpdated)
	         HdmiCec::_instance->sendDeviceUpdateInfo(header.from.toInt());
       }
       void HdmiCec::process (const ReportPowerStatus &msg, const Header &header)
       {
	   uint32_t  oldPowerStatus,newPowerStatus;
	   printHeader(header);
	   LOGINFO("Command: ReportPowerStatus Power Status from:%s status : %s \n",header.from.toString().c_str(),msg.status.toString().c_str());
	   HdmiCec::_instance->addDevice(header.from.toInt());
       }
//=========================================== HdmiCec =========================================

        HdmiCec::HdmiCec()
        : PluginHost::JSONRPC(),smConnection(nullptr),cecEnableStatus(false)
        {
            HdmiCec::_instance = this;
            InitializeIARM();

            Register(HDMICEC_METHOD_SET_ENABLED, &HdmiCec::setEnabledWrapper, this);
            Register(HDMICEC_METHOD_GET_ENABLED, &HdmiCec::getEnabledWrapper, this);
            Register(HDMICEC_METHOD_GET_CEC_ADDRESSES, &HdmiCec::getCECAddressesWrapper, this);
            Register(HDMICEC_METHOD_SEND_MESSAGE, &HdmiCec::sendMessageWrapper, this);
            Register(HDMICEC_METHOD_GET_ACTIVE_SOURCE_STATUS, &HdmiCec::getActiveSourceStatus, this);
            Register("getDeviceList", &HdmiCec::getDeviceList, this);

            physicalAddress = 0x0F0F0F0F;

            logicalAddressDeviceType = "None";
            logicalAddress = 0xFF;

            loadSettings();
            if (cecSettingEnabled)
            {
                setEnabled(cecSettingEnabled);
            }
            else
            {
                setEnabled(false);
                Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_ENABLED, JsonValue(false));
            }
        }

        HdmiCec::~HdmiCec()
        {
        }

        void HdmiCec::Deinitialize(PluginHost::IShell* /* service */)
        {
            isDeviceActiveSource = false;
            HdmiCec::_instance->sendActiveSourceEvent();
            CECDisable();
            HdmiCec::_instance = nullptr;

            DeinitializeIARM();

        }

        uint32_t HdmiCec::getActiveSourceStatus(const JsonObject& parameters, JsonObject& response)
        {

            LOGINFO("getActiveSourceStatus isDeviceActiveSource: %d \n ",isDeviceActiveSource);
            response["status"] = isDeviceActiveSource;
            returnResponse(true);
        }

        const void HdmiCec::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                //IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECHOST_NAME, IARM_BUS_CECHost_EVENT_DEVICESTATUSCHANGE,cecDeviceStatusEventHandler) ); // It didn't do anything in original service
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED,cecMgrEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, dsHdmiEventHandler) );
            }
        }

        void HdmiCec::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                //IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECHOST_NAME, IARM_BUS_CECHost_EVENT_DEVICESTATUSCHANGE) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_EVENT_STATUS_UPDATED) );
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG) );
            }
        }

        void HdmiCec::cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdmiCec::_instance)
                return;

            if( !strcmp(owner, IARM_BUS_CECMGR_NAME))
            {
                switch (eventId)
                {
                    case IARM_BUS_CECMGR_EVENT_DAEMON_INITIALIZED:
                    {
                        HdmiCec::_instance->onCECDaemonInit();
                    }
                    break;
                    case IARM_BUS_CECMGR_EVENT_STATUS_UPDATED:
                    {
                        IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = new IARM_Bus_CECMgr_Status_Updated_Param_t;
                        if(evtData)
                        {
                            memcpy(evtData,data,sizeof(IARM_Bus_CECMgr_Status_Updated_Param_t));
                            HdmiCec::_instance->cecStatusUpdated(evtData);
                        }
                    }
                    break;
                    default:
                    /*Do nothing*/
                    break;
                }
            }
        }

        void HdmiCec::dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if(!HdmiCec::_instance)
                return;

            if (IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG == eventId)
            {
                IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
                int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;
                LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);
                HdmiCec::_instance->onHdmiHotPlug(hdmi_hotplug_event);
                //Trigger CEC device poll here
                pthread_cond_signal(&(_instance->m_condSig));
            }
        }

        void HdmiCec::onCECDaemonInit()
        {
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

        void HdmiCec::cecStatusUpdated(void *evtStatus)
        {
            IARM_Bus_CECMgr_Status_Updated_Param_t *evtData = (IARM_Bus_CECMgr_Status_Updated_Param_t *)evtStatus;
            if(evtData)
            {
               try{
                    getPhysicalAddress();

                    unsigned int logicalAddr = evtData->logicalAddress;
                    std::string logicalAddrDeviceType = DeviceType(LogicalAddress(evtData->logicalAddress).getType()).toString().c_str();

                    LOGWARN("cecLogicalAddressUpdated: logical address updated: %d , saved : %d ", logicalAddr, logicalAddress);
                    if (logicalAddr != logicalAddress || logicalAddrDeviceType != logicalAddressDeviceType)
                    {
                        logicalAddress = logicalAddr;
                        logicalAddressDeviceType = logicalAddrDeviceType;
                        cecAddressesChanged(LOGICAL_ADDR_CHANGED);
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

        void HdmiCec::onHdmiHotPlug(int connectStatus)
        {
            if (HDMI_HOT_PLUG_EVENT_CONNECTED == connectStatus)
            {
                LOGWARN("onHdmiHotPlug Status : %d ", connectStatus);
                getPhysicalAddress();
                getLogicalAddress();
            }
            return;
        }

        uint32_t HdmiCec::setEnabledWrapper(const JsonObject& parameters, JsonObject& response)
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

        uint32_t HdmiCec::getEnabledWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["enabled"] = getEnabled();
            returnResponse(true);
        }

        uint32_t HdmiCec::getCECAddressesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            response["CECAddresses"] = getCECAddresses();
            returnResponse(true);
        }

        uint32_t HdmiCec::sendMessageWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            std::string message;

            if (parameters.HasLabel("message"))
            {
                message = parameters["message"].String();
            }
            else
            {
                returnResponse(false);
            }

            sendMessage(message);
            returnResponse(true);
        }

        bool HdmiCec::loadSettings()
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            file.Open();
            JsonObject parameters;
            parameters.IElement::FromFile(file);

            file.Close();

            getBoolParameter(CEC_SETTING_ENABLED, cecSettingEnabled);

            return cecSettingEnabled;
        }

        void HdmiCec::setEnabled(bool enabled)
        {
           LOGWARN("Entered setEnabled ");

           if (cecSettingEnabled != enabled)
           {
               Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_ENABLED, JsonValue(enabled));
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

        void HdmiCec::CECEnable(void)
        {
            LOGWARN("Entered CECEnable");
            if (cecEnableStatus)
            {
                LOGWARN("CEC Already Enabled");
                return;
            }

            char c;
            IARM_Result_t retVal = IARM_RESULT_SUCCESS;
            retVal = IARM_Bus_Call_with_IPCTimeout(IARM_BUS_CECMGR_NAME, IARM_BUS_CECMGR_API_isAvailable, (void *)&c, sizeof(c), 1000);
            if(retVal != IARM_RESULT_SUCCESS) {
                LOGINFO("CECMGR is not available. Failed to enable HdmiCec Plugin");
                cecEnableStatus = false;
                return;
            } else {
                LOGINFO("CECMGR is available");
            }

            if(0 == libcecInitStatus)
            {
                try
                {
                    LibCCEC::getInstance().init();
                }
                catch (const std::exception e)
                {
                    LOGWARN("CEC exception caught from CECEnable");
                }
            }
            libcecInitStatus++;

            smConnection = new Connection(LogicalAddress::UNREGISTERED,false,"ServiceManager::Connection::");
            smConnection->open();
            smConnection->addFrameListener(this);

            //Acquire CEC Addresses
            getPhysicalAddress();
            getLogicalAddress();
            if(smConnection)
            {

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
            cecEnableStatus = true;
            return;
        }

        void HdmiCec::CECDisable(void)
        {
            LOGWARN("Entered CECDisable ");

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
                LibCCEC::getInstance().term();
            }

            if(libcecInitStatus > 0)
            {
                libcecInitStatus--;
            }

            return;
        }


        void HdmiCec::getPhysicalAddress()
        {
            LOGINFO("Entered getPhysicalAddress ");

            uint32_t physAddress = 0x0F0F0F0F;

            try {
                    LibCCEC::getInstance().getPhysicalAddress(&physAddress);

                    LOGINFO("getPhysicalAddress: physicalAddress: %x %x %x %x ", (physAddress >> 24) & 0xFF, (physAddress >> 16) & 0xFF, (physAddress >> 8)  & 0xFF, (physAddress) & 0xFF);
                    if (physAddress != physicalAddress)
                    {
                        physicalAddress = physAddress;
                        cecAddressesChanged(PHYSICAL_ADDR_CHANGED);
                    }
            }
            catch (const std::exception e)
            {
                LOGWARN("DS exception caught from getPhysicalAddress");
            }
            return;
        }

        void HdmiCec::getLogicalAddress()
        {
            LOGINFO("Entered getLogicalAddress ");

            try{
                int addr = LibCCEC::getInstance().getLogicalAddress(DEV_TYPE_TUNER);

                std::string logicalAddrDeviceType = DeviceType(LogicalAddress(addr).getType()).toString().c_str();

                LOGWARN("logical address obtained is %d , saved logical address is %d ", addr, logicalAddress);

                if ((int)logicalAddress != addr || logicalAddressDeviceType != logicalAddrDeviceType)

                {
                    logicalAddress = addr;
                    logicalAddressDeviceType = logicalAddrDeviceType;
                    cecAddressesChanged(LOGICAL_ADDR_CHANGED);
                }
            }
            catch (const std::exception e)
            {
                LOGWARN("CEC exception caught from getLogicalAddress ");
            }

            return;
        }

        bool HdmiCec::getEnabled()
        {
            LOGWARN("Entered getEnabled ");
            if(true == cecEnableStatus)
                return true;
            else
                return false;
        }

        void HdmiCec::setName(std::string name)
        {
            //SVCLOG_WARN("%s \r\n",__FUNCTION__);
            return;
        }

        std::string HdmiCec::getName()
        {
            //SVCLOG_WARN("%s \r\n",__FUNCTION__);
            IARM_Result_t ret = IARM_RESULT_INVALID_STATE;
            if (ret != IARM_RESULT_SUCCESS)
            {
                LOGWARN("getName :: IARM_BUS_CEC_HOST_GetOSDName failed ");
                return "STB";
            }

            return "STB";
        }

        JsonObject HdmiCec::getCECAddresses()
        {
            JsonObject CECAddress;
            CECAddress["physicalAddress"] = physicalAddress;
            CECAddress["logicalAddress"] = logicalAddress;
            CECAddress["deviceType"] = logicalAddressDeviceType;
            LOGWARN("getCECAddresses: physicalAddress : %x logicalAddress :%x ", physicalAddress ,logicalAddress);
            return CECAddress;
        }

        // Copy of Core::FromString, which doesn't add extra zero at the end
        uint16_t HdmiCec::FromBase64String(const string& newValue, uint8_t object[], uint16_t& length, const TCHAR* ignoreList)
        {
            uint8_t state = 0;
            uint16_t index = 0;
            uint16_t filler = 0;
            uint8_t lastStuff = 0;

            while ((index < newValue.size()) && (filler < length)) {
                uint8_t converted;
                TCHAR current = newValue[index++];

                if ((current >= 'A') && (current <= 'Z')) {
                    converted = (current - 'A');
                } else if ((current >= 'a') && (current <= 'z')) {
                    converted = (current - 'a' + 26);
                } else if ((current >= '0') && (current <= '9')) {
                    converted = (current - '0' + 52);
                } else if (current == '+') {
                    converted = 62;
                } else if (current == '/') {
                    converted = 63;
                } else if ((ignoreList != nullptr) && (::strchr(ignoreList, current) != nullptr)) {
                    continue;
                } else {
                    break;
                }

                if (state == 0) {
                    lastStuff = converted << 2;
                    state = 1;
                } else if (state == 1) {
                    object[filler++] = (((converted & 0x30) >> 4) | lastStuff);
                    lastStuff = ((converted & 0x0F) << 4);
                    state = 2;
                } else if (state == 2) {
                    object[filler++] = (((converted & 0x3C) >> 2) | lastStuff);
                    lastStuff = ((converted & 0x03) << 6);
                    state = 3;
                } else if (state == 3) {
                    object[filler++] = ((converted & 0x3F) | lastStuff);
                    state = 0;
                }
            }

            // No need to do this
            /*if ((state != 0) && (filler < length)) {
                object[filler++] = lastStuff;
                LOGINFO("state %d, lastStuff = %d", state, lastStuff);
            }*/

            length = filler;

            return (index);
        }

        void HdmiCec::sendMessage(std::string message)
        {
            LOGINFO("sendMessage ");

            if(true == cecEnableStatus)
            {
                std::vector <unsigned char> buf;
                buf.resize(message.size());

                uint16_t decodedLen = message.size();
                FromBase64String(message, (uint8_t*)buf.data(), decodedLen, NULL);

                if(decodedLen>=2)
                {
                     if(buf.at(1)== ACTIVE_SOURCE)
                     {
                         LOGINFO("sendMessage  sending active source messages set isDeviceActiveSource to true \n ");
                         isDeviceActiveSource = true;
			 HdmiCec::_instance->sendActiveSourceEvent();
                     }
                }
                CECFrame frame = CECFrame((const uint8_t *)buf.data(), decodedLen);
        //      SVCLOG_WARN("Frame to be sent from servicemanager in %s \n",__FUNCTION__);
        //      frame.hexDump();
                smConnection->sendAsync(frame);
            }
            else
                LOGWARN("cecEnableStatus=false");
            return;
        }

        void HdmiCec::sendActiveSourceEvent()
        {
            JsonObject params;
            params["status"] = isDeviceActiveSource;
            LOGWARN("sendActiveSourceEvent isDeviceActiveSource: %d ",isDeviceActiveSource);
            sendNotify(eventString[HDMICEC_EVENT_ACTIVE_SOURCE_STATUS_UPDATED], params);
        }

        void HdmiCec::cecAddressesChanged(int changeStatus)
        {
            JsonObject params;
            JsonObject CECAddresses;

            LOGWARN(" cecAddressesChanged Change Status : %d ", changeStatus);
            if(PHYSICAL_ADDR_CHANGED == changeStatus)
            {
                CECAddresses["physicalAddress"] = physicalAddress;
            }
            else if(LOGICAL_ADDR_CHANGED == changeStatus)
            {
                CECAddresses["logicalAddress"] = logicalAddress;
                CECAddresses["deviceType"] = logicalAddressDeviceType;
            }
            else
            {
                //Do Nothing
            }

            params["CECAddresses"] = CECAddresses;
            LOGWARN(" cecAddressesChanged  send : %s ", HDMICEC_EVENT_ON_CEC_ADDRESS_CHANGE);

            sendNotify(HDMICEC_EVENT_ON_CEC_ADDRESS_CHANGE, params);

            return;
        }

        void HdmiCec::notify(const CECFrame &in) const
        {
            LOGINFO("Inside notify ");
            size_t length;
            const uint8_t *input_frameBuf = NULL;
            CECFrame Frame = in;
        //  Frame.hexDump();
            Frame.getBuffer(&input_frameBuf,&length);

            if(length >=2)
            {
                 if(input_frameBuf[1] == ROUTING_CHANGE || input_frameBuf[1] == ROUTING_INFORMATION || input_frameBuf[1] == ACTIVE_SOURCE || input_frameBuf[1] == SET_STREAM_PATH)
                 {
		     int paIndex = (input_frameBuf[1]==ROUTING_CHANGE) ? 4: 2;
                     unsigned int tempPhyAddres = ( ((input_frameBuf[paIndex] >> 4 & 0x0f) <<24) |((input_frameBuf[paIndex]  & 0x0f) <<16) |
                                                    ((input_frameBuf[paIndex+1] >> 4 & 0x0f) <<8) | (input_frameBuf[paIndex+1]  & 0x0f));

		     if(physicalAddress != tempPhyAddres)
                         isDeviceActiveSource = false;
		     else
                         isDeviceActiveSource = true;
                     HdmiCec::_instance->sendActiveSourceEvent();
                     LOGINFO("Active Source Event : Device Physical Address :%x Physical Address from message :%x isDeviceActiveSource status :%d   ",physicalAddress,tempPhyAddres,isDeviceActiveSource);
                 }
            }

            std::vector <char> buf;
            buf.resize(length * 2);
	    string bufbase64 = "";
            Core::ToString((uint8_t*)input_frameBuf, length, true, bufbase64);

            if (HdmiCec::_instance) {
                MessageDecoder((*(HdmiCec::_instance))).decode(in);
            } else {
                LOGWARN("HdmiCec::_instance NULL Cec msg decoding failed.");
            }
            LOGINFO("recvMessage :%d  :%s ",bufbase64.length(),bufbase64.c_str());
            (const_cast<HdmiCec*>(this))->onMessage(bufbase64.c_str());
            return;
        }

        void HdmiCec::onMessage( const char *message )
        {
            JsonObject params;
            params["message"] = message;
            sendNotify(HDMICEC_EVENT_ON_MESSAGE, params);
        }

        uint32_t HdmiCec::getDeviceList (const JsonObject& parameters, JsonObject& response)
        {   //sample servicemanager response:
		LOGINFOMETHOD();
		//Trigger CEC device poll here
		pthread_cond_signal(&(_instance->m_condSig));

		bool success = true;
		response["numberofdevices"] = HdmiCec::_instance->m_numberOfDevices;
		LOGINFO("getDeviceListWrapper  m_numberOfDevices :%d \n", HdmiCec::_instance->m_numberOfDevices);
		JsonArray deviceListArg;
		try
		{
			int i = 0;
			for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				if (BIT_CHECK(deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
					JsonObject device;
					device["logicalAddress"] = HdmiCec::_instance->deviceList[i].m_logicalAddress.toInt();
					device["osdName"] = HdmiCec::_instance->deviceList[i].m_osdName.toString().c_str();
					device["vendorID"] = HdmiCec::_instance->deviceList[i].m_vendorID.toString().c_str();
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

	bool HdmiCec::pingDeviceUpdateList (int idev)
	{
		bool isConnected = false;
		if(!HdmiCec::_instance)
		{
			LOGERR("HdmiCec::_instance not existing");
			return isConnected;
		}
		if ( !(_instance->smConnection) || _instance->logicalAddress == LogicalAddress::UNREGISTERED || (false == cecEnableStatus)){
			LOGERR("Exiting from pingDeviceUpdateList _instance->smConnection:%p, _instance->logicalAddress:%d, cecEnableStatus=%d",
					_instance->smConnection, _instance->logicalAddress, cecEnableStatus);
			return isConnected;
		}

		LOGWARN("PING for  0x%x \r\n",idev);
		try {
			_instance->smConnection->ping(LogicalAddress(_instance->logicalAddress), LogicalAddress(idev), Throw_e());
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

	void  HdmiCec::sendDeviceUpdateInfo(const int logicalAddress)
	{
		JsonObject params;
		params["logicalAddress"] = JsonValue(logicalAddress);
		LOGINFO("Device info updated notification send: for logical address:%d\r\n", logicalAddress);
		sendNotify(eventString[HDMICEC_EVENT_DEVICE_INFO_UPDATED], params);
	}

	void HdmiCec::addDevice(const int logicalAddress) {

		if(!HdmiCec::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if ( !(BIT_CHECK(HdmiCec::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		 {
			BIT_SET(HdmiCec::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT);
			HdmiCec::_instance->deviceList[logicalAddress].m_logicalAddress = LogicalAddress(logicalAddress);
			HdmiCec::_instance->m_numberOfDevices++;
			LOGINFO("New cec ligical address add notification send:  \r\n");
			sendNotify(eventString[HDMICEC_EVENT_DEVICE_ADDED], JsonObject());
		 }
		//Two source devices can have same logical address.
		requestCecDevDetails(logicalAddress);
	}

	void HdmiCec::removeAllCecDevices() {
		int i = 0;
		for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
			removeDevice (i);
		}
	}
	void HdmiCec::removeDevice(const int logicalAddress) {
		if(!HdmiCec::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED ){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if (BIT_CHECK(HdmiCec::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
		{
			_instance->m_numberOfDevices--;
			_instance->deviceList[logicalAddress].clear();
			LOGINFO("Cec ligical address remove notification send:  \r\n");
			sendNotify(eventString[HDMICEC_EVENT_DEVICE_REMOVED], JsonObject());
		}
	}

	void HdmiCec::sendUnencryptMsg(unsigned char* msg, int size)
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

	void HdmiCec::requestVendorID(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned char sender = (unsigned char)(_instance->logicalAddress & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request vendor id
		msg [1] = 0x8c;
		LOGINFO("Sending msg request vendor id %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCec::requestOsdName(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned char sender = (unsigned char)(_instance->logicalAddress & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request OSD  name
		msg [1] = 0x46;
		LOGINFO("Sending msg request osd name %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCec::requestCecDevDetails(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		requestVendorID (newDevlogicalAddress);
		requestOsdName (newDevlogicalAddress);
	}

	void HdmiCec::threadRun()
	{
		if(!HdmiCec::_instance)
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

	void HdmiCec::threadUpdateCheck()
	{
		if(!HdmiCec::_instance)
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
				if (BIT_CHECK(HdmiCec::_instance->deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
				{
					int itr = 0;
					bool retry = true;
					int iCounter = 0;
					for (itr = 0; ((itr<5)&&(retry)); itr++){

						if (!HdmiCec::_instance->deviceList[i].m_isOSDNameUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 2sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCec::_instance->requestOsdName (i);
							retry = true;
						}
						else {
							retry = false;
						}

						if (!HdmiCec::_instance->deviceList[i].m_isVendorIDUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 1sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCec::_instance->requestVendorID (i);
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



