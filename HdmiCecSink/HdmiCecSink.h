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

#include "libIBus.h"
#include "ccec/Assert.hpp"
#include "ccec/Messages.hpp"
#include "ccec/MessageDecoder.hpp"
#include "ccec/MessageProcessor.hpp"

#undef Assert // this define from Connection.hpp conflicts with WPEFramework

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"

#include <thread>
#include <mutex>


namespace WPEFramework {

    namespace Plugin {
        class HdmiCecSinkFrameListener : public FrameListener
        {
        public:
            HdmiCecSinkFrameListener(MessageProcessor &processor) : processor(processor) {}
            void notify(const CECFrame &in) const;
            ~HdmiCecSinkFrameListener() {}
        private:
            MessageProcessor &processor;
        };
        
        class HdmiCecSinkProcessor : public MessageProcessor
        {
        public:
            HdmiCecSinkProcessor(Connection &conn) : conn(conn) {}
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

		class CECDeviceParams {
			public:
				
			enum {
				REQUEST_NONE = 0,
				REQUEST_PHISICAL_ADDRESS = 1,
				REQUEST_CEC_VERSION,
				REQUEST_DEVICE_VENDOR_ID,
				REQUEST_POWER_STATUS,
				REQUEST_OSD_NAME,
			};

			enum {
				REQUEST_DONE = 0,
				REQUEST_NOT_DONE,
				REQUEST_TIME_ELAPSED,
			};
			
			DeviceType m_deviceType;
			LogicalAddress m_logicalAddress;
			PhysicalAddress m_physicalAddr;
			Version m_cecVersion;
			VendorID m_vendorID;
			OSDName m_osdName;
			PowerStatus m_powerStatus;
			bool m_isDevicePresent;
			bool m_isDeviceDisconnected;
			Language m_currentLanguage;
			bool m_isActiveSource;
			bool m_isDeviceTypeUpdated;
			bool m_isPAUpdated;
			bool m_isVersionUpdated;
			bool m_isOSDNameUpdated;
			bool m_isVendorIDUpdated;
			bool m_isPowerStatusUpdated;
			int  m_isRequested;
			int  m_isRequestRetry;
			std::chrono::system_clock::time_point m_requestTime;
			std::vector<FeatureAbort> m_featureAborts;
			std::chrono::system_clock::time_point m_lastPowerUpdateTime;
			
			CECDeviceParams() 
			: m_deviceType(0), m_logicalAddress(0),m_physicalAddr(0,0,0,0),m_cecVersion(0),m_vendorID(0,0,0),m_osdName("NA"),m_powerStatus(0),m_currentLanguage("NA")
			{
				m_isDevicePresent = false;
				m_isActiveSource = false;
				m_isPAUpdated = false;
				m_isVersionUpdated = false;
				m_isOSDNameUpdated = false;
				m_isVendorIDUpdated = false;
				m_isPowerStatusUpdated = false;
				m_isDeviceDisconnected = false;
				m_isDeviceTypeUpdated = false;
				m_isRequestRetry = 0;
			}

			void clear( ) 
			{
				m_deviceType = 0;
				m_logicalAddress = 0;
				m_physicalAddr = PhysicalAddress(0,0,0,0);
				m_cecVersion = 0;
				m_vendorID = VendorID(0,0,0);
				m_osdName = "NA";
				m_powerStatus = 0;
 				m_currentLanguage = "NA";
				m_isDevicePresent = false;
				m_isActiveSource = false;
				m_isPAUpdated = false;
				m_isVersionUpdated = false;
				m_isOSDNameUpdated = false;
				m_isVendorIDUpdated = false;
				m_isPowerStatusUpdated = false;
				m_isDeviceDisconnected = false;
				m_isDeviceTypeUpdated = false;
			}

			void printVariable()
			{
				LOGWARN("Device LogicalAddress %s", m_logicalAddress.toString().c_str());
				LOGWARN("Device Type %s", m_deviceType.toString().c_str());
				LOGWARN("Device Present %d", m_isDevicePresent);
				LOGWARN("Active Source %d", m_isActiveSource);
				LOGWARN("PA Updated %d", m_isPAUpdated);
				LOGWARN("Version Updated %d", m_isVersionUpdated);
				LOGWARN("OSDName Updated %d", m_isOSDNameUpdated);
				LOGWARN("PowerStatus Updated %d", m_isPowerStatusUpdated);
				LOGWARN("VendorID Updated %d", m_isPowerStatusUpdated);
				LOGWARN("CEC Version : %s", m_cecVersion.toString().c_str());
				LOGWARN("Vendor ID : %s", m_vendorID.toString().c_str());
				LOGWARN("PhisicalAddress : %s", m_physicalAddr.toString().c_str());
				LOGWARN("OSDName : %s", m_osdName.toString().c_str());
				LOGWARN("Power Status : %s", m_powerStatus.toString().c_str());
				LOGWARN("Language : %s", m_currentLanguage.toString().c_str());
			}

			bool isAllUpdated() {
				if( !m_isPAUpdated 
					|| !m_isVersionUpdated 
					|| !m_isOSDNameUpdated
					|| !m_isVendorIDUpdated 
					|| !m_isPowerStatusUpdated 
					|| !m_isDeviceTypeUpdated ){
					return false;
				}
				return true;
			}

			void update( const DeviceType &deviceType ) {
				m_deviceType = deviceType;
				m_isDeviceTypeUpdated  = true;
			}

			void update( const PhysicalAddress &physical_addr ) {
				m_physicalAddr = physical_addr;
				m_isPAUpdated = true;
			}

			void update ( const VendorID &vendorId) {
				m_vendorID = vendorId;
				m_isVendorIDUpdated = true;
			}

			void update ( const Version &version ) {
				m_cecVersion = version;
				m_isVersionUpdated = true;
			}

			void update ( const OSDName    &osdName ) {
				m_osdName = osdName;
				m_isOSDNameUpdated = true;
			}

			void update ( const PowerStatus &status ) {
				m_powerStatus = status;
				m_isPowerStatusUpdated = true;
				m_lastPowerUpdateTime = std::chrono::system_clock::now();
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
        class HdmiCecSink : public AbstractPlugin {

		enum {
			POLL_THREAD_STATE_NONE,
			POLL_THREAD_STATE_IDLE,
			POLL_THREAD_STATE_POLL,	
			POLL_THREAD_STATE_PING,
			POLL_THREAD_STATE_INFO,
			POLL_THREAD_STATE_WAIT,
			POLL_THREAD_STATE_CLEAN,
			POLL_THREAD_STATE_UPDATE,
			POLL_THREAD_STATE_EXIT,
		};
        public:
            HdmiCecSink();
            virtual ~HdmiCecSink();
            static HdmiCecSink* _instance;
			CECDeviceParams deviceList[16];
			int m_currentActiveSource;
			void updateInActiveSource(const int logical_address, const InActiveSource &source );
			void updateActiveSource(const int logical_address, const ActiveSource &source );
			void updateTextViewOn(const int logicalAddress);
			void updateImageViewOn(const int logicalAddress);
			void removeDevice(const int logicalAddress);
			void addDevice(const int logicalAddress);
			void printDeviceList();
			void setActivePath();
			int m_numberOfDevices; /* Number of connected devices othethan own device */
        private:
            // We do not allow this plugin to be copied !!
            HdmiCecSink(const HdmiCecSink&) = delete;
            HdmiCecSink& operator=(const HdmiCecSink&) = delete;

            //Begin methods
            uint32_t setEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setOSDNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getOSDNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setVendorIdWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getVendorIdWrapper(const JsonObject& parameters, JsonObject& response);
			uint32_t printDeviceListWrapper(const JsonObject& parameters, JsonObject& response);
			uint32_t setActivePathWrapper(const JsonObject& parameters, JsonObject& response);
			uint32_t getDeviceListWrapper(const JsonObject& parameters, JsonObject& response);
			uint32_t getActiveSourceWrapper(const JsonObject& parameters, JsonObject& response);
			
            //End methods
            std::string logicalAddressDeviceType;
            bool cecSettingEnabled;
            bool cecOTPSettingEnabled;
            bool cecEnableStatus;
			bool m_isHdmiInConnected;
			uint8_t m_deviceType;
			int m_logicalAddressAllocated;
			std::thread m_pollThread;
			uint32_t m_pollThreadState;
			uint32_t m_pollNextState;
			uint32_t m_sleepTime;
            std::mutex m_pollMutex;
            Connection *smConnection;
			std::vector<uint8_t> m_connectedDevices;
            HdmiCecSinkProcessor *msgProcessor;
            HdmiCecSinkFrameListener *msgFrameListener;
            const void InitializeIARM();
            void DeinitializeIARM();
			void allocateLogicalAddress(int deviceType);
			void allocateLAforTV();
			void pingDevices(std::vector<int> &connected , std::vector<int> &disconnected);
			void CheckHdmiInState();
			void request(const int logicalAddress);
			int requestType(const int logicalAddress);
			int requestStatus(const int logicalAddress);
			void requestPowerStatus(const int logicalAddress);
			static void threadRun();
			void cecMonitoringThread();
            static void cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void pwrMgrModeChangeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void onCECDaemonInit();
            void cecStatusUpdated(void *evtStatus);
            void onHdmiHotPlug(int connectStatus);
			void onPowerStateON();
            bool loadSettings();
            void persistSettings(bool enableStatus);
            void persistOTPSettings(bool enableStatus);
            void persistOSDName(const char *name);
            void persistVendorId(unsigned int vendorID);
            void setEnabled(bool enabled);
            bool getEnabled();
            void CECEnable(void);
            void CECDisable(void);
            void getPhysicalAddress();
            void getLogicalAddress();
            void cecAddressesChanged(int changeStatus);
        };
	} // namespace Plugin
} // namespace WPEFramework




