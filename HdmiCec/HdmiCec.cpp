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

#include "utils.h"

#define HDMICEC_METHOD_SET_ENABLED "setEnabled"
#define HDMICEC_METHOD_GET_ENABLED "getEnabled"
#define HDMICEC_METHOD_GET_CEC_ADDRESSES "getCECAddresses"
#define HDMICEC_METHOD_SEND_MESSAGE "sendMessage"

#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_MESSAGE "onMessage"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define HDMICEC_EVENT_ON_CEC_ADDRESS_CHANGE "cecAddressesChanged"

#define PHYSICAL_ADDR_CHANGED 1
#define LOGICAL_ADDR_CHANGED 2
#define DEV_TYPE_TUNER 1

#define HDMI_HOT_PLUG_EVENT_CONNECTED 0

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

        HdmiCec::HdmiCec()
        : AbstractPlugin(),smConnection(nullptr),cecEnableStatus(false)
        {
            HdmiCec::_instance = this;

            InitializeIARM();

            registerMethod(HDMICEC_METHOD_SET_ENABLED, &HdmiCec::setEnabledWrapper, this);
            registerMethod(HDMICEC_METHOD_GET_ENABLED, &HdmiCec::getEnabledWrapper, this);
            registerMethod(HDMICEC_METHOD_GET_CEC_ADDRESSES, &HdmiCec::getCECAddressesWrapper, this);
            registerMethod(HDMICEC_METHOD_SEND_MESSAGE, &HdmiCec::sendMessageWrapper, this);

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
            CECDisable();
            HdmiCec::_instance = nullptr;

            DeinitializeIARM();

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
            LOGINFO("Entered getCECAddresses ");

            char pa[32] = {0};
            snprintf(pa, sizeof(pa), "\\u00%02X\\u00%02X\\u00%02X\\u00%02X", (physicalAddress >> 24) & 0xff, (physicalAddress >> 16) & 0xff, (physicalAddress >> 8) & 0xff, physicalAddress & 0xff);

            CECAddress["physicalAddress"] = (const char *)pa;

            JsonObject logical;
            logical["deviceType"] = logicalAddressDeviceType;
            logical["logicalAddress"] = logicalAddress;

            CECAddress["logicalAddresses"] = logical;
            LOGWARN("getCECAddresses: physicalAddress from QByteArray : %x %x %x %x ", (physicalAddress >> 24) & 0xFF, (physicalAddress >> 16) & 0xFF, (physicalAddress >> 8)  & 0xFF, (physicalAddress) & 0xFF);
            LOGWARN("getCECAddresses: logical address: %x  ", logicalAddress);

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

                CECFrame frame = CECFrame((const uint8_t *)buf.data(), decodedLen);
        //      SVCLOG_WARN("Frame to be sent from servicemanager in %s \n",__FUNCTION__);
        //      frame.hexDump();
                smConnection->sendAsync(frame);
            }
            else
                LOGWARN("cecEnableStatus=false");
            return;
        }

        void HdmiCec::cecAddressesChanged(int changeStatus)
        {
            JsonObject params;
            JsonObject CECAddresses;

            LOGWARN(" cecAddressesChanged Change Status : %d ", changeStatus);
            if(PHYSICAL_ADDR_CHANGED == changeStatus)
            {
                char pa[32] = {0};
                snprintf(pa, sizeof(pa), "\\u00%02X\\u00%02X\\u00%02X\\u00%02X", (physicalAddress >> 24) & 0xff, (physicalAddress >> 16) & 0xff, (physicalAddress >> 8) & 0xff, physicalAddress & 0xff);

                CECAddresses["physicalAddress"] = (const char *)pa;
            }
            else if(LOGICAL_ADDR_CHANGED == changeStatus)
            {
                CECAddresses["logicalAddresses"] = logicalAddress;
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
        //  SVCLOG_WARN("Frame received by servicemanager is \n");
        //  Frame.hexDump();
            Frame.getBuffer(&input_frameBuf,&length);

            std::vector <char> buf;
            buf.resize(length * 2);

            uint16_t encodedLen = Core::URL::Base64Encode(input_frameBuf, length, buf.data(), buf.size());
            buf[encodedLen] = 0;

            (const_cast<HdmiCec*>(this))->onMessage(buf.data());
            return;
        }

        void HdmiCec::onMessage( const char *message )
        {
            JsonObject params;
            params["message"] = message;
            sendNotify(HDMICEC_EVENT_ON_MESSAGE, params);
        }

    } // namespace Plugin
} // namespace WPEFramework



