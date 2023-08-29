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

#include "libIBus.h"

#include "Module.h"
#include "dsTypes.h"

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
        class HdmiInput : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:

            // We do not allow this plugin to be copied !!
            HdmiInput(const HdmiInput&) = delete;
            HdmiInput& operator=(const HdmiInput&) = delete;

            void InitializeIARM();
            void DeinitializeIARM();

            //Begin methods
            uint32_t getHDMIInputDevicesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t writeEDIDWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t readEDIDWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getRawHDMISPDWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getHDMISPDWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setEdidVersionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getEdidVersionWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t startHdmiInput(const JsonObject& parameters, JsonObject& response);
            uint32_t stopHdmiInput(const JsonObject& parameters, JsonObject& response);

            uint32_t setVideoRectangleWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getSupportedGameFeatures(const JsonObject& parameters, JsonObject& response);
            uint32_t getHdmiGameFeatureStatusWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            JsonArray getHDMIInputDevices();
            void writeEDID(int deviceId, std::string message);
            std::string readEDID(int iPort);
            std::string getRawHDMISPD(int iPort);
            std::string getHDMISPD(int iPort);
            int setEdidVersion(int iPort, int iEdidVer);
            int getEdidVersion(int iPort);
            bool getHdmiALLMStatus(int iPort);

            bool setVideoRectangle(int x, int y, int width, int height);

            void hdmiInputHotplug( int input , int connect);
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

	    void hdmiInputSignalChange( int port , int signalStatus);
            static void dsHdmiSignalStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            void hdmiInputStatusChange( int port , bool isPresented);
	    static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

	    void hdmiInputVideoModeUpdate( int port , dsVideoPortResolution_t resolution);
	    static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            void hdmiInputALLMChange( int port , bool allmMode);
            static void dsHdmiGameFeatureStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

	    void hdmiInputAviContentTypeChange(int port, int content_type);
            static void dsHdmiAviContentTypeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
        public:
            HdmiInput();
            virtual ~HdmiInput();
            virtual const string Initialize(PluginHost::IShell* shell) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            void terminate();

            BEGIN_INTERFACE_MAP(HdmiInput)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        public:
            static HdmiInput* _instance;
        };
	} // namespace Plugin
} // namespace WPEFramework
