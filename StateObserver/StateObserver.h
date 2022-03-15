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

 /**
 * @file StateObserver.h
 * @brief Thunder Plugin based Implementation for StateObserver service API's (RDK-25848).
 */

 /**
  @mainpage StateObserver
  */

#pragma once

#ifndef STATEOBSERVER_H
#define STATEOBSERVER_H
#include <cjson/cJSON.h>

#include "Module.h"
#include "libIBus.h"
#include "utils.h"
#include "utils.h"
#include "AbstractPlugin.h"

//State Observer Properties
extern const string SYSTEM_EXIT_OK;
extern const string SYSTEM_CHANNEL_MAP;
extern const string SYSTEM_CARD_DISCONNECTED;
extern const string SYSTEM_TUNE_READY;
extern const string SYSTEM_CMAC;
extern const string SYSTEM_MOTO_ENTITLEMENT;
extern const string SYSTEM_MOTO_HRV_RX;
extern const string SYSTEM_DAC_INIT_TIMESTAMP;
extern const string SYSTEM_CARD_CISCO_STATUS;
extern const string SYSTEM_VIDEO_PRESENTING;
extern const string SYSTEM_HDMI_OUT;
extern const string SYSTEM_HDCP_ENABLED;
extern const string SYSTEM_HDMI_EDID_READ;
extern const string SYSTEM_FIRMWARE_DWNLD;
extern const string SYSTEM_TIME_SOURCE;
extern const string SYSTEM_TIME_ZONE;
extern const string SYSTEM_CA_SYSTEM;
extern const string SYSTEM_ESTB_IP;
extern const string SYSTEM_ECM_IP;
extern const string SYSTEM_ECM_MAC;
extern const string SYSTEM_LAN_IP;
extern const string SYSTEM_MOCA;
extern const string SYSTEM_DOCSIS;
extern const string SYSTEM_DSG_BROADCAST_CHANNEL;
extern const string SYSTEM_DSG_CA_TUNNEL;
extern const string SYSTEM_CABLE_CARD;
extern const string SYSTEM_CABLE_CARD_DWNLD;
extern const string SYSTEM_CVR_SUBSYSTEM;
extern const string SYSTEM_DOWNLOAD;
extern const string SYSTEM_VOD_AD;
extern const string SYSTEM_CARD_SERIAL_NO;
extern const string SYSTEM_STB_SERIAL_NO;
extern const string SYSTEM_RF_CONNECTED;
extern const string SYSTEM_IP_MODE;

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

		/**
		 * @brief This class provides the interface to get the value of various device properties
		 * and set up event listeners to be notified when the state of the device changes.
		 *
 		 */

		class StateObserver : public AbstractPlugin {
		public:
			static const string STATE_OBSERVER_PLUGIN_NAME;
			static const string EVT_STATE_OBSERVER_PROPERTY_CHANGED;


		private:
			typedef Core::JSON::String JString;
			typedef Core::JSON::ArrayType<JString> JStringArray;
			typedef Core::JSON::Boolean JBool;

			// We do not allow this plugin to be copied !!
			StateObserver(const StateObserver&) = delete;
			StateObserver& operator=(const StateObserver&) = delete;

			//Begin methods
			uint32_t getValues(const JsonObject& parameters, JsonObject& response);
			uint32_t registerListeners(const JsonObject& parameters, JsonObject& response);
			uint32_t unregisterListeners(const JsonObject& parameters, JsonObject& response);
			uint32_t setApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response);
			uint32_t getApiVersionNumberWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getRegisteredPropertyNames(const JsonObject &parameters, JsonObject &response);
			uint32_t getNameWrapper(const JsonObject& parameters, JsonObject& response);
			void getVal(std::vector<string> pname,JsonObject& response);
			void InitializeIARM();
			void DeinitializeIARM();
			//End methods


			virtual string getName();
			virtual unsigned int getApiVersionNumber();
			void setApiVersionNumber(unsigned int apiVersionNumber);

			//Begin events
			//End events
		public:
			StateObserver();
			virtual ~StateObserver();
			//IPlugin methods
			virtual const string Initialize(PluginHost::IShell* service) override;
			virtual void Deinitialize(PluginHost::IShell* service) override;
			static void onReportStateObserverEvents(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
			void notify(std::string eventname, JsonObject& param);
			void setProp(JsonObject& params,std::string propName,int state,int error);

		public:
			static StateObserver* _instance;
		private:
			uint32_t m_apiVersionNumber;
		};

	} // namespace Plugin
} // namespace WPEFramework


#endif //STATEOBSERVER_H
