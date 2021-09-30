/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
* Copyright 2021 Liberty Global Service B.V.
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

#include <atomic>
#include <map>
#include <mutex>
#include <stdint.h>
#include "ccec/FrameListener.hpp"
#include "ccec/Connection.hpp"

#include "libIBus.h"

#undef Assert // this define from Connection.hpp conflicts with WPEFramework

#include "Module.h"
#include "utils.h"
#include "AbstractPlugin.h"

namespace WPEFramework {

    namespace Plugin {

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
        class LgiHdmiCec : public AbstractPlugin, public FrameListener {
        private:

            // We do not allow this plugin to be copied !!
            LgiHdmiCec(const LgiHdmiCec&) = delete;
            LgiHdmiCec& operator=(const LgiHdmiCec&) = delete;

            //Begin methods
            uint32_t setEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getCECAddressesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t sendMessageWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t enableOneTouchViewWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t triggerActionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setPingIntervalWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getConnectedDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setNameWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setOneTouchViewPolicyWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods
        public:
            LgiHdmiCec();
            virtual ~LgiHdmiCec();
            virtual void Deinitialize(PluginHost::IShell* service) override;

        public:
            static LgiHdmiCec* _instance;
        private:
            std::string logicalAddressDeviceType;
            unsigned int logicalAddress;
            unsigned int physicalAddress;
            std::atomic_bool cecSettingEnabled;
            std::atomic_bool cecEnableStatus;
            Connection *smConnection;

            const void InitializeIARM();
            void DeinitializeIARM();
            static void cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void cecHostDeviceStatusChangedEventHandler(const char* owner_str, IARM_EventId_t eventId, void* data_ptr, size_t len);
            static void cecHostDeviceStatusUpdateEndEventHandler(const char* owner_str, IARM_EventId_t eventId, void* data_ptr, size_t len);
            void onCECDaemonInit();
            void cecStatusUpdated(void *evtStatus);
            void onHdmiHotPlug(int connectStatus);
            void onDeviceStatusChanged(IARM_EventId_t eventId, const void* data_ptr, size_t len);
            void onDeviceStatusUpdateEnd(IARM_EventId_t eventId, const void* data_ptr, size_t len);
            void onDevicesChanged();

            void getConnectedDevices(JsonArray &deviceList);

            bool setChangedDeviceOsdName(const char* name, int logical_address);
            bool setChangedDeviceVendorId(uint32_t vendor_id, int logical_address);
            bool setChangedDeviceConnectedState(int connected, int logical_address);
            bool setChangedDevicePowerState(int power_state, int logical_address);
            bool setChangedDeviceSystemAudioMode(int system_audio_mode);

            bool loadSettings();

            void persistSettings(bool enableStatus);
            void setEnabled(bool enabled);
            void CECEnable(void);
            void CECDisable(void);
            bool getPhysicalAddress();
            bool getLogicalAddress();
            void readAddresses();
            bool getEnabled();
            std::string getName();
            JsonObject getCECAddresses();

            uint16_t FromBase64String(const string& newValue, uint8_t object[], uint16_t& length, const TCHAR* ignoreList);
            void sendMessage(std::string message);
            void cecAddressesChanged(int changeStatus);

            void notify(const CECFrame &in) const;
            void onMessage(const char *message);

            std::atomic<int> m_scan_id;
            std::atomic_bool m_updated;
            std::atomic_bool m_rescan_in_progress;
            std::atomic_bool m_system_audio_mode;

            typedef struct device_s
            {
                std::string physical_address;
                std::string osdName;
                std::string vendor_id;
                bool connected = false;
                bool power_state = true;
            } device_t;

            typedef std::map<std::string, device_t> m_devices_map_t;
            m_devices_map_t m_devices;
            m_devices_map_t m_scan_devices;
            std::mutex m_mutex;
        };
    } // namespace Plugin
} // namespace WPEFramework


