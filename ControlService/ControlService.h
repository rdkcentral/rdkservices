/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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
#include "ctrlm_ipc_key_codes.h"

#include <mutex>

#define IARM_CONTROLSERVICE_PLUGIN_NAME    "Control_Service"

// Substitute for the old AbstractService Status enumeration
typedef enum {
    STATUS_OK           = 0,
    STATUS_FAILURE,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_STATE,
    STATUS_METHOD_NOT_FOUND,
    STATUS_FMR_NOT_SUPPORTED
} StatusCode;

typedef enum
{
    RF4CE_CHIP_CONNECTIVITY_NOT_CONNECTED,
    RF4CE_CHIP_CONNECTIVITY_CONNECTED,
    RF4CE_CHIP_CONNECTIVITY_NOT_SUPPORTED,
    RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR,
} eCheckRf4ceChipConnectivity;

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

        class ControlService : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::ArrayType<JsonObject> JObjectArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            ControlService(const ControlService&) = delete;
            ControlService& operator=(const ControlService&) = delete;

            //Begin methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);
            uint32_t getQuirks(const JsonObject& parameters, JsonObject& response);

            uint32_t getAllRemoteDataWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getSingleRemoteDataWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getLastKeypressSourceWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getLastPairedRemoteDataWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setValuesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getValuesWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t startPairingModeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t endPairingModeWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t findLastUsedRemoteWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t canFindMyRemoteWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t checkRf4ceChipConnectivityWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onControl(int remoteId, int keyCode, string& source, string& type, string& data);
            void onXRPairingStart(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits);
            void onXRValidationUpdate(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits);
            void onXRValidationComplete(int remoteId, char* remoteType, int bindingType, int validationStatus);
            void onXRConfigurationComplete(int remoteId, char* remoteType, int bindingType, int configurationStatus);
            //End events

        public:
            ControlService();
            virtual ~ControlService();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(ControlService)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP
        private:
            void InitializeIARM();
            void DeinitializeIARM();
            // Handlers for IARM events
            static void controlEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void irmgrHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void ctrlmHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void pairingHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Underlying private implementations for public wrapper methods
            StatusCode getAllRemoteData(JsonObject& response);
            StatusCode getSingleRemoteData(JsonObject& remoteInfo, int remoteId);
            StatusCode getLastPairedRemoteData(JsonObject& remoteInfo);
            StatusCode getLastKeypressSource(JsonObject& keypressInfo);
            StatusCode setValues(const JsonObject& parameters);
            StatusCode getValues(JsonObject& settings);
            StatusCode startPairingMode(int mode, int restrictions);
            StatusCode endPairingMode(int& bindStatus);
            StatusCode findMyRemote(int timeOutPeriod, bool bOnlyLastUsed);
            bool       canFindMyRemote();
            eCheckRf4ceChipConnectivity checkRf4ceChipConnectivity();

            // Local utility methods
            void setApiVersionNumber(uint32_t apiVersionNumber);
            int numericCtrlm2Int(ctrlm_key_code_t ctrlm_key);

            char* getRemoteModel(char *remoteType);
            char* getRemoteModelVersion(char *remoteType);
            const char* getPairingType(ctrlm_rcu_binding_type_t pairingType);

            bool getIrRemoteUsage(ctrlm_main_iarm_call_ir_remote_usage_t&  irRemoteUsage);

            bool getRf4ceNetworkId(ctrlm_network_id_t& rf4ceId);
            bool getRf4ceNetworkStatus(ctrlm_main_iarm_call_network_status_t&  netStatus);

            bool getRf4ceStbData(JsonObject& stbData);
            bool getRf4ceBindRemote(JsonObject& remoteInfo, ctrlm_rcu_iarm_call_controller_status_t& ctrlStatus);
            bool getAllRf4ceBindRemotes(void);
            bool getLastPairedRf4ceBindRemote(JsonObject& remoteInfo);

        public:
            static ControlService* _instance;
        private:
            uint32_t    m_apiVersionNumber;

            JsonObject  m_remoteInfo[CTRLM_MAIN_MAX_BOUND_CONTROLLERS];
            int         m_numOfBindRemotes;

            std::mutex  m_callMutex;

            // Used to remember the "golden" and "entered" digits, during 3-digit manual pairing validation
            threeDigits m_goldenValDigits;
            threeDigits m_enteredValDigits;
        };
	} // namespace Plugin
} // namespace WPEFramework
