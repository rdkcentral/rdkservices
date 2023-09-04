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

#include "RemoteControl.h"
#include "libIBusDaemon.h"
#include "UtilsUnused.h"
#include "UtilsCStr.h"
#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#include "UtilsString.h"

#include <exception>

#define IARM_FACTORY_RESET_TIMEOUT  (15 * 1000)  // 15 seconds, in milliseconds
#define IARM_IRDB_CALLS_TIMEOUT     (10 * 1000)  // 10 seconds, in milliseconds

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 3
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework {

    namespace {
        static Plugin::Metadata<Plugin::RemoteControl> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin {
 
        SERVICE_REGISTRATION(RemoteControl, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        RemoteControl* RemoteControl::_instance = nullptr;

        RemoteControl::RemoteControl()
            : PluginHost::JSONRPC()
            , m_apiVersionNumber((uint32_t)-1)   /* default max uint32_t so everything gets enabled */    //TODO(MROLLINS) Can't we access this from jsonrpc interface?
        {
            LOGINFO("ctor");
            RemoteControl::_instance = this;

            Register("getApiVersionNumber",    &RemoteControl::getApiVersionNumber,   this);
            Register("startPairing",           &RemoteControl::startPairing,          this);
            Register("getNetStatus",           &RemoteControl::getNetStatus,          this);
            Register("getIRDBManufacturers",   &RemoteControl::getIRDBManufacturers,  this);
            Register("getIRDBModels",          &RemoteControl::getIRDBModels,         this);
            Register("getIRCodesByAutoLookup", &RemoteControl::getIRCodesByAutoLookup,this);
            Register("getIRCodesByNames",      &RemoteControl::getIRCodesByNames,     this);
            Register("setIRCode",              &RemoteControl::setIRCode,             this);
            Register("clearIRCodes",           &RemoteControl::clearIRCodes,          this);
            Register("getLastKeypressSource",  &RemoteControl::getLastKeypressSource, this);
            Register("configureWakeupKeys",    &RemoteControl::configureWakeupKeys,   this);
            Register("initializeIRDB",         &RemoteControl::initializeIRDB,        this);
            Register("findMyRemote",           &RemoteControl::findMyRemote,          this);
            Register("factoryReset",           &RemoteControl::factoryReset,          this);

            setApiVersionNumber(1);
        }

        RemoteControl::~RemoteControl()
        {
            //LOGINFO("dtor");
        }

        const string RemoteControl::Initialize(PluginHost::IShell* /* service */)
        {
            InitializeIARM();
            // On success return empty, to indicate there is no error text.
            return (string());
        }

        void RemoteControl::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            RemoteControl::_instance = nullptr;
        }

        void RemoteControl::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                m_hasOwnProcess = true;
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME,  CTRLM_RCU_IARM_EVENT_RCU_STATUS,  remoteEventHandler) );
                // Register for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT, remoteEventHandler) );
            }
            else
                m_hasOwnProcess = false;
        }

        //TODO(MROLLINS) - we need to install crash handler to ensure DeinitializeIARM gets called
        void RemoteControl::DeinitializeIARM()
        {
            if (m_hasOwnProcess)
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME,  CTRLM_RCU_IARM_EVENT_RCU_STATUS,  remoteEventHandler) );
                // Remove handlers for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, remoteEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT, remoteEventHandler) );

                IARM_CHECK( IARM_Bus_Disconnect() );
                IARM_CHECK( IARM_Bus_Term() );
                m_hasOwnProcess = false;
            }
        }

        void RemoteControl::remoteEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (RemoteControl::_instance)
                RemoteControl::_instance->iarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle btremote IARM events without a RemoteControl plugin instance!");
        }

        //bt handlers
        void RemoteControl::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                LOGERR("ERROR - unexpected event: owner %s, eventId: %d, data: %p, size: %d.",
                       owner, (int)eventId, data, len);
                return;
            }

            if ((data == NULL) || (len == 0))
            {
                LOGERR("ERROR - event with NO DATA: eventId: %d, data: %p, size: %d.", (int)eventId, data, len);
                return;
            }
            ctrlm_main_iarm_event_json_t *eventData = static_cast<ctrlm_main_iarm_event_json_t *>(data);

            switch(eventId) {
                case CTRLM_RCU_IARM_EVENT_RCU_STATUS:
                case CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN:
                case CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS:
                case CTRLM_RCU_IARM_EVENT_VALIDATION_END:
                case CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE:
                case CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT:
                    LOGWARN("Got CTRLM_RCU_IARM_EVENT event.");
                    onStatus(eventData);
                    break;
                default:
                    LOGERR("ERROR - unexpected ctrlm event: eventId: %d, data: %p, size: %d.",
                           (int)eventId, data, len);
                    break;
            }
        }  // End iarmEventHandler()

        //Begin methods
        uint32_t RemoteControl::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t RemoteControl::startPairing(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_START_PAIRING, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_START_PAIRING Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("START PAIRING call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_START_PAIRING returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getNetStatus(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("GET RCU STATUS call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRDBManufacturers(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS, (void *)call, totalsize, IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("IRDB MANUFACTURERS call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRDBModels(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_MODELS, (void *)call, totalsize, IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MODELS Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("IRDB MODELS call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MODELS returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRCodesByAutoLookup(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP, (void *)call, totalsize, IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("IRDB AUTO LOOKUP call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRCodesByNames(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_CODES, (void *)call, totalsize, IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CODES Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("GET IR CODES call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CODES returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::setIRCode(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_SET_CODE, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_SET_CODE Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("SET IR CODES call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_SET_CODE returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::clearIRCodes(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("CLEAR IR CODES call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getLastKeypressSource(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_LAST_KEYPRESS_GET, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_LAST_KEYPRESS_GET Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("GET LAST KEYPRESS call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_LAST_KEYPRESS_GET returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::configureWakeupKeys(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("WRITE RCU WAKEUP CONFIG call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::initializeIRDB(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_INITIALIZE, (void *)call, totalsize, IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_INITIALIZE Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("INITIALIZE IRDB call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_INITIALIZE returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::findMyRemote(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE, (void *)call, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("FIND MY REMOTE call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE returned FAILURE!");

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::factoryReset(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            ctrlm_main_iarm_call_json_t *call = NULL;
            IARM_Result_t                res;
            string                       jsonParams;
            bool                         bSuccess = false;
            size_t                       totalsize = 0;

            parameters.ToString(jsonParams);
            totalsize = sizeof(ctrlm_main_iarm_call_json_t) + jsonParams.size() + 1;
            call      = (ctrlm_main_iarm_call_json_t*)calloc(1, totalsize);

            if (call == NULL)
            {
                LOGERR("ERROR - Cannot allocate IARM structure - size: %u.", (unsigned)totalsize);
                bSuccess = false;
                returnResponse(bSuccess);
            }

            call->api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            size_t len = jsonParams.copy(call->payload, jsonParams.size());
            call->payload[len] = '\0';

            // The default timeout for IARM calls is 5 seconds, but this call could take longer and we need to ensure the remotes receive
            // the message before the larger system factory reset operation continues.  Therefore, make this timeout longer.
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_FACTORY_RESET, (void *)call, totalsize, IARM_FACTORY_RESET_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FACTORY_RESET Bus Call FAILED, res: %d.", (int)res);
                bSuccess = false;
                free(call);
                returnResponse(bSuccess);
            }

            JsonObject result;
            result.FromString(call->result);
            bSuccess = result["success"].Boolean();
            response = result;
            free(call);

            if (bSuccess)
                LOGINFO("FACTORY RESET call SUCCESS!");
            else
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FACTORY_RESET returned FAILURE!");

            returnResponse(bSuccess);
        }
        //End methods

        //Begin ble events
        void RemoteControl::onStatus(ctrlm_main_iarm_event_json_t* eventData)
        {
            JsonObject params;

            params.FromString(eventData->payload);

            sendNotify("onStatus", params);
        }
        //End ble events

        //Begin local private utility methods
        void RemoteControl::setApiVersionNumber(unsigned int apiVersionNumber)
        {
            LOGINFO("setting version: %d", (int)apiVersionNumber);
            m_apiVersionNumber = apiVersionNumber;
        }
        //End local private utility methods
    } // namespace Plugin
} // namespace WPEFramework

