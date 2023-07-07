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

        // Class to aid in 3-digit validation during pairing
        class threeDigits
        {
        public:
            threeDigits() { clear(); }
            void clear() { digit1 = digit2 = digit3 = -1; }
            int size()
            {
                int size = 0;
                if (digit1 > -1) size++;
                if (digit2 > -1) size++;
                if (digit3 > -1) size++;
                return size;
            }
            int digit1;
            int digit2;
            int digit3;
        };

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
            uint32_t startPairingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getNetStatusWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRDBManufacturersWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRDBModelsWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRCodesByAutoLookupWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getIRCodesByNamesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setIRCodeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t clearIRCodesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getLastKeypressSourceWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t configureWakeupKeysWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t initializeIRDBWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t findMyRemoteWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t factoryReset(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onStatus(ctrlm_iarm_RcuStatus_params_t* statusEvt);
            void onXRPairingStart(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits);
            void onXRValidationUpdate(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits);
            void onXRValidationComplete(int remoteId, char* remoteType, int bindingType, int validationStatus, bool bFromValidationUpdate);
            void onXRConfigurationComplete(int remoteId, char* remoteType, int bindingType, ctrlm_controller_status_t *status, int configurationStatus);
            void onPairingWindowTimeout(int validationStatus);
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
            static void btRemoteEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            static void rf4ceEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void btIarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void rf4ceIarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void rf4cePairingHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // BLE underlying private implementations for public wrapper methods
            bool bleStartPairing(unsigned int timeout, JsonObject& response);
            bool bleGetNetStatus(JsonObject& response);

            // Rf4ce underlying private implementations for public wrapper methods
            bool rf4ceStartPairing(unsigned int timeout, JsonObject& response);
            bool rf4ceGetNetStatus(JsonObject& response);

            // Generic underlying private implementations for public wrapper methods
            bool getIRDBManufacturers(int netType, string avDevType, string manufacturer, JsonObject& response);
            bool getIRDBModels(int netType, string avDevType, string manufacturer, string model, JsonObject& response);
            bool getIRCodesByAutoLookup(int netType, JsonObject& response);
            bool getIRCodesByNames(int netType, string avDevType, string manufacturer, string model, JsonObject& response);
            bool setIRCode(int netType, string avDevType, int remoteId, string code, JsonObject& response);
            bool clearIRCodes(int netType, int remoteId, JsonObject& response);
            bool getLastKeypressSource(JsonObject& keypressInfo, int netType);
            bool configureWakeupKeys(int netType, ctrlm_rcu_wakeup_config_t config, int *customList, int customListSize, JsonObject& response);
            bool initializeIRDB(int netType, JsonObject& response);
            bool findMyRemote(int netType, ctrlm_fmr_alarm_level_t level, JsonObject& response);

            // Local utility methods
            void setApiVersionNumber(uint32_t apiVersionNumber);

            // BLE methods
            const char *ctrlm_ble_state_str(ctrlm_ble_state_t status);
            const char *ctrlm_ir_prog_state_str(ctrlm_ir_state_t status);

            ctrlm_network_id_t getBleNetworkID();

            // Rf4ce methods
            ctrlm_network_id_t getRf4ceNetworkID();
            bool getRf4ceStbData(JsonObject& status);
            bool getRf4ceNetworkStatus(ctrlm_main_iarm_call_network_status_t&  netStatus);
            bool getRf4ceBindRemote(JsonObject& remoteInfo, ctrlm_rcu_iarm_call_controller_status_t& ctrlStatus);
            bool getRf4ceRemoteData(JsonObject& remoteInfo, int controller_id, ctrlm_controller_status_t *status);
            bool getAllRf4ceBindRemotes();
            void rf4ceGetIeeeMacStr(unsigned long long ieee_address_long_long, char *ieee_address_str, int ieee_address_str_size);
            int numericCtrlm2Int(ctrlm_key_code_t ctrlm_key);
            char* rf4ceGetRemoteModel(char *remoteType);

            // Generic methods
            const char *networkTypeStr(int network_type);
            std::string                 wakeupConfigToString(ctrlm_rcu_wakeup_config_t config);
            ctrlm_rcu_wakeup_config_t   wakeupConfigFromString(std::string configStr);
            JsonArray                   wakeupCustomListToArray(int *list, int listSize);
            ctrlm_fmr_alarm_level_t     findMyRemoteLevelFromString(std::string configStr);

        public:
            static RemoteControl* _instance;
        private:
            // Generic members
            uint32_t   m_apiVersionNumber;
            bool       m_hasOwnProcess;
            JsonArray  m_netTypesArray;

            JsonObject  m_rf4ceRemoteInfo[CTRLM_MAIN_MAX_BOUND_CONTROLLERS];
            int         m_rf4ceNumOfBindRemotes;

            // Used to remember the "golden" and "entered" digits, during 3-digit manual pairing validation
            threeDigits m_goldenValDigits;
            threeDigits m_enteredValDigits;

        };
	} // namespace Plugin
} // namespace WPEFramework
