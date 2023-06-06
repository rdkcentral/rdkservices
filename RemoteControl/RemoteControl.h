/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 RDK Management
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

#include "Module.h"
#include "libIBus.h"

#include "ctrlm_ipc.h"
#include "ctrlm_ipc_rcu.h"
#include "ctrlm_ipc_ble.h"

#define IARM_REMOTECONTROL_PLUGIN_NAME    "Remote_Control"


namespace WPEFramework {

    namespace Plugin {

        class RemoteControl;  // Forward declaration

		// This is a server for a JSONRPC communication channel.
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other method to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        // Note that most of the above is now inherited from the AbstractPlugin class.
        class RemoteControl : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::ArrayType<JsonObject> JObjectArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            RemoteControl(const RemoteControl&) = delete;
            RemoteControl& operator=(const RemoteControl&) = delete;

            //Begin methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
            uint32_t startPairing(const JsonObject& parameters, JsonObject& response);
            uint32_t getNetStatus(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRDBManufacturers(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRDBModels(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRCodesByAutoLookup(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRCodesByNames(const JsonObject& parameters, JsonObject& response);
            uint32_t setIRCode(const JsonObject& parameters, JsonObject& response);
            uint32_t clearIRCodes(const JsonObject& parameters, JsonObject& response);
            uint32_t getLastKeypressSource(const JsonObject& parameters, JsonObject& response);
            uint32_t configureWakeupKeys(const JsonObject& parameters, JsonObject& response);
            uint32_t initializeIRDB(const JsonObject& parameters, JsonObject& response);
            uint32_t findMyRemote(const JsonObject& parameters, JsonObject& response);
            uint32_t factoryReset(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onStatus(ctrlm_main_iarm_event_json_t* eventData);
            //End events

        public:
            RemoteControl();
            virtual ~RemoteControl();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(RemoteControl)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            void InitializeIARM();
            void DeinitializeIARM();
            // Handlers for ControlMgr BT Remote events
            static void remoteEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Local utility methods
            void setApiVersionNumber(uint32_t apiVersionNumber);

        public:
            static RemoteControl* _instance;
        private:
            // Generic members
            uint32_t   m_apiVersionNumber;
            bool       m_hasOwnProcess;
        };
	} // namespace Plugin
} // namespace WPEFramework
