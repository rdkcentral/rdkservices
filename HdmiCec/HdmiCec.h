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
        class HdmiCec : public AbstractPlugin, public FrameListener {
        private:

            // We do not allow this plugin to be copied !!
            HdmiCec(const HdmiCec&) = delete;
            HdmiCec& operator=(const HdmiCec&) = delete;

            //Begin methods
            uint32_t setEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getEnabledWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getCECAddressesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t sendMessageWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods


        public:
            HdmiCec();
            virtual ~HdmiCec();
            virtual void Deinitialize(PluginHost::IShell* service) override;

        public:
            static HdmiCec* _instance;
        private:
            std::string logicalAddressDeviceType;
            unsigned int logicalAddress;
            unsigned int physicalAddress;
            bool cecSettingEnabled;
            bool cecEnableStatus;
            Connection *smConnection;

            const void InitializeIARM();
            void DeinitializeIARM();
            static void cecMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void onCECDaemonInit();
            void cecStatusUpdated(void *evtStatus);
            void onHdmiHotPlug(int connectStatus);

            bool loadSettings();

            void persistSettings(bool enableStatus);
            void setEnabled(bool enabled);
            void CECEnable(void);
            void CECDisable(void);
            void getPhysicalAddress();
            void getLogicalAddress();
            bool getEnabled();
            void setName(std::string name);
            std::string getName();
            JsonObject getCECAddresses();

            uint16_t FromBase64String(const string& newValue, uint8_t object[], uint16_t& length, const TCHAR* ignoreList);
            void sendMessage(std::string message);
            void cecAddressesChanged(int changeStatus);

            void notify(const CECFrame &in) const;
            void onMessage(const char *message);

        };
	} // namespace Plugin
} // namespace WPEFramework


