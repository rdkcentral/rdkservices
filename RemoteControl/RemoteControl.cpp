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

#include "irMgr.h"

// These could probably be improved upon, if we knew more...
#define MAC_MIN     (0x0000000FFFFF)
#define MAC_MAX     (0xF0FFFFFFFFFF)

#define RF4CE_IEEE_MAC_ADDRESS_STR_MAX    24

#define IARM_FACTORY_RESET_TIMEOUT  (15 * 1000)  // 15 seconds, in milliseconds
#define IARM_IRDB_CALLS_TIMEOUT     (10 * 1000)  // 10 seconds, in milliseconds


// Local types and definitions
// Pairing validation status
typedef enum
{
    VALIDATION_SUCCESS      = 0,
    VALIDATION_TIMEOUT      = 1,
    VALIDATION_FAILURE      = 2,
    VALIDATION_ABORT        = 3,
    VALIDATION_WRONG_CODE   = 4
} eValidationStatusType;

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

            Register("getApiVersionNumber",    &RemoteControl::getApiVersionNumber,          this);

            Register("startPairing",           &RemoteControl::startPairingWrapper,          this);
            Register("getNetStatus",           &RemoteControl::getNetStatusWrapper,          this);
            Register("getIRDBManufacturers",   &RemoteControl::getIRDBManufacturersWrapper,  this);
            Register("getIRDBModels",          &RemoteControl::getIRDBModelsWrapper,         this);
            Register("getIRCodesByAutoLookup", &RemoteControl::getIRCodesByAutoLookupWrapper,this);
            Register("getIRCodesByNames",      &RemoteControl::getIRCodesByNamesWrapper,     this);
            Register("setIRCode",              &RemoteControl::setIRCodeWrapper,             this);
            Register("clearIRCodes",           &RemoteControl::clearIRCodesWrapper,          this);
            Register("getLastKeypressSource",  &RemoteControl::getLastKeypressSourceWrapper, this);
            Register("configureWakeupKeys",    &RemoteControl::configureWakeupKeysWrapper,   this);
            Register("initializeIRDB",         &RemoteControl::initializeIRDBWrapper,        this);
            Register("findMyRemote",           &RemoteControl::findMyRemoteWrapper,          this);
            Register("factoryReset",           &RemoteControl::factoryReset,                 this);

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
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME,  CTRLM_RCU_IARM_EVENT_RCU_STATUS,  btRemoteEventHandler) );
                // Register for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT, rf4ceEventHandler) );
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
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME,  CTRLM_RCU_IARM_EVENT_RCU_STATUS,  btRemoteEventHandler) );
                // Remove handlers for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, rf4ceEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT, rf4ceEventHandler) );

                IARM_CHECK( IARM_Bus_Disconnect() );
                IARM_CHECK( IARM_Bus_Term() );
                m_hasOwnProcess = false;
            }
        }

        void RemoteControl::btRemoteEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (RemoteControl::_instance)
                RemoteControl::_instance->btIarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle btremote IARM events without a RemoteControl plugin instance!");
        }

        void RemoteControl::rf4ceEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (RemoteControl::_instance)
                RemoteControl::_instance->rf4ceIarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle rf4ce IARM events without a RemoteControl plugin instance!");
        }


        //bt handlers
        void RemoteControl::btIarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            LOGINFO("ctrlm IARM BLE API revision is %d.", CTRLM_BLE_IARM_BUS_API_REVISION);
            if (!strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                ctrlm_iarm_RcuStatus_params_t*   statusEvt = NULL;

                if ((data == NULL) || (len == 0))
                {
                    LOGERR("ERROR - event with NO DATA: eventId: %d, data: %p, size: %d.", (int)eventId, data, len);
                    return;
                }

                switch(eventId) {
                    case CTRLM_RCU_IARM_EVENT_RCU_STATUS:
                        statusEvt = (ctrlm_iarm_RcuStatus_params_t*)data;
                        LOGWARN("Got CTRLM_RCU_IARM_EVENT_RCU_STATUS event.");
                        if (CTRLM_BLE_IARM_BUS_API_REVISION == statusEvt->api_revision)
                        {
                            onStatus(statusEvt);
                        }
                        else
                        {
                            LOGERR("ERROR - IARM RCU_STATUS_DATA with wrong ctrlm BLE API revision - should be %d, event is %d.",
                                    CTRLM_BLE_IARM_BUS_API_REVISION, (int)statusEvt->api_revision);
                        }
                    break;

                    default:
                        LOGERR("ERROR - unexpected ctrlm event: eventId: %d, data: %p, size: %d.",
                               (int)eventId, data, len);
                    break;
                }
            }
            else
            {
                LOGERR("ERROR - unexpected event: owner %s, eventId: %d, data: %p, size: %d.",
                       owner, (int)eventId, data, len);
            }
        }  // End iarmEventHandler()

        //rf4ce handlers
        void RemoteControl::rf4ceIarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (!strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                if ((eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_END) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT))
                {
                    rf4cePairingHandler(owner, eventId, data, len);
                }
                else
                {
                    LOGWARN("Ignoring unexpected ControlMgr event - eventId: %d!!", eventId);
                }
            }
            else
            {
                LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
            }
        }  // End iarmEventHandler()

        void RemoteControl::rf4cePairingHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN)
            {
                LOGINFO("Got a controlMgr VALIDATION_BEGIN event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_validation_begin_t *valBegin = (ctrlm_rcu_iarm_event_validation_begin_t*)data;
                    if (valBegin->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        LOGINFO("VALIDATION_BEGIN - network_id: %d, network_type: %d, controller_id: %d, controller_type: %s, binding_type: %d, validation_type: %d.\n",
                                valBegin->network_id, valBegin->network_type, valBegin->controller_id, valBegin->controller_type,
                                valBegin->binding_type, valBegin->validation_type);
                        LOGINFO("VALIDATION_BEGIN - validation_keys as an int array: 0x%02x, 0x%02x, 0x%02x.\n",
                                valBegin->validation_keys[0], valBegin->validation_keys[1], valBegin->validation_keys[2]);

                        if (valBegin->binding_type == CTRLM_RCU_BINDING_TYPE_INTERACTIVE)
                        {
                            // 3-digit manual pairing begins
                            m_enteredValDigits.clear();
                            m_goldenValDigits.digit1 = numericCtrlm2Int(valBegin->validation_keys[0]);
                            m_goldenValDigits.digit2 = numericCtrlm2Int(valBegin->validation_keys[1]);
                            m_goldenValDigits.digit3 = numericCtrlm2Int(valBegin->validation_keys[2]);

                            LOGINFO("VALIDATION_BEGIN manual 3-digit pairing - code: %d,%d,%d.",
                                    m_goldenValDigits.digit1, m_goldenValDigits.digit2, m_goldenValDigits.digit3);

                            onXRPairingStart((int)(valBegin->controller_id), valBegin->controller_type, (int)(valBegin->binding_type), m_goldenValDigits);
                        }
                        else
                        {
                            // We don't know what to do with a validation begin if it isn't 3-digit manual pairing!
                            LOGWARN("VALIDATION_BEGIN without BINDING_TYPE_INTERACTIVE - no XRE event sent!!");
                        }
                    }
                    else
                    {
                        LOGERR("Wrong VALIDATION_BEGIN ctrlm API revision - expected %d, event is %d!!",
                               CTRLM_RCU_IARM_BUS_API_REVISION, valBegin->api_revision);
                    }
                }
                else
                {
                    LOGERR("VALIDATION_BEGIN data is NULL!");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS)
            {
                LOGINFO("Got a controlMgr VALIDATION_KEY_PRESS event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_key_press_t *valKey = (ctrlm_rcu_iarm_event_key_press_t*)data;
                    bool bIsDigitCorrect = false;
                    if (valKey->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        LOGINFO("VALIDATION_KEY_PRESS - network_id: %d, network_type: %d, controller_id: %d, controller_type: %s, "
                                "binding_type: %d, key_status: %d, key_code: 0x%02X.\n",
                                valKey->network_id, valKey->network_type, valKey->controller_id, valKey->controller_type,
                                valKey->binding_type, valKey->key_status, valKey->key_code);

                        if (valKey->key_status == CTRLM_KEY_STATUS_DOWN)
                        {
                            // Build what was entered for codes as they arrive
                            int digit = numericCtrlm2Int(valKey->key_code);
                            LOGINFO("VALIDATION_KEY_PRESS - entered number %d.", digit);
                            if (m_enteredValDigits.size() == 0)
                            {
                                m_enteredValDigits.digit1 = digit;
                                // Test for the correct 1st digit
                                if (m_goldenValDigits.digit1 == m_enteredValDigits.digit1)
                                {
                                    bIsDigitCorrect = true;
                                }
                            }
                            else if (m_enteredValDigits.size() == 1)
                            {
                                m_enteredValDigits.digit2 = digit;
                                // Test for the correct 2nd digit
                                if (m_goldenValDigits.digit2 == m_enteredValDigits.digit2)
                                {
                                    bIsDigitCorrect = true;
                                }
                            }
                            else if (m_enteredValDigits.size() == 2)
                            {
                                m_enteredValDigits.digit3 = digit;
                                // Test for the correct 3rd digit
                                if (m_goldenValDigits.digit3 == m_enteredValDigits.digit3)
                                {
                                    bIsDigitCorrect = true;
                                }
                            }

                            // If we got an incorrect digit...
                            if (!bIsDigitCorrect)
                            {
                                bool bFromValidationUpdate = true;
                                LOGINFO("Sending onXRValidationComplete event from VALIDATION_KEY_PRESS - wrong digit entered!");
                                m_enteredValDigits.clear();

                                // Send the Validation Complete (with a WRONG_CODE error) from here, because controlMgr won't send VALIDATION_END in this case
                                onXRValidationComplete((int)(valKey->controller_id), valKey->controller_type,
                                                       (int)(valKey->binding_type), (int)VALIDATION_WRONG_CODE, bFromValidationUpdate);
                            }
                            else if (m_enteredValDigits.size() <= 3)
                            {
                                LOGINFO("Sending normal onXRValidationUpdate event from VALIDATION_KEY_PRESS");
                                // Send normal 3-digit update event
                                onXRValidationUpdate((int)(valKey->controller_id), valKey->controller_type, (int)(valKey->binding_type), m_enteredValDigits);
                            }
                            else
                            {
                                LOGERR("Keypress ERROR: More than 3 validation keypresses arrived!!");
                            }
                        }
                    }
                    else
                    {
                        LOGERR("Wrong VALIDATION_KEY_PRESS ctrlm API revision - expected %d, event is %d!!",
                               CTRLM_RCU_IARM_BUS_API_REVISION, valKey->api_revision);
                    }
                }
                else
                {
                    LOGERR("VALIDATION_KEY_PRESS data is NULL!");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_END)
            {
                LOGINFO("Got a controlMgr VALIDATION_END event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_validation_end_t *valEnd = (ctrlm_rcu_iarm_event_validation_end_t*)data;
                    if (valEnd->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        int validationStatus = 0;
                        LOGINFO("VALIDATION_END - network_id: %d, network_type: %d, controller_id: %d, controller_type: %s, "
                                "binding_type: %d, validation_type: %d, result: %d.\n",
                                valEnd->network_id, valEnd->network_type, valEnd->controller_id, valEnd->controller_type,
                                valEnd->binding_type, valEnd->validation_type, valEnd->result);

                        m_enteredValDigits.clear();


                        // Re-map the controlMgr validation end result to our XRE validationStatus
                        switch (valEnd->result) {
                            case CTRLM_RCU_VALIDATION_RESULT_SUCCESS:           validationStatus = VALIDATION_SUCCESS;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_PENDING:           validationStatus = VALIDATION_FAILURE;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_TIMEOUT:           validationStatus = VALIDATION_TIMEOUT;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_COLLISION:         validationStatus = VALIDATION_FAILURE;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_FAILURE:           validationStatus = VALIDATION_FAILURE;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_ABORT:             validationStatus = VALIDATION_ABORT;        break;
                            case CTRLM_RCU_VALIDATION_RESULT_FULL_ABORT:        validationStatus = VALIDATION_ABORT;        break;
                            case CTRLM_RCU_VALIDATION_RESULT_FAILED:            validationStatus = VALIDATION_WRONG_CODE;   break;
                            case CTRLM_RCU_VALIDATION_RESULT_BIND_TABLE_FULL:   validationStatus = VALIDATION_FAILURE;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_IN_PROGRESS:       validationStatus = VALIDATION_FAILURE;      break;
                            case CTRLM_RCU_VALIDATION_RESULT_CTRLM_RESTART:     validationStatus = VALIDATION_FAILURE;      break;
                            default:                                            validationStatus = VALIDATION_FAILURE;      break;
                        }

                        // Send the ValidationComplete XRE event
                        bool bFromValidationUpdate = false;
                        onXRValidationComplete((int)(valEnd->controller_id), valEnd->controller_type,
                                               (int)(valEnd->binding_type), validationStatus, bFromValidationUpdate);
                    }
                    else
                    {
                        LOGERR("Wrong VALIDATION_END ctrlm API revision - expected %d, event is %d!!",
                               CTRLM_RCU_IARM_BUS_API_REVISION, valEnd->api_revision);
                    }
                }
                else
                {
                    LOGERR("VALIDATION_END data is NULL!");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE)
            {
                LOGINFO("Got a controlMgr CONFIGURATION_COMPLETE event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_configuration_complete_t *cfgComplete = (ctrlm_rcu_iarm_event_configuration_complete_t*)data;
                    if (cfgComplete->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        LOGINFO("CONFIGURATION_COMPLETE - network_id: %d, network_type: %d, controller_id: %d, controller_type: %s, binding_type: %d, result: %d.\n",
                                cfgComplete->network_id, cfgComplete->network_type, cfgComplete->controller_id, cfgComplete->controller_type,
                                cfgComplete->binding_type, cfgComplete->result);

                        if (cfgComplete->result == CTRLM_RCU_CONFIGURATION_RESULT_SUCCESS) {
                            LOGINFO("CONFIGURATION_COMPLETE succeeded!");
                        } else {
                            LOGERR("FAILURE in CONFIGURATION_COMPLETE, configurationStatus: %d!", (int)cfgComplete->result);
                        }

                        // We do not currently remap any of these result codes - it is a pass-thru to the configurationStatus value.
                        onXRConfigurationComplete((int)(cfgComplete->controller_id), cfgComplete->controller_type,
                                                  (int)(cfgComplete->binding_type), &cfgComplete->status, (int)cfgComplete->result);
                    }
                    else
                    {
                        LOGERR("Wrong CONFIGURATION_COMPLETE ctrlm API revision - expected %d, event is %d!!",
                               CTRLM_RCU_IARM_BUS_API_REVISION, cfgComplete->api_revision);
                    }
                }
                else
                {
                    LOGERR("CONFIGURATION_COMPLETE data is NULL!");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT)
            {
                LOGINFO("Got a controlMgr Pairing Window timeout event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_rf4ce_pairing_window_timeout_t *pairingWindowTimeout = (ctrlm_rcu_iarm_event_rf4ce_pairing_window_timeout_t*)data;
                    if (pairingWindowTimeout->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        onPairingWindowTimeout((int)pairingWindowTimeout->validation_result);
                    }
                    else
                    {
                        LOGERR("Wrong PAIRING_WINDOW_TIMEOUT ctrlm API revision - expected %d, event is %d!!",
                               CTRLM_RCU_IARM_BUS_API_REVISION, pairingWindowTimeout->api_revision);
                    }
                }
                else
                {
                    LOGERR("PAIRING_WINDOW_TIMEOUT data is NULL!");
                }
            }
            else
            {
                LOGWARN("Ignoring unexpected pairing event - owner: %s, eventId: %d!!", owner, eventId);
            }
        } // End of rf4cePairingHandler

        //Begin methods
        uint32_t RemoteControl::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t RemoteControl::startPairingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool         bSuccess = false;
            const char*  paramKey = NULL;
            int          netType  = CTRLM_NETWORK_TYPE_RF4CE;
            unsigned int timeout  = 0;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are either NO parameters, or the one we need is missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires the 'netType' parameter!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }
            // Get the timeout from the parameters. If not present, use 0 to mean infinity.
            paramKey = "timeout";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if (value < 0)
                {
                    // The timeout value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'timeout' parameter value: %d!", value);
                    returnResponse(false);
                }
                timeout = (unsigned int)value;
                LOGINFO("timeout passed in is %u.", timeout);
            }
            else
            {
                LOGINFO("timeout not passed in.  Using current timeout");
                timeout = 0;
            }

            if (netType == CTRLM_NETWORK_TYPE_RF4CE)
            {
                bSuccess = rf4ceStartPairing(timeout, response);
            }
            else
            {
                bSuccess = bleStartPairing(timeout, response);
            }

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getNetStatusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are either NO parameters, or the one we need is missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires the 'netType' parameter!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            if (netType == CTRLM_NETWORK_TYPE_RF4CE)
            {
                bSuccess = rf4ceGetNetStatus(response);
            }
            else
            {
                bSuccess = bleGetNetStatus(response);
            }

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRDBManufacturersWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;
            string      avDevType;
            string      manufacturer;

            // All three input parameters, the "netType" and "avDevType" and "manufacturer" are mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType") || !parameters.HasLabel("avDevType") || !parameters.HasLabel("manufacturer"))
            {
                // There are either NO parameters, or some of the ones we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType', 'avDevType' and 'manufacturer' parameters!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            // Get the avDevType from the parameters
            paramKey = "avDevType";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'type' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else
                {
                    // Convert to upper-case
                    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
                    avDevType = value;
                    LOGINFO("'avDevType' set to <%s>.", value.c_str());
                }
            }

            // Get the manufacturer from the parameters
            paramKey = "manufacturer";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    manufacturer = value;
                    LOGINFO("'manufacturer' set to <%s>.", value.c_str());
                }
            }

            bSuccess = getIRDBManufacturers(netType, avDevType, manufacturer, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRDBModelsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;
            string      avDevType;
            string      manufacturer;
            string      model;

            // The netType", "avDevType", "manufacturer", and "model" parameters are all mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType") || !parameters.HasLabel("avDevType") ||
                !parameters.HasLabel("manufacturer") || !parameters.HasLabel("model"))
            {
                // There are either NO parameters, or some of the four we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType', 'avDevType', 'manufacturer', and 'model' parameters!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            // Get the type from the parameters
            paramKey = "avDevType";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'avDevType' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else
                {
                    // Convert to upper-case
                    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
                    avDevType = value;
                    LOGINFO("'avDevType' set to <%s>.", value.c_str());
                }
            }

            // Get the manufacturer from the parameters
            paramKey = "manufacturer";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    manufacturer = value;
                    LOGINFO("'manufacturer' set to <%s>.", value.c_str());
                }
            }

            // Get the model from the parameters
            paramKey = "model";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'model' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'model' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    model = value;
                    LOGINFO("'model' set to <%s>.", value.c_str());
                }
            }

            bSuccess = getIRDBModels(netType, avDevType, manufacturer, model, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRCodesByAutoLookupWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType", "infoFrame" parameters are all mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are either NO parameters, or some of the three we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType' parameters!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            bSuccess = getIRCodesByAutoLookup(netType, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getIRCodesByNamesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;
            string      avDevType;
            string      manufacturer;
            string      model = "";

            // The netType", "avDevType", and "manufacturer" parameters are all mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType") || !parameters.HasLabel("avDevType") ||
                !parameters.HasLabel("manufacturer"))
            {
                // There are either NO parameters, or some of the three we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType', 'avDevType', and 'manufacturer' parameters!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            // Get the type from the parameters
            paramKey = "avDevType";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'avDevType' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else
                {
                    // Convert to upper-case
                    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
                    avDevType = value;
                    LOGINFO("'avDevType' set to <%s>.", value.c_str());
                }
            }

            // Get the manufacturer from the parameters
            paramKey = "manufacturer";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'manufacturer' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    manufacturer = value;
                    LOGINFO("'manufacturer' set to <%s>.", value.c_str());
                }
            }

            // Get the model from the parameters
            paramKey = "model";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    LOGWARN("WARNING - 'model' parameter: value is an empty string!");
                    model = "";
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'model' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    model = value;
                }
            }
            LOGINFO("'model' set to <%s>.", model.c_str());

            bSuccess = getIRCodesByNames(netType, avDevType, manufacturer, model, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::setIRCodeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool               bSuccess = false;
            const char*        paramKey = NULL;
            int                remoteId = 0;
            int                netType = CTRLM_NETWORK_TYPE_RF4CE;
            string             avDevType;
            string             code;

            // The "remoteId", "netType", "avDevType", and "code" parameters are all mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("remoteId") || !parameters.HasLabel("netType") ||
                !parameters.HasLabel("avDevType") || !parameters.HasLabel("code"))
            {
                // There are either NO parameters, or some of the four we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'remoteId', 'netType', 'avDevType', and 'code' parameters!");
                returnResponse(false);
            }


            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            // Get the remoteId from the parameters.
            paramKey = "remoteId";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ( ((netType == CTRLM_NETWORK_TYPE_RF4CE) && ((value < 1) || (value > 9))) ||                // rf4ce remoteId
                     ((netType == CTRLM_NETWORK_TYPE_BLUETOOTH_LE) && ((value < 1) || (value > 9))) )          // sky ble remoteId
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'remoteId' parameter value: %d!", value);
                    returnResponse(false);
                }
                remoteId = value;
                LOGINFO("remoteId passed in is %d.", remoteId);
            }

            // Get the type from the parameters
            paramKey = "avDevType";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'avDevType' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else
                {
                    // Convert to upper-case
                    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
                    avDevType = value;
                    if((avDevType!="TV") && (avDevType!="AMP") )
                    {
                        // The avDevType value is not valid.  Must be "TV" or "AMP". Exit now.
                        LOGERR("ERROR - Bad 'avDevType' parameter: value is %s.", value.c_str());
                        returnResponse(false);
                    }
                    LOGINFO("'avDevType' set to <%s>.", value.c_str());
                }
            }

            // Get the code from the parameters
            paramKey = "code";
            if (parameters.HasLabel(paramKey))
            {
                string value;
                getStringParameter(paramKey, value);
                if (value.size() == 0)
                {
                    // Failure - this is an empty string.
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'code' parameter: value is an empty string!");
                    returnResponse(false);
                }
                else if (value.size() > CTRLM_MAX_PARAM_STR_LEN)
                {
                    // This is a mandatory input parameter, so fail now.
                    LOGERR("ERROR - Bad 'code' parameter: length is too long <%d>!", value.size());
                    returnResponse(false);
                }
                else
                {
                    code = value;
                    LOGINFO("'code' set to <%s>.", value.c_str());
                }
            }

            bSuccess = setIRCode(netType, avDevType, remoteId, code, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::clearIRCodesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool               bSuccess = false;
            const char*        paramKey = NULL;
            int                remoteId = 0ULL;
            int                netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "remoteId", and "netType" parameters are both mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("remoteId") || !parameters.HasLabel("netType"))
            {
                // There are either NO parameters, or one of the two we need are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'remoteId' and 'netType' parameters!");
                returnResponse(false);
            }


            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            // Get the remoteId from the parameters.
            paramKey = "remoteId";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ( ((netType == CTRLM_NETWORK_TYPE_RF4CE) && ((value < 1) || (value > 9))) ||                // rf4ce remoteId
                     ((netType == CTRLM_NETWORK_TYPE_BLUETOOTH_LE) && ((value < 1) || (value > 9))) )          // sky ble remoteId
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'remoteId' parameter value: %d!", value);
                    returnResponse(false);
                }
                remoteId = value;
                LOGINFO("remoteId passed in is %d.", remoteId);
            }

            bSuccess = clearIRCodes(netType, remoteId, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::getLastKeypressSourceWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            const char* paramKey = NULL;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are NO parameters.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires the 'netType' parameter!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            bSuccess = getLastKeypressSource(response, netType);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::configureWakeupKeysWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool        bSuccess = false;
            int         netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are NO parameters.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires the 'netType' parameter!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            if (parameters.HasLabel("netType"))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter("netType", value);
                if (value != CTRLM_NETWORK_TYPE_BLUETOOTH_LE)
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }
            else
            {
                LOGERR("ERROR - 'netType' parameter missing!");
                returnResponse(false);
            }

            std::string wakeupConfig;
            ctrlm_rcu_wakeup_config_t advConfig = CTRLM_RCU_WAKEUP_CONFIG_INVALID;

            if (parameters.HasLabel("wakeupConfig"))
            {
                getStringParameter("wakeupConfig", wakeupConfig);
                LOGINFO("wakeupConfig passed in is <%s>.", wakeupConfig.c_str());
                transform(wakeupConfig.begin(), wakeupConfig.end(), wakeupConfig.begin(), ::tolower);

                advConfig = wakeupConfigFromString(wakeupConfig);
                if (advConfig == CTRLM_RCU_WAKEUP_CONFIG_INVALID)
                {
                    LOGERR("ERROR - Bad 'wakeupConfig' parameter: value is %s.", wakeupConfig.c_str());
                    returnResponse(false);
                }
            }
            else
            {
                LOGERR("ERROR - 'wakeupConfig' parameter missing!");
                returnResponse(false);
            }

            std::string customKeys;
            std::vector<std::string> customList_str;
            int customList[CTRLM_WAKEUP_CONFIG_LIST_MAX_SIZE];
            if (parameters.HasLabel("customKeys"))
            {
                getStringParameter("customKeys", customKeys);
                if (customKeys.empty())
                {
                    LOGERR("ERROR - Bad 'customKeys' parameter: value empty.");
                    returnResponse(false);
                }
                Utils::String::split(customList_str, customKeys, ", ");
                if (customList_str.size() > CTRLM_WAKEUP_CONFIG_LIST_MAX_SIZE)
                {
                    LOGERR("ERROR - Bad 'customKeys' parameter: value too large.");
                    returnResponse(false);
                }
                int i = 0;
                for (auto const &keyStr : customList_str)
                {
                    try
                    {
                        customList[i] = std::stoi(keyStr);
                    }
                    catch (...)
                    {
                        LOGERR("ERROR - Bad 'customKeys' parameter: value not a number.");
                        returnResponse(false);
                    }
                    i++;
                }
            }
            else
            {
                if (advConfig == CTRLM_RCU_WAKEUP_CONFIG_CUSTOM)
                {
                    LOGERR("ERROR - 'wakeupConfig' is custom, but 'customKeys' parameter missing!");
                    returnResponse(false);
                }
            }

            bSuccess = configureWakeupKeys(netType, advConfig, customList, customList_str.size(), response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::initializeIRDBWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool               bSuccess = false;
            const char*        paramKey = NULL;
            int                netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType"))
            {
                // There are either NO parameters, or the one we need is missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType' parameters!");
                returnResponse(false);
            }


            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            bSuccess = initializeIRDB(netType, response);

            returnResponse(bSuccess);
        }

        uint32_t RemoteControl::findMyRemoteWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool               bSuccess = false;
            const char*        paramKey = NULL;
            int                netType  = CTRLM_NETWORK_TYPE_RF4CE;

            // The "netType" parameter is mandatory.
            if (!parameters.IsSet() || !parameters.HasLabel("netType") || !parameters.HasLabel("level"))
            {
                // There are either NO parameters, or the one we need is missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'netType' and 'level' parameters!");
                returnResponse(false);
            }

            // Get the net type from the parameters.
            paramKey = "netType";
            if (parameters.HasLabel(paramKey))
            {
                int value = CTRLM_NETWORK_TYPE_RF4CE;
                getNumberParameter(paramKey, value);
                if ((value < CTRLM_NETWORK_TYPE_RF4CE) || (value > CTRLM_NETWORK_TYPE_BLUETOOTH_LE))
                {
                    // The netType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'netType' parameter value: %d!", value);
                    returnResponse(false);
                }
                netType = value;
                LOGINFO("netType passed in is <%s>.", networkTypeStr(netType));
            }

            std::string levelStr;
            ctrlm_fmr_alarm_level_t level;
            paramKey = "level";
            if (parameters.HasLabel(paramKey))
            {
                getStringParameter(paramKey, levelStr);
                LOGINFO("levelStr passed in is <%s>.", levelStr.c_str());
                transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::tolower);

                level = findMyRemoteLevelFromString(levelStr);
                if (level == CTRLM_FMR_LEVEL_INVALID)
                {
                    LOGERR("ERROR - Bad 'level' parameter: value is %s.", levelStr.c_str());
                    returnResponse(false);
                }
            }
            else
            {
                LOGERR("ERROR - 'level' parameter missing!");
                returnResponse(false);
            }

            bSuccess = findMyRemote(netType, level, response);

            returnResponse(bSuccess);
        }


        uint32_t RemoteControl::factoryReset(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool bSuccess = false;

            ctrlm_main_iarm_call_factory_reset_t reset;
            memset((void*)&reset, 0, sizeof(reset));
            reset.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            reset.network_id = CTRLM_MAIN_NETWORK_ID_ALL;
    
            // The default timeout for IARM calls is 5 seconds, but this call could take longer and we need to ensure the remotes receive
            // the message before the larger system factory reset operation continues.  Therefore, make this timeout longer.
            IARM_Result_t res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_FACTORY_RESET, (void*)&reset, sizeof(reset), IARM_FACTORY_RESET_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FACTORY_RESET IARM_Bus_Call FAILED, res: %d", (int)res);
            }
            else if (reset.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FACTORY_RESET FAILED, result: %d.", (int)reset.result);
            }
            else
            {
                bSuccess = true;
            }

            returnResponse(bSuccess);
        }
        //End methods


        //Begin ble events
        void RemoteControl::onStatus(ctrlm_iarm_RcuStatus_params_t* statusEvt)
        {
            JsonObject               status;
            JsonArray                remoteArray;
            JsonObject               response;

            // Build the "remotes" array
            for (int i = 0; i < statusEvt->num_remotes; i++)
            {
                JsonObject remoteInfo;
                remoteInfo["macAddress"]      = std::string(statusEvt->remotes[i].ieee_address_str);
                remoteInfo["connected"]       = JsonValue((bool)statusEvt->remotes[i].connected);
                remoteInfo["name"]            = std::string(statusEvt->remotes[i].name);
                remoteInfo["remoteId"]        = JsonValue((int)statusEvt->remotes[i].controller_id);
                remoteInfo["deviceId"]        = JsonValue((int)statusEvt->remotes[i].deviceid);
                remoteInfo["make"]            = std::string(statusEvt->remotes[i].make);
                remoteInfo["model"]           = std::string(statusEvt->remotes[i].model);
                remoteInfo["hwVersion"]       = std::string(statusEvt->remotes[i].hwrev);
                remoteInfo["swVersion"]       = std::string(statusEvt->remotes[i].rcuswver);
                remoteInfo["btlVersion"]      = std::string(statusEvt->remotes[i].btlswver);
                remoteInfo["serialNumber"]    = std::string(statusEvt->remotes[i].serialno);
                remoteInfo["batteryPercent"]  = JsonValue((int)statusEvt->remotes[i].batterylevel);
                remoteInfo["wakeupKeyCode"]   = JsonValue((int)statusEvt->remotes[i].wakeup_key_code);
                remoteInfo["wakeupConfig"]    = wakeupConfigToString(statusEvt->remotes[i].wakeup_config);
                if (statusEvt->remotes[i].wakeup_config == CTRLM_RCU_WAKEUP_CONFIG_CUSTOM) {
                    remoteInfo["wakeupCustomList"] = JsonValue(wakeupCustomListToArray(statusEvt->remotes[i].wakeup_custom_list, statusEvt->remotes[i].wakeup_custom_list_size));
                }

                if ((strcmp(statusEvt->remotes[i].tv_code, "0")) && (statusEvt->remotes[i].tv_code[0] != 0))
                {
                    remoteInfo["tvIRCode"]              = std::string(statusEvt->remotes[i].tv_code);
                }
                if ((strcmp(statusEvt->remotes[i].avr_code, "0")) && (statusEvt->remotes[i].avr_code[0] != 0))
                {
                    remoteInfo["ampIRCode"]             = std::string(statusEvt->remotes[i].avr_code);
                }

                remoteArray.Add(remoteInfo);
            }

            status["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_BLUETOOTH_LE);
            status["netTypesSupported"]    = m_netTypesArray;
            status["pairingState"]         = std::string(ctrlm_ble_state_str(statusEvt->status));
            status["irProgState"]          = std::string(ctrlm_ir_prog_state_str(statusEvt->ir_state));
            status["remoteData"]           = remoteArray;
//            status["fmrState"]             = std::string("Optional");
//            status["fmrSource"]            = std::string("Optional");
            response["status"] = status;

            LOGINFO("Sending Thunder event - status: %d, number of remotes: %d.",
                    (int)statusEvt->status, statusEvt->num_remotes);

            sendNotify("onStatus", response);
        }
        //End ble events

        //Begin rf4ce events
        void RemoteControl::onXRPairingStart(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits)
        {
            JsonObject response;
            JsonObject params;
            JsonArray  remoteArray;

            params["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            params["netTypesSupported"]    = JsonValue(1);
            params["pairingState"]         = std::string("PAIRING");
            params["remoteData"]           = remoteArray;
            params["remoteId"] = JsonValue(remoteId);
            params["remoteType"] = std::string(remoteType);
            params["bindingType"] = JsonValue(bindingType);
            params["validationDigit1"]     = JsonValue(validationDigits.digit1);
            params["validationDigit2"]     = JsonValue(validationDigits.digit2);
            params["validationDigit3"]     = JsonValue(validationDigits.digit3);

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, validationDigit1 <%d>, validationDigit2 <%d>, validationDigit3 <%d>\n",
                    remoteId, remoteType, bindingType, validationDigits.digit1, validationDigits.digit2, validationDigits.digit3);
					
            response["status"]             = params;
            sendNotify("onStatus", response);
        }

        void RemoteControl::onXRValidationUpdate(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits)
        {
            JsonObject response;
            JsonObject params;
            string str1 = "n/a";
            string str2 = "n/a";
            string str3 = "n/a";
            JsonArray  remoteArray;

            params["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            params["netTypesSupported"]    = JsonValue(1);
            params["pairingState"]         = std::string("VALIDATING");
            params["remoteData"]           = remoteArray;
            params["remoteId"]             = JsonValue(remoteId);
            params["remoteType"]           = std::string(remoteType);
            params["bindingType"]          = JsonValue(bindingType);

            if (validationDigits.digit1 > -1)
            {
                params["validationDigit1"] = JsonValue(validationDigits.digit1);
                str1 = std::to_string(validationDigits.digit1);
            }
            if (validationDigits.digit2 > -1)
            {
                params["validationDigit2"] = JsonValue(validationDigits.digit2);
                str2 = std::to_string(validationDigits.digit2);
            }
            if (validationDigits.digit3 > -1)
            {
                params["validationDigit3"] = JsonValue(validationDigits.digit3);
                str3 = std::to_string(validationDigits.digit3);
            }

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, validationDigit1 <%s>, validationDigit2 <%s>, validationDigit3 <%s>\n",
                    remoteId, remoteType, bindingType, str1.c_str(), str2.c_str(), str3.c_str());
					
            response["status"]             = params;
            sendNotify("onStatus", response);
        }

        void RemoteControl::onXRValidationComplete(int remoteId, char* remoteType, int bindingType, int validationStatus, bool bFromValidationUpdate)
        {
            JsonObject response;
            JsonObject params;
            JsonArray  remoteArray;

            params["netType"]           = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            params["netTypesSupported"] = JsonValue(1);
            if((bFromValidationUpdate) || (validationStatus == VALIDATION_SUCCESS))
            {
                params["pairingState"]  = std::string("VALIDATION_COMPLETE");
            }
            else if(validationStatus != VALIDATION_SUCCESS)
            {
                params["pairingState"]  = std::string("FAILED");
            }
            params["remoteData"]        = remoteArray;
            params["remoteId"]          = JsonValue(remoteId);
            params["remoteType"]        = std::string(remoteType);
            params["bindingType"]       = JsonValue(bindingType);
            params["validationStatus"]  = JsonValue(validationStatus);

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, validationStatus <%d>\n",
                    remoteId, remoteType, bindingType, validationStatus);
					
            response["status"]          = params;
            sendNotify("onStatus", response);
        }

        void RemoteControl::onXRConfigurationComplete(int remoteId, char* remoteType, int bindingType, ctrlm_controller_status_t *status, int configurationStatus)
        {
            JsonObject response;
            JsonObject params;
            JsonObject remoteInfo;
            JsonArray  remoteArray;

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, configurationStatus <%d>\n",
                    remoteId, remoteType, bindingType, configurationStatus);

            params["netType"]                 = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            params["netTypesSupported"]       = JsonValue(1);
            if(configurationStatus == CTRLM_RCU_CONFIGURATION_RESULT_SUCCESS)
            {
                getRf4ceRemoteData(remoteInfo, remoteId, status);
                remoteArray.Add(remoteInfo);
                params["pairingState"]        = std::string("CONFIGURATION_COMPLETE");
            }
            else
            {
                params["pairingState"]        = std::string("FAILED");
                params["configurationStatus"] = JsonValue(configurationStatus);
            }
            params["remoteData"]              = remoteArray;
            response["status"]                = params;
            sendNotify("onStatus", response);
        }

        void RemoteControl::onPairingWindowTimeout(int validationStatus)
        {
            JsonObject response;
            JsonObject params;
            JsonObject remoteInfo;
            JsonArray  remoteArray;

            LOGINFO("configurationStatus <%d>\n", validationStatus);

            params["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            params["netTypesSupported"]    = JsonValue(1);
            params["pairingState"]         = std::string("FAILED");
            params["validationStatus"]     = JsonValue(validationStatus);
            params["remoteData"]           = remoteArray;
            response["status"]             = params;
            sendNotify("onStatus", response);
        }
        //End rf4ce events

        // Begin private method implementations
        // BLE/ASAPI-based section
        bool RemoteControl::bleStartPairing(unsigned int timeout, JsonObject& response)
        {
            IARM_Result_t                              res;
            ctrlm_iarm_call_StartPairing_params_t      startSearching;
            ctrlm_network_id_t                         bleId = getBleNetworkID();

            // Start by finding the network_id of the rf4ce network on this STB.
            if (bleId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No BLE network_id found!!");
                return false;
            }

            memset((void*)&startSearching, 0, sizeof(startSearching));
            startSearching.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            startSearching.network_id   = bleId;
            startSearching.timeout      = timeout;

            // Make the IARM call to controlMgr
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_START_PAIRING, (void *)&startSearching, sizeof(startSearching));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_START_PAIRING IARM_Bus_Call FAILED, res: %d.", (int)res);
                return false;
            }
            else
            {
                if (startSearching.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_START_PAIRING FAILED, result: %d.", (int)startSearching.result);
                    return false;
                }
            }
            return true;
        }

        bool RemoteControl::bleGetNetStatus(JsonObject& response)
        {
            ctrlm_iarm_RcuStatus_params_t          netStatus;
            IARM_Result_t                          res;
            JsonObject                             status;
            JsonArray                              remoteArray;
            ctrlm_network_id_t                     bleId = getBleNetworkID();

            // Start by finding the network_id of the BLE network on this STB.
            if (bleId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No BLE network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&netStatus, 0, sizeof(netStatus));
            netStatus.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            netStatus.network_id   = bleId;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS, (void*)&netStatus, sizeof(netStatus));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - NETWORK_STATUS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            LOGINFO("BLE network status - netType: <BLE>, network_id: <%d>, status: <%s>, ir_state: <%s>, num_remotes: <%d>.\n",
                       netStatus.network_id, ctrlm_ble_state_str(netStatus.status), ctrlm_ir_prog_state_str(netStatus.ir_state), netStatus.num_remotes);

            if (netStatus.num_remotes == 0)
            {
                LOGWARN("WARNING - No BLE controllers found!");
            }


            if (netStatus.num_remotes > 0)
            {
                LOGINFO("Number of bound remotes is %d.", netStatus.num_remotes);

                // Build the "remotes" array
                for (int i = 0; i < netStatus.num_remotes; i++)
                {
                    JsonObject remote;
                    remote["macAddress"]        = std::string(netStatus.remotes[i].ieee_address_str);
                    remote["connected"]         = JsonValue((bool)netStatus.remotes[i].connected);
                    remote["name"]              = std::string(netStatus.remotes[i].name);
                    remote["remoteId"]          = JsonValue((int)netStatus.remotes[i].controller_id);
                    remote["deviceId"]          = JsonValue((int)netStatus.remotes[i].deviceid);
                    remote["make"]              = std::string(netStatus.remotes[i].make);
                    remote["model"]             = std::string(netStatus.remotes[i].model);
                    remote["hwVersion"]         = std::string(netStatus.remotes[i].hwrev);
                    remote["swVersion"]         = std::string(netStatus.remotes[i].rcuswver);
                    remote["btlVersion"]        = std::string(netStatus.remotes[i].btlswver);
                    remote["serialNumber"]      = std::string(netStatus.remotes[i].serialno);
                    remote["batteryPercent"]    = JsonValue((int)netStatus.remotes[i].batterylevel);
                    remote["wakeupKeyCode"]     = JsonValue((int)netStatus.remotes[i].wakeup_key_code);
                    remote["wakeupConfig"]      = wakeupConfigToString(netStatus.remotes[i].wakeup_config);
                    if (netStatus.remotes[i].wakeup_config == CTRLM_RCU_WAKEUP_CONFIG_CUSTOM) {
                        remote["wakeupCustomList"] = JsonValue(wakeupCustomListToArray(netStatus.remotes[i].wakeup_custom_list, netStatus.remotes[i].wakeup_custom_list_size));
                    }

                    if ((strcmp(netStatus.remotes[i].tv_code, "0")) && (netStatus.remotes[i].tv_code[0] != 0))
                    {
                        remote["tvIRCode"]    = std::string(netStatus.remotes[i].tv_code);
                    }
                    if ((strcmp(netStatus.remotes[i].avr_code, "0")) && (netStatus.remotes[i].avr_code[0] != 0))
                    {
                        remote["ampIRCode"]   = std::string(netStatus.remotes[i].avr_code);
                    }

                    remoteArray.Add((JsonObject)remote);
                }
            }

            status["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_BLUETOOTH_LE);
            status["netTypesSupported"]    = m_netTypesArray;
            status["pairingState"]         = std::string(ctrlm_ble_state_str(netStatus.status));
            status["irProgState"]          = std::string(ctrlm_ir_prog_state_str(netStatus.ir_state));
            status["remoteData"]           = remoteArray;
//            status["fmrState"]             = std::string("Optional");
//            status["fmrSource"]            = std::string("Optional");
            response["status"] = status;
            return true;
        }

        // RF4CE-based section
        bool RemoteControl::rf4ceStartPairing(unsigned int timeout, JsonObject& response)
        {
            IARM_Result_t                       res;
            ctrlm_main_iarm_call_property_t     property;
            if(timeout > 0)
            {
                //Set the active period
                property.api_revision   = CTRLM_MAIN_IARM_BUS_API_REVISION;
                property.result         = CTRLM_IARM_CALL_RESULT_INVALID;
                property.network_id     = CTRLM_MAIN_NETWORK_ID_ALL;
                property.name           = CTRLM_PROPERTY_ACTIVE_PERIOD_SCREENBIND;
                property.value          = timeout * 1000;

                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_PROPERTY_SET, &property, sizeof(property));
                if (res != IARM_RESULT_SUCCESS) {
                    LOGERR("ERROR - Set Active Period <%d> IARM_Bus_Call FAILED, res: %d", timeout, (int)res);
                    return false;
                } else {
                    if (property.result != CTRLM_IARM_CALL_RESULT_SUCCESS) {
                        LOGERR("ERROR - Set Active Period FAILED, call_result: %d", (int)property.result);
                        return false;
                    }
                }
            }
            ctrlm_main_iarm_call_control_service_pairing_mode_t iarmMode;
            ctrlm_network_id_t                                  rf4ceId = getRf4ceNetworkID();

            // First, need to go get the network_id of the rf4ce network on this STB.
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return false;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            memset((void*)&iarmMode, 0, sizeof(iarmMode));
            iarmMode.api_revision       = CTRLM_MAIN_IARM_BUS_API_REVISION;
            iarmMode.network_id         = rf4ceId;
            iarmMode.pairing_mode       = 1;
            iarmMode.restrict_by_remote = 0;

            // Make the IARM call to controlMgr
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE, (void *)&iarmMode, sizeof(iarmMode));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CONTROL_SERVICE_START_PAIRING_MODE IARM_Bus_Call FAILED, res: %d.", (int)res);
                return false;
            }
            else
            {
                if (iarmMode.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROL_SERVICE_START_PAIRING_MODE FAILED, result: %d.", (int)iarmMode.result);
                    return false;
                }
            }
            return true;
        }

        bool RemoteControl::rf4ceGetNetStatus(JsonObject& response)
        {
            JsonArray       infoArray;
            JsonObject      status;

            // The STB data items are directly part of the response - not nested.
            if (!getRf4ceStbData(status))
            {
                LOGERR("ERROR - attempt to get STB data failed!!");
                return false;
            }
            // Get the remoteInfo array updated, and stored in the response as "remoteData".
            if (!getAllRf4ceBindRemotes())
            {
                LOGERR("ERROR - attempt to get STB data failed!!");
                return false;

            }
            else if (m_rf4ceNumOfBindRemotes > 0)
            {
                LOGINFO("Number of bound remotes is %d.", m_rf4ceNumOfBindRemotes);

                for (int i = 0; i < m_rf4ceNumOfBindRemotes; i++)
                {
                    infoArray.Add((JsonObject)m_rf4ceRemoteInfo[i]);
                }
            }
            status["remoteData"] = infoArray;
            response["status"]   = status;
            return true;
        }

        // Generic section
        bool RemoteControl::getIRDBManufacturers(int netType, string avDevType, string manufacturer, JsonObject& response)
        {
            ctrlm_iarm_call_IRManufacturers_params_t       irMfrParams;
            IARM_Result_t                                  res;
            ctrlm_network_id_t                             networkId;
            const char*                                    paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            memset((void*)&irMfrParams, 0, sizeof(irMfrParams));
            irMfrParams.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irMfrParams.network_id   = networkId;
            irMfrParams.type = (avDevType == "AMP" ? CTRLM_IR_DEVICE_AMP : CTRLM_IR_DEVICE_TV);
            snprintf(irMfrParams.manufacturer, sizeof(irMfrParams.manufacturer), "%s", manufacturer.c_str());

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS, (void*)&irMfrParams, sizeof(irMfrParams), IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irMfrParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS FAILED, result: %d.", (int)irMfrParams.result);
                    return false;
                }
            }

            LOGINFO("response <%s>", irMfrParams.response);
            response.FromString(irMfrParams.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::getIRDBModels(int netType, string avDevType, string manufacturer, string model, JsonObject& response)
        {
            ctrlm_iarm_call_IRModels_params_t              irModels;
            IARM_Result_t                                  res;
            ctrlm_network_id_t                             networkId;
            const char*                                    paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            memset((void*)&irModels, 0, sizeof(irModels));
            irModels.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irModels.network_id   = networkId;
            irModels.type = (avDevType == "AMP" ? CTRLM_IR_DEVICE_AMP : CTRLM_IR_DEVICE_TV);
            snprintf(irModels.manufacturer, sizeof(irModels.manufacturer), "%s", manufacturer.c_str());
            snprintf(irModels.model, sizeof(irModels.model), "%s", model.c_str());

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME,  CTRLM_MAIN_IARM_CALL_IR_MODELS, (void*)&irModels, sizeof(irModels), IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR -  CTRLM_MAIN_IARM_CALL_IR_MODELS IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irModels.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR -  CTRLM_MAIN_IARM_CALL_IR_MODELS FAILED, result: %d.", (int)irModels.result);
                    return false;
                }
            }

            LOGINFO("response <%s>", irModels.response);
            response.FromString(irModels.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::getIRCodesByAutoLookup(int netType, JsonObject& response)
        {
            ctrlm_iarm_call_IRAutoLookup_params_t          irAutoLookup;
            IARM_Result_t                                  res;
            ctrlm_network_id_t                             networkId;
            const char*                                    paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            memset((void*)&irAutoLookup, 0, sizeof(irAutoLookup));
            irAutoLookup.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irAutoLookup.network_id   = networkId;
            
            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP, (void*)&irAutoLookup, sizeof(irAutoLookup), IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irAutoLookup.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP FAILED, result: %d.", (int)irAutoLookup.result);
                    return false;
                }
            }

            LOGINFO("response <%s>", irAutoLookup.response);
            response.FromString(irAutoLookup.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::getIRCodesByNames(int netType, string avDevType, string manufacturer, string model, JsonObject& response)
        {
            ctrlm_iarm_call_IRCodes_params_t               irCodes;
            IARM_Result_t                                  res;
            ctrlm_network_id_t                             networkId;
            const char*                                    paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            memset((void*)&irCodes, 0, sizeof(irCodes));
            irCodes.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irCodes.network_id   = networkId;
            irCodes.type         = (avDevType == "AMP" ? CTRLM_IR_DEVICE_AMP : CTRLM_IR_DEVICE_TV);
            snprintf(irCodes.manufacturer, sizeof(irCodes.manufacturer), "%s", manufacturer.c_str());
            snprintf(irCodes.model, sizeof(irCodes.model), "%s", model.c_str());

            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_CODES, (void*)&irCodes, sizeof(irCodes), IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CODES IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irCodes.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CODES FAILED, result: %d.", (int)irCodes.result);
                    return false;
                }
            }

            LOGINFO("response <%s>", irCodes.response);
            response.FromString(irCodes.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::setIRCode(int netType, string avDevType, int remoteId, string code, JsonObject& response)
        {
            ctrlm_iarm_call_IRSetCode_params_t       irCodeParams;
            IARM_Result_t                            res;
            ctrlm_network_id_t                       networkId;
            const char*                              paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&irCodeParams, 0, sizeof(irCodeParams));
            irCodeParams.api_revision  = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irCodeParams.network_id    = networkId;
            irCodeParams.controller_id = remoteId;
            irCodeParams.type = (avDevType == "AMP" ? CTRLM_IR_DEVICE_AMP : CTRLM_IR_DEVICE_TV);
            snprintf(irCodeParams.code, sizeof(irCodeParams.code), "%s", code.c_str());
            LOGINFO("Code <%s>", irCodeParams.code);
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_SET_CODE, (void*)&irCodeParams, sizeof(irCodeParams));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_SET_CODE IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irCodeParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_SET_CODE FAILED, result: %d.", (int)irCodeParams.result);
                    return false;
                }
            }
            LOGINFO("response <%s>", irCodeParams.response);
            response.FromString(irCodeParams.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::clearIRCodes(int netType, int remoteId, JsonObject& response)
        {
            ctrlm_iarm_call_IRClear_params_t       irClearParams;
            IARM_Result_t                          res;
            ctrlm_network_id_t                     networkId;
            const char*                            paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&irClearParams, 0, sizeof(irClearParams));
            irClearParams.api_revision   = CTRLM_MAIN_IARM_BUS_API_REVISION;
            irClearParams.network_id     = networkId;
            irClearParams.controller_id  = remoteId;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE, (void*)&irClearParams, sizeof(irClearParams));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irClearParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE FAILED, result: %d.", (int)irClearParams.result);
                    return false;
                }
            }
            LOGINFO("response <%s>", irClearParams.response);
            response.FromString(irClearParams.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::getLastKeypressSource(JsonObject& keypressInfo, int netType)
        {
            ctrlm_main_iarm_call_last_key_info_t    lastKeyInfo;
            IARM_Result_t                           res;
            ctrlm_network_id_t                      networkId;

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            // Get the current lastKeyInfo from the ControlMgr, which tracks all the information.
            memset((void*)&lastKeyInfo, 0, sizeof(lastKeyInfo));
            lastKeyInfo.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            lastKeyInfo.network_id   = networkId;
            if (netType == CTRLM_NETWORK_TYPE_RF4CE)
            {
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET, (void*)&lastKeyInfo, sizeof(lastKeyInfo));
            }
            else
            {
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_LAST_KEYPRESS_GET, (void*)&lastKeyInfo, sizeof(lastKeyInfo));
            }

            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - LAST_KEY_INFO_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (lastKeyInfo.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - LAST_KEY_INFO_GET FAILED, call_result: %d", (int)lastKeyInfo.result);
                    return false;
                }
            }

            // If we get a zero timestamp, we treat it as a fatal error.
            if (lastKeyInfo.timestamp == 0LL)
            {
                LOGERR("ERROR - LAST_KEY_INFO_GET timestamp is ZERO!!");
                return false;
            }

            keypressInfo["controllerId"]       = JsonValue((int)lastKeyInfo.controller_id);
            #if defined(__aarch64__)
                keypressInfo["timestamp"]          = JsonValue((long)lastKeyInfo.timestamp);   // This timestamp is already in milliseconds
            #else
                keypressInfo["timestamp"]          = JsonValue((long long)lastKeyInfo.timestamp);   // This timestamp is already in milliseconds
            #endif
          
            keypressInfo["sourceName"]         = std::string(lastKeyInfo.source_name);

            if (lastKeyInfo.source_type == IARM_BUS_IRMGR_KEYSRC_RF)
            {
                keypressInfo["sourceType"]     = std::string("RF");
            }
            else
            {
                keypressInfo["sourceType"]     = std::string("IR");
            }

            keypressInfo["sourceKeyCode"]      = JsonValue((int)lastKeyInfo.source_key_code);
            keypressInfo["bIsScreenBindMode"]  = JsonValue((bool)lastKeyInfo.is_screen_bind_mode);
            keypressInfo["remoteKeypadConfig"] = JsonValue((int)lastKeyInfo.remote_keypad_config);
            LOGINFO("controllerId <%d>, key_code <*>, src_type <%d>, timestamp <%lldms>, isScreenBindMode <%d>, remoteKeypadConfig <%d>, sourceName <%s>\n",
                    (int)lastKeyInfo.controller_id, (int)lastKeyInfo.source_type, lastKeyInfo.timestamp,
                    (int)lastKeyInfo.is_screen_bind_mode, (int)lastKeyInfo.remote_keypad_config, lastKeyInfo.source_name);

            return true;
        }

        bool RemoteControl::configureWakeupKeys(int netType, ctrlm_rcu_wakeup_config_t config, int *customList, int customListSize, JsonObject& response)
        {
            ctrlm_iarm_call_WriteRcuWakeupConfig_params_t   iarmParams;
            IARM_Result_t                                   res;
            ctrlm_network_id_t                              networkId;

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            memset((void*)&iarmParams, 0, sizeof(iarmParams));
            iarmParams.api_revision   = CTRLM_MAIN_IARM_BUS_API_REVISION;
            iarmParams.network_id     = networkId;
            iarmParams.config         = config;
            iarmParams.customListSize = customListSize;
            
            if (customList != NULL) {
                if (customListSize > (int)(sizeof(iarmParams.customList)/sizeof(iarmParams.customList[0])))
                {
                    LOGERR("ERROR - customList size too large!");
                    return false;
                }
                memcpy(iarmParams.customList, customList, customListSize * sizeof(iarmParams.customList[0]));
            }

            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG, (void*)&iarmParams, sizeof(iarmParams));

            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - %s IARM_Bus_Call FAILED, res: %d", CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG, (int)res);
                return false;
            }
            else
            {
                if (iarmParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - %s FAILED, call_result: %d", CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG, (int)iarmParams.result);
                    return false;
                }
            }
            return true;
        }

        bool RemoteControl::initializeIRDB(int netType, JsonObject& response)
        {
            ctrlm_iarm_call_initialize_irdb_params_t        initializeIRDBParams;
            IARM_Result_t                                   res;
            ctrlm_network_id_t                              networkId;
            const char*                                     paramKey = "success";

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&initializeIRDBParams, 0, sizeof(initializeIRDBParams));
            initializeIRDBParams.api_revision   = CTRLM_MAIN_IARM_BUS_API_REVISION;
            initializeIRDBParams.network_id     = networkId;
                
            // The default timeout for IARM calls is 5 seconds, but this call could take longer since the results could come from a cloud IRDB.
            // So increase the timeout to IARM_IRDB_CALLS_TIMEOUT
            res = IARM_Bus_Call_with_IPCTimeout(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_INITIALIZE, (void*)&initializeIRDBParams, sizeof(initializeIRDBParams), IARM_IRDB_CALLS_TIMEOUT);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_IR_INITIALIZE IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (initializeIRDBParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_INITIALIZE_IRDB FAILED, result: %d.", (int)initializeIRDBParams.result);
                    return false;
                }
            }
            LOGINFO("response <%s>", initializeIRDBParams.response);
            response.FromString(initializeIRDBParams.response);

            if (response.HasLabel(paramKey))
            {
                bool       value      = 0;
                JsonObject parameters = response;
                getBoolParameter(paramKey, value);
                return value;
            }

            return false;
        }

        bool RemoteControl::findMyRemote(int netType, ctrlm_fmr_alarm_level_t level, JsonObject& response)
        {
            ctrlm_iarm_call_FindMyRemote_params_t           findMyRemoteParams;
            IARM_Result_t                                   res;
            ctrlm_network_id_t                              networkId;

            if(netType==CTRLM_NETWORK_TYPE_RF4CE)
            {
                networkId = getRf4ceNetworkID();
            }
            else
            {
                networkId = getBleNetworkID();
            }

            // Start by finding the network_id of the rf4ce network on this STB.
            if (networkId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&findMyRemoteParams, 0, sizeof(findMyRemoteParams));
            findMyRemoteParams.api_revision   = CTRLM_MAIN_IARM_BUS_API_REVISION;
            findMyRemoteParams.network_id     = networkId;
            findMyRemoteParams.level          = level;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE, (void*)&findMyRemoteParams, sizeof(findMyRemoteParams));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (findMyRemoteParams.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE FAILED, result: %d.", (int)findMyRemoteParams.result);
                    return false;
                }
            }
            return true;
        }
        // End private method implementations

        //Begin local private utility methods
        int RemoteControl::numericCtrlm2Int(ctrlm_key_code_t ctrlm_key)
        {
            int keyCode = 0;   // default

             switch (ctrlm_key) {
                 case CTRLM_KEY_CODE_DIGIT_0:   keyCode = 0; break;
                 case CTRLM_KEY_CODE_DIGIT_1:   keyCode = 1; break;
                 case CTRLM_KEY_CODE_DIGIT_2:   keyCode = 2; break;
                 case CTRLM_KEY_CODE_DIGIT_3:   keyCode = 3; break;
                 case CTRLM_KEY_CODE_DIGIT_4:   keyCode = 4; break;
                 case CTRLM_KEY_CODE_DIGIT_5:   keyCode = 5; break;
                 case CTRLM_KEY_CODE_DIGIT_6:   keyCode = 6; break;
                 case CTRLM_KEY_CODE_DIGIT_7:   keyCode = 7; break;
                 case CTRLM_KEY_CODE_DIGIT_8:   keyCode = 8; break;
                 case CTRLM_KEY_CODE_DIGIT_9:   keyCode = 9; break;
                 default:                       keyCode = 0;
             }

             return keyCode;
        }

        char* RemoteControl::rf4ceGetRemoteModel(char *remoteType)
        {
            static char remoteModel[CTRLM_RCU_VERSION_LENGTH];
            char *temp;

            strncpy(remoteModel, remoteType, CTRLM_RCU_VERSION_LENGTH);
            temp = strchr(remoteModel, '-');
            if (temp != NULL)
            {
                temp[0] = '\0';
            }
            else
            {
                remoteModel[CTRLM_RCU_VERSION_LENGTH - 1] = '\0';
            }

            return remoteModel;
        }

        void RemoteControl::setApiVersionNumber(unsigned int apiVersionNumber)
        {
            LOGINFO("setting version: %d", (int)apiVersionNumber);
            m_apiVersionNumber = apiVersionNumber;
        }
        //End local private utility methods

        // Begin local private BLE utility methods
        const char *RemoteControl::ctrlm_ble_state_str(ctrlm_ble_state_t status)
        {
            switch(status) {
            case CTRLM_BLE_STATE_INITIALIZING: return("INITIALISING");
            case CTRLM_BLE_STATE_IDLE:         return("IDLE");
            case CTRLM_BLE_STATE_SEARCHING:    return("SEARCHING");
            case CTRLM_BLE_STATE_PAIRING:      return("PAIRING");
            case CTRLM_BLE_STATE_COMPLETE:     return("COMPLETE");
            case CTRLM_BLE_STATE_FAILED:       return("FAILED");
            default:                           return("UNKNOWN");
            }
        }
        const char *RemoteControl::ctrlm_ir_prog_state_str(ctrlm_ir_state_t status)
        {
            switch(status) {
            case CTRLM_IR_STATE_IDLE:          return("IDLE");
            case CTRLM_IR_STATE_WAITING:       return("WAITING");
            case CTRLM_IR_STATE_COMPLETE:      return("COMPLETE");
            case CTRLM_IR_STATE_FAILED:        return("FAILED");
            default:                           return("UNKNOWN");
            }
        }

        ctrlm_network_id_t RemoteControl::getBleNetworkID()
        {
            ctrlm_main_iarm_call_status_t   status;
            ctrlm_network_id_t              bleId = CTRLM_MAIN_NETWORK_ID_INVALID;
            IARM_Result_t                   res;

            memset((void*)&status, 0, sizeof(status));
            status.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_STATUS_GET, (void*)&status, sizeof(status));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - STATUS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
            }
            else
            {
                if (status.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - STATUS_GET FAILED, call_result: %d", (int)status.result);
                }
                else
                {
                    m_netTypesArray.Clear();

                    // Apparent success, search for the RF4CE network ID.
                    for (int i = 0; i < status.network_qty; i++)
                    {
                        m_netTypesArray.Add(status.networks[i].id);
                        if (status.networks[i].type == CTRLM_NETWORK_TYPE_BLUETOOTH_LE)
                        {
                            bleId = status.networks[i].id;
                        }
                    }
                }
            }

            return bleId;
        }
        //End BLElocal private utility methods

        // Begin local private rf4ce utility methods
        ctrlm_network_id_t RemoteControl::getRf4ceNetworkID()
        {
            ctrlm_main_iarm_call_status_t   status;
            ctrlm_network_id_t              rf4ceId = CTRLM_MAIN_NETWORK_ID_INVALID;
            IARM_Result_t                   res;

            memset((void*)&status, 0, sizeof(status));
            status.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_STATUS_GET, (void*)&status, sizeof(status));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - STATUS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
            }
            else
            {
                if (status.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - STATUS_GET FAILED, call_result: %d", (int)status.result);
                }
                else
                {
                    m_netTypesArray.Clear();

                    // Apparent success, search for the RF4CE network ID.
                    for (int i = 0; i < status.network_qty; i++)
                    {
                        m_netTypesArray.Add(status.networks[i].id);
                        if (status.networks[i].type == CTRLM_NETWORK_TYPE_RF4CE)
                        {
                            rf4ceId = status.networks[i].id;
                        }
                    }
                }
            }

            return rf4ceId;
        }

        void RemoteControl::rf4ceGetIeeeMacStr(unsigned long long ieee_address_long_long, char *ieee_address_str, int ieee_address_str_size)
        {
            char ieee_address_no_colons[RF4CE_IEEE_MAC_ADDRESS_STR_MAX];	///< IN - MAC address of the RCU
            int index;
            int index2 = 0;

            //Make sure user passes in an array of the expected size 
            if(ieee_address_str_size != RF4CE_IEEE_MAC_ADDRESS_STR_MAX)
            {
                LOGERR("ERROR - ieee_address_str size too large!");
                return;
            }

            memset(ieee_address_str, 0, RF4CE_IEEE_MAC_ADDRESS_STR_MAX);

            snprintf(ieee_address_no_colons, sizeof(ieee_address_no_colons), "%016llX", ieee_address_long_long);
            int length  = strlen(ieee_address_no_colons);
            for (index=0; index<length && index2<RF4CE_IEEE_MAC_ADDRESS_STR_MAX; index++)
            {
                if((index!=0) && (index % 2 == 0))
                {
                    ieee_address_str[index2++] = ':';
                }
                if(index2<RF4CE_IEEE_MAC_ADDRESS_STR_MAX)
                {
                    ieee_address_str[index2++] = ieee_address_no_colons[index];
                }
            }
            //Make sure we NULL terminated properly
            ieee_address_str[RF4CE_IEEE_MAC_ADDRESS_STR_MAX-1] = 0;
        }

        bool RemoteControl::getRf4ceNetworkStatus(ctrlm_main_iarm_call_network_status_t&  netStatus)
        {
            IARM_Result_t      res;
            ctrlm_network_id_t rf4ceId = getRf4ceNetworkID();

            // Start by finding the network_id of the rf4ce network on this STB.
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&netStatus, 0, sizeof(netStatus));
            netStatus.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            netStatus.network_id   = rf4ceId;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET, (void*)&netStatus, sizeof(netStatus));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - NETWORK_STATUS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (netStatus.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - NETWORK_STATUS_GET FAILED, call_result: %d", (int)netStatus.result);
                    return false;
                }
            }
            LOGINFO("RF4CE network status - version_hal: %s, controller_qty: %d, pan_id: 0x%04X, rf_channel number: 0x%02X, rf_channel quality: 0x%02X.\n",
                       netStatus.status.rf4ce.version_hal, netStatus.status.rf4ce.controller_qty, netStatus.status.rf4ce.pan_id,
                       netStatus.status.rf4ce.rf_channel_active.number, netStatus.status.rf4ce.rf_channel_active.quality);

            return true;
        }

        bool RemoteControl::getRf4ceStbData(JsonObject& status)
        {
            ctrlm_main_iarm_call_network_status_t   netStatus;

            // At this time, status consists of RF4CE info and pairing statistics.
            // Start by getting the network status for the RF4CE network on this STB.
            if (!getRf4ceNetworkStatus(netStatus))
            {
                LOGERR("ERROR - getRf4ceNetworkStatus() failed!!");
                return false;
            }

            if (netStatus.status.rf4ce.controller_qty == 0)
            {
                LOGWARN("WARNING - No RF4CE controllers found!");
            }

            // Store the STB-based information from the RF4CE network
            status["netType"]              = JsonValue((int)CTRLM_NETWORK_TYPE_RF4CE);
            status["netTypesSupported"]    = m_netTypesArray;
            if(netStatus.status.rf4ce.controller_qty > 0)
                status["pairingState"]     = std::string("COMPLETE");
            else
                status["pairingState"]     = std::string("FAILED");
//            status["irProgState"]          = std::string("Optional");
//            status["fmrState"]             = std::string("Optional");
//            status["fmrSource"]            = std::string("Optional");

            return true;
        } // End getStbData()

        bool RemoteControl::getAllRf4ceBindRemotes()   // Results are in m_rf4ceRemoteInfo array
        {
            ctrlm_main_iarm_call_network_status_t   netStatus;
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatus;

            // Get the status of all the paired remotes on the rf4ce network.
            // Start by getting the network status for the rf4ce network on this STB.
            if (!getRf4ceNetworkStatus(netStatus))
            {
                LOGERR("ERROR - getRf4ceNetworkStatus() failed!!");
                return false;
            }

            // Make sure we don't overrrun the m_rf4ceRemoteInfo array.
            if (netStatus.status.rf4ce.controller_qty > CTRLM_MAIN_MAX_BOUND_CONTROLLERS)
            {
                LOGERR("LOGIC ERROR - controller_qty(%d) greater than maximum(%d) - limiting to max!!!",
                       (int)netStatus.status.rf4ce.controller_qty, CTRLM_MAIN_MAX_BOUND_CONTROLLERS);
                netStatus.status.rf4ce.controller_qty = CTRLM_MAIN_MAX_BOUND_CONTROLLERS;
            }

            // There are one or more controllers paired on the rf4ce network.
            // Get the status for each one, and put them in the m_rf4ceRemoteInfo array.
            m_rf4ceNumOfBindRemotes = 0;
            for (int i = 0; i < netStatus.status.rf4ce.controller_qty; i++)
            {
                memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
                ctrlStatus.api_revision  = CTRLM_RCU_IARM_BUS_API_REVISION;
                ctrlStatus.network_id    = netStatus.network_id;
                ctrlStatus.controller_id = netStatus.status.rf4ce.controllers[i];

                if (!getRf4ceBindRemote(m_rf4ceRemoteInfo[i], ctrlStatus))
                {
                    LOGERR("ERROR - controller_status for remoteId %d NOT found!", (int)ctrlStatus.controller_id);
                    return false;
                }
            }
            // Save how many bound remotes there are in the array.
            m_rf4ceNumOfBindRemotes = netStatus.status.rf4ce.controller_qty;

            return true;
        }

        bool RemoteControl::getRf4ceBindRemote(JsonObject& remoteInfo, ctrlm_rcu_iarm_call_controller_status_t& ctrlStatus)
        {
            IARM_Result_t   res;
            char            strMACAddress[RF4CE_IEEE_MAC_ADDRESS_STR_MAX] = { 0 };

            // Only do the IARM Bus call to ControlMgr here, if the controller_status isn't already filled in.
            // Otherwise, just do the load of the remoteInfo object from the controller_status passed in.
            if ((ctrlStatus.status.ieee_address == 0LL) && (ctrlStatus.status.short_address == 0) && (ctrlStatus.status.time_binding == 0))
            {
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS, (void*)&ctrlStatus, sizeof(ctrlStatus));
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROLLER_STATUS IARM_Bus_Call FAILED, res: %d, controller_id: %d",
                           (int)res, (int)ctrlStatus.controller_id);
                    return false;
                }
                else
                {
                    if (ctrlStatus.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                    {
                        LOGERR("ERROR - CONTROLLER_STATUS FAILED, call_result: %d, controller_id: %d",
                               (int)ctrlStatus.result, (int)ctrlStatus.controller_id);
                        return false;
                    }
                    else
                    {
                        LOGINFO("Successfully got ctrlm_controller_status_t for remoteId %d.", (int)ctrlStatus.controller_id);
                    }
                }
            }

            remoteInfo.Clear();

            // Load the remoteInfo object with the data from the ctrlm_controller_status_t.
            rf4ceGetIeeeMacStr(ctrlStatus.status.ieee_address, strMACAddress, sizeof(strMACAddress));
            remoteInfo["macAddress"]                = std::string(strMACAddress);
            remoteInfo["connected"]                 = JsonValue(true);
            remoteInfo["name"]                      = std::string(ctrlStatus.status.type);
            remoteInfo["remoteId"]                  = JsonValue((int)ctrlStatus.controller_id);
            remoteInfo["make"]                      = std::string(ctrlStatus.status.manufacturer);
            remoteInfo["model"]                     = std::string(rf4ceGetRemoteModel(ctrlStatus.status.type));
            remoteInfo["hwVersion"]                 = std::string(ctrlStatus.status.version_hardware);
            remoteInfo["swVersion"]                 = std::string(ctrlStatus.status.version_software);
            remoteInfo["btlVersion"]                = std::string(ctrlStatus.status.version_bootloader);
            remoteInfo["batteryPercent"]            = JsonValue((int)ctrlStatus.status.battery_level_percent);
            if ((strcmp(ctrlStatus.status.irdb_entry_id_name_tv, "0")) && (ctrlStatus.status.irdb_entry_id_name_tv[0] != 0))
            {
                remoteInfo["tvIRCode"]              = std::string(ctrlStatus.status.irdb_entry_id_name_tv);
            }
            if ((strcmp(ctrlStatus.status.irdb_entry_id_name_avr, "0")) && (ctrlStatus.status.irdb_entry_id_name_avr[0] != 0))
            {
                remoteInfo["ampIRCode"]             = std::string(ctrlStatus.status.irdb_entry_id_name_avr);
            }

            LOGINFO("controller_id: %d, type: %s, SW version: %s, HW version: %s, time_last_key: %lu.",
                    (int)ctrlStatus.controller_id, ctrlStatus.status.type,
                    ctrlStatus.status.version_software, ctrlStatus.status.version_hardware, ctrlStatus.status.time_last_key);

            return true;
        } // End getRf4ceBindRemote()

        bool RemoteControl::getRf4ceRemoteData(JsonObject& remoteInfo, int controller_id, ctrlm_controller_status_t *status)
        {
            char            strMACAddress[RF4CE_IEEE_MAC_ADDRESS_STR_MAX] = { 0 };

            remoteInfo.Clear();

            // Load the remoteInfo object with the data from the ctrlm_controller_status_t.
            rf4ceGetIeeeMacStr(status->ieee_address, strMACAddress, sizeof(strMACAddress));
            remoteInfo["macAddress"]                = std::string(strMACAddress);
            remoteInfo["connected"]                 = JsonValue(true);
            remoteInfo["name"]                      = std::string(status->type);
            remoteInfo["remoteId"]                  = JsonValue((int)controller_id);
            remoteInfo["make"]                      = std::string(status->manufacturer);
            remoteInfo["model"]                     = std::string(rf4ceGetRemoteModel(status->type));
            remoteInfo["hwVersion"]                 = std::string(status->version_hardware);
            remoteInfo["swVersion"]                 = std::string(status->version_software);
            remoteInfo["btlVersion"]                = std::string(status->version_bootloader);
            remoteInfo["batteryPercent"]            = JsonValue((int)status->battery_level_percent);
            if ((strcmp(status->irdb_entry_id_name_tv, "0")) && (status->irdb_entry_id_name_tv[0] != 0))
            {
                remoteInfo["tvIRCode"]              = std::string(status->irdb_entry_id_name_tv);
            }
            if ((strcmp(status->irdb_entry_id_name_avr, "0")) && (status->irdb_entry_id_name_avr[0] != 0))
            {
                remoteInfo["ampIRCode"]             = std::string(status->irdb_entry_id_name_avr);
            }

            LOGINFO("controller_id: %d, type: %s, SW version: %s, HW version: %s, time_last_key: %lu.",
                    (int)controller_id, status->type,
                    status->version_software, status->version_hardware, status->time_last_key);

            return true;
        } // End getRf4ceBindRemote()
        //End rf4celocal private utility methods

        // Begin local private generic utility methods
        const char *RemoteControl::networkTypeStr(int network_type) {
            switch(network_type) {
                case CTRLM_NETWORK_TYPE_RF4CE:        return("RF4CE");
                case CTRLM_NETWORK_TYPE_BLUETOOTH_LE: return("BLE");
                case CTRLM_NETWORK_TYPE_IP:           return("IP");
                case CTRLM_NETWORK_TYPE_DSP:          return("DSP");
                case CTRLM_NETWORK_TYPE_INVALID:
                default:                              return("INVALID");
            }
        }

        std::string RemoteControl::wakeupConfigToString(ctrlm_rcu_wakeup_config_t config) {
            std::string configStr;
            switch (config) {
                case CTRLM_RCU_WAKEUP_CONFIG_ALL:
                    configStr = "all";
                    break;
                case CTRLM_RCU_WAKEUP_CONFIG_NONE:
                    configStr = "none";
                    break;
                case CTRLM_RCU_WAKEUP_CONFIG_CUSTOM:
                    configStr = "custom";
                    break;
                default:
                    configStr = "invalid";
                    break;
            }
            return configStr;
        }
        ctrlm_rcu_wakeup_config_t RemoteControl::wakeupConfigFromString(std::string configStr) {
            ctrlm_rcu_wakeup_config_t config;
            if (0 == configStr.compare("all")) {
                config = CTRLM_RCU_WAKEUP_CONFIG_ALL;
            } else if (0 == configStr.compare("none")) {
                config = CTRLM_RCU_WAKEUP_CONFIG_NONE;
            } else if (0 == configStr.compare("custom")) {
                config = CTRLM_RCU_WAKEUP_CONFIG_CUSTOM;
            } else {
                config = CTRLM_RCU_WAKEUP_CONFIG_INVALID;
            }
            return config;
        }
        JsonArray RemoteControl::wakeupCustomListToArray(int *list, int listSize) {
            JsonArray wakeupCustomList;
            for (int i = 0; i < listSize; i++) {
                wakeupCustomList.Add(list[i]);
            }
            return wakeupCustomList;
        }

        ctrlm_fmr_alarm_level_t RemoteControl::findMyRemoteLevelFromString(std::string configStr) {
            ctrlm_fmr_alarm_level_t config;
            if (0 == configStr.compare("off")) {
                config = CTRLM_FMR_DISABLE;
            } else if (0 == configStr.compare("mid")) {
                config = CTRLM_FMR_LEVEL_MID;
            } else if (0 == configStr.compare("high")) {
                config = CTRLM_FMR_LEVEL_HIGH;
            } else {
                config = CTRLM_FMR_LEVEL_INVALID;
            }
            return config;
        }
        //End generic local private utility methods

    } // namespace Plugin
} // namespace WPEFramework

