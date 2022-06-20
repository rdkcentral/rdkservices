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

#pragma once

#include <stdint.h>
#include "ccec/FrameListener.hpp"
#include "ccec/Connection.hpp"

#include "libIARM.h"
#include "ccec/Assert.hpp"
#include "ccec/Messages.hpp"
#include "ccec/MessageDecoder.hpp"
#include "ccec/MessageProcessor.hpp"

#undef Assert // this define from Connection.hpp conflicts with WPEFramework

#include "Module.h"
#include "utils.h"

namespace WPEFramework {

    namespace Plugin {
        class HdmiCec_2FrameListener : public FrameListener
        {
        public:
            HdmiCec_2FrameListener(MessageProcessor &processor) : processor(processor) {}
            void notify(const CECFrame &in) const;
            ~HdmiCec_2FrameListener() {}
        private:
            MessageProcessor &processor;
        };
        
        class HdmiCec_2Processor : public MessageProcessor
        {
        public:
            HdmiCec_2Processor(Connection &conn) : conn(conn) {}
                void process (const ActiveSource &msg, const Header &header);
	        void process (const InActiveSource &msg, const Header &header);
	        void process (const ImageViewOn &msg, const Header &header);
	        void process (const TextViewOn &msg, const Header &header);
	        void process (const RequestActiveSource &msg, const Header &header);
	        void process (const Standby &msg, const Header &header);
	        void process (const GetCECVersion &msg, const Header &header);
	        void process (const CECVersion &msg, const Header &header);
	        void process (const SetMenuLanguage &msg, const Header &header);
	        void process (const GiveOSDName &msg, const Header &header);
	        void process (const GivePhysicalAddress &msg, const Header &header);
	        void process (const GiveDeviceVendorID &msg, const Header &header);
	        void process (const SetOSDString &msg, const Header &header);
	        void process (const SetOSDName &msg, const Header &header);
	        void process (const RoutingChange &msg, const Header &header);
	        void process (const RoutingInformation &msg, const Header &header);
	        void process (const SetStreamPath &msg, const Header &header);
	        void process (const GetMenuLanguage &msg, const Header &header);
	        void process (const ReportPhysicalAddress &msg, const Header &header);
            void process (const DeviceVendorID &msg, const Header &header);
	        void process (const GiveDevicePowerStatus &msg, const Header &header);
	        void process (const ReportPowerStatus &msg, const Header &header);
	        void process (const FeatureAbort &msg, const Header &header);
	        void process (const Abort &msg, const Header &header);
	        void process (const Polling &msg, const Header &header);
        private:
            Connection conn;
            void printHeader(const Header &header)
            {
                printf("Header : From : %s \n", header.from.toString().c_str());
                printf("Header : to   : %s \n", header.to.toString().c_str());
            }
        
        };

#define BIT_DEVICE_PRESENT    (0)

	class CECDeviceInfo_2 {
		public:

		LogicalAddress m_logicalAddress;
		VendorID m_vendorID;
		OSDName m_osdName;
		//<Bits 16 - 1: unused><Bit 0: DevicePresent>
		short m_deviceInfoStatus;
	        bool m_isOSDNameUpdated;
	        bool m_isVendorIDUpdated;

		CECDeviceInfo_2()
		: m_logicalAddress(0),m_vendorID(0,0,0),m_osdName("NA"), m_isOSDNameUpdated (false), m_isVendorIDUpdated (false)
		{
			BITMASK_CLEAR(m_deviceInfoStatus, 0xFFFF); //Clear all bits
		}

		void clear( )
		{
			m_logicalAddress = 0;
			m_vendorID = VendorID(0,0,0);
			m_osdName = "NA";
			BITMASK_CLEAR(m_deviceInfoStatus, 0xFFFF); //Clear all bits
			m_isOSDNameUpdated = false;
			m_isVendorIDUpdated = false;
		}

		bool update ( const VendorID &vendorId) {
			bool isVendorIdUpdated = false;
			if (!m_isVendorIDUpdated)
				isVendorIdUpdated = true; //First time no need to cross check the value. Since actual value can be default value
			else
				isVendorIdUpdated = (m_vendorID.toString().compare(vendorId.toString())==0)?false:true;

			m_isVendorIDUpdated = true;
			m_vendorID = vendorId;
			return isVendorIdUpdated;
		}

		bool update ( const OSDName    &osdName ) {
			bool isOSDNameUpdated = false;
			if (!m_isOSDNameUpdated)
				isOSDNameUpdated = true; //First time no need to cross check the value. Since actual value can be default value
			else
				isOSDNameUpdated = (m_osdName.toString().compare(osdName.toString())==0)?false:true;

			m_isOSDNameUpdated = true;
			m_osdName = osdName;
			return isOSDNameUpdated;
		}

	};

		// This is a server for a JSONRPC communication channel. 
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class HdmiCec_2 : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        public:
            HdmiCec_2();
            virtual ~HdmiCec_2();
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }
            static HdmiCec_2* _instance;
            CECDeviceInfo_2 deviceList[16];
            pthread_cond_t m_condSig;
            pthread_mutex_t m_lock;
            pthread_cond_t m_condSigUpdate;
            pthread_mutex_t m_lockUpdate;

            void SendStandbyMsgEvent(const int logicalAddress);
            void sendActiveSourceEvent();
            void addDevice(const int logicalAddress);
            void removeDevice(const int logicalAddress);
            void sendUnencryptMsg(unsigned char* msg, int size);
            void sendDeviceUpdateInfo(const int logicalAddress);

            BEGIN_INTERFACE_MAP(HdmiCec_2)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            // We do not allow this plugin to be copied !!
            HdmiCec_2(const HdmiCec_2&) = delete;
            HdmiCec_2& operator=(const HdmiCec_2&) = delete;

            //Begin methods
            uint32_t setEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getOTPEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setOSDNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getOSDNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setVendorIdWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getVendorIdWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t performOTPActionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t sendStandbyMessageWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getDeviceList (const JsonObject& parameters, JsonObject& response);
            uint32_t getActiveSourceStatus(const JsonObject& parameters, JsonObject& response);

            //End methods
            std::string logicalAddressDeviceType;
            bool cecSettingEnabled;
            bool cecOTPSettingEnabled;
            bool cecEnableStatus;
            bool IsCecMgrActivated;
            Connection *smConnection;
            int m_numberOfDevices;
            bool m_pollThreadExit;
            std::thread m_pollThread;
            bool m_updateThreadExit;
            std::thread m_UpdateThread;

            HdmiCec_2Processor *msgProcessor;
            HdmiCec_2FrameListener *msgFrameListener;
            const void InitializeIARM();
            void DeinitializeIARM();
            static void cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void onCECDaemonInit();
            void cecStatusUpdated(void *evtStatus);
            void onHdmiHotPlug(int connectStatus);
            bool loadSettings();
            void persistSettings(bool enableStatus);
            void persistOTPSettings(bool enableStatus);
            void persistOSDName(const char *name);
            void persistVendorId(unsigned int vendorID);
            void setEnabled(bool enabled, bool isPersist);
            bool getEnabled();
            void setOTPEnabled(bool enabled);
            bool getOTPEnabled();
            bool performOTPAction();
            void CECEnable(void);
            void CECDisable(void);
            void getPhysicalAddress();
            void getLogicalAddress();
            void cecAddressesChanged(int changeStatus);
            bool sendStandbyMessage();
            bool pingDeviceUpdateList (int idev);
            void removeAllCecDevices();
            void requestVendorID(const int newDevlogicalAddress);
            void requestOsdName(const int newDevlogicalAddress);
            void requestCecDevDetails(const int logicalAddress);
            static void threadRun();
            static void threadUpdateCheck();
        };
	} // namespace Plugin
} // namespace WPEFramework




