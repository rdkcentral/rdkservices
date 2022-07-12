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

#include "ControlService.h"
#include "libIBusDaemon.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"

#include "irMgr.h"
#include "comcastIrKeyCodes.h"

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

// Find My Remote event types
typedef enum
{
    FMR_EVENT_VALUE_INTERACTION_SUCCESS = 0,
    FMR_EVENT_VALUE_SESSION_TIMEOUT     = 1
} eFindMyRemoteEventValue;

// Ghost code values for event handling
#define GHOST_CODE_IR_POWER_TOGGLE  0
#define GHOST_CODE_VOLUME_UP        1
#define GHOST_CODE_VOLUME_DOWN      2
#define GHOST_CODE_MUTE             3
#define GHOST_CODE_TV_INPUT         4

#define IEEE_MAC_ADDRESS_STR_MAX    30

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::ControlService> metadata(
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


        SERVICE_REGISTRATION(ControlService, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        ControlService* ControlService::_instance = nullptr;

        ControlService::ControlService()
            : PluginHost::JSONRPC()
            , m_apiVersionNumber((uint32_t)-1)   /* default max uint32_t so everything gets enabled */    //TODO(MROLLINS) Can't we access this from jsonrpc interface?
        {
            LOGINFO("ctor");
            ControlService::_instance = this;

            Register("getApiVersionNumber", &ControlService::getApiVersionNumber, this);
            Register("getQuirks", &ControlService::getQuirks, this);

            Register("getAllRemoteData", &ControlService::getAllRemoteDataWrapper, this);
            Register("getSingleRemoteData", &ControlService::getSingleRemoteDataWrapper, this);
            Register("getLastKeypressSource", &ControlService::getLastKeypressSourceWrapper, this);
            Register("getLastPairedRemoteData", &ControlService::getLastPairedRemoteDataWrapper, this);

            Register("setValues", &ControlService::setValuesWrapper, this);
            Register("getValues", &ControlService::getValuesWrapper, this);

            Register("startPairingMode", &ControlService::startPairingModeWrapper, this);
            Register("endPairingMode", &ControlService::endPairingModeWrapper, this);

            Register("canFindMyRemote", &ControlService::canFindMyRemoteWrapper, this);
            Register("findLastUsedRemote", &ControlService::findLastUsedRemoteWrapper, this);

            Register("checkRf4ceChipConnectivity", &ControlService::checkRf4ceChipConnectivityWrapper, this);

            setApiVersionNumber(7);
        }

        ControlService::~ControlService()
        {
            //LOGINFO("dtor");

        }

        const string ControlService::Initialize(PluginHost::IShell* /* service */)
        {
            InitializeIARM();
            // On success return empty, to indicate there is no error text.
            return (string());
        }

        void ControlService::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            ControlService::_instance = nullptr;
        }

        void ControlService::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                // Register for the irMgr irkey events
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY, controlEventHandler) );

                // Register for ControlMgr ghost code events
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_KEY_GHOST, controlEventHandler) );
                // Register for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, controlEventHandler) );
                // Register for ControlMgr API 6 event additions (battery and reboot)
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT, controlEventHandler) );
                // Register for ControlMgr API 6 polling REVERSE_CMD BEGIN and END events (for FindMyRemote)
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END, controlEventHandler) );
                // Register for ControlMgr API 6+ onControl event pass-through
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONTROL, controlEventHandler) );
            }
        }

        void ControlService::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;

                // Remove handlers for irMgr control irkey events
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY, controlEventHandler) );

                // Remove handler for ControlMgr ghost code events
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_KEY_GHOST, controlEventHandler) );
                // Remove handlers for ControlMgr pairing-related events
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_VALIDATION_END, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE, controlEventHandler) );
                // Remove handlers for ControlMgr API 6 event additions (battery and reboot)
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE, controlEventHandler) );
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT, controlEventHandler) );
                // Remove handlers for ControlMgr API 6 polling REVERSE_CMD BEGIN and END events (for FindMyRemote)
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END, controlEventHandler) );
                // Remove handler for ControlMgr API 6+ onControl event pass-through
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_CONTROL, controlEventHandler) );
            }
        }

        // Begin event handlers
        void ControlService::controlEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (ControlService::_instance)
                ControlService::_instance->iarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle IARM events without a ControlService plugin instance!");
        }

        void ControlService::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            // Pick either the irMgr, ControlMgr, or ControlMgr pairing event handlers
            if (!strcmp(owner, IARM_BUS_IRMGR_NAME))
            {
                if (eventId == IARM_BUS_IRMGR_EVENT_IRKEY)
                    irmgrHandler(owner, eventId, data, len);
                else
                    LOGWARN("Ignoring unexpected irMgr event - eventId: %d!!", eventId);
            }
            else if (!strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                if ((eventId == CTRLM_RCU_IARM_EVENT_KEY_GHOST) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END) ||
                    (eventId == CTRLM_RCU_IARM_EVENT_CONTROL))
                {
                    ctrlmHandler(owner, eventId, data, len);
                }
                else if ((eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN) ||
                         (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS) ||
                         (eventId == CTRLM_RCU_IARM_EVENT_VALIDATION_END) ||
                         (eventId == CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE))
                {
                    pairingHandler(owner, eventId, data, len);
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
        }

        void ControlService::irmgrHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (eventId == IARM_BUS_IRMGR_EVENT_IRKEY)
            {
                IARM_Bus_IRMgr_EventData_t *irEventData = (IARM_Bus_IRMgr_EventData_t*)data;
                int keyCode  = irEventData->data.irkey.keyCode;
                int keySrc   = irEventData->data.irkey.keySrc;
                int keyType  = irEventData->data.irkey.keyType;
                int remoteId = irEventData->data.irkey.keySourceId;

                if ((keyType == KET_KEYUP) || (keyType == KET_KEYREPEAT)) {
                    // Don't process keyup or repeat events - only use keydown.
                    return;
                }

                if (len != sizeof(IARM_Bus_IRMgr_EventData_t)) {
                    LOGERR("ERROR - Got IARM_BUS_IRMGR_EVENT_IRKEY event with bad data length: %u, should be: %u!!",
                           len, sizeof(IARM_Bus_IRMgr_EventData_t));
                }
                // We only care about the SETUP key - don't clutter the log with other irMgr keypress events
                if (keyCode == KED_SETUP)
                {
                    if(keySrc != IARM_BUS_IRMGR_KEYSRC_IR)
                    {
                        LOGERR("Got Invalid SETUP key event, keySrc: %d, remoteId: %d.", keySrc, remoteId);
                    }
                    else
                    {
                        string type     = "SETUP";
                        string source   = "IR";
                        string data     = "";
                        int deviceID    = -1;

                        LOGINFO("Got IARM_BUS_IRMGR_EVENT_IRKEY event, remoteId: %d, keyCode: KED_SETUP.\n.", remoteId);

                        onControl(deviceID, keyCode, source, type, data);
                    }
                }
            }
        }

        void ControlService::ctrlmHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (eventId == CTRLM_RCU_IARM_EVENT_KEY_GHOST)
            {
                LOGINFO("Got a controlMgr ghost key event!");
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_key_ghost_t *msg = (ctrlm_rcu_iarm_event_key_ghost_t*)data;
                    int remoteId = msg->controller_id;
                    int ghostCode = msg->ghost_code;

                    string source   = "RF";
                    string type     = "ghost";
                    string data     = std::to_string((unsigned int)msg->remote_keypad_config);

                    if (len != sizeof(ctrlm_rcu_iarm_event_key_ghost_t))
                    {
                        LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_KEY_GHOST event with bad data length: %u, should be: %u!!\n",
                               len, sizeof(ctrlm_rcu_iarm_event_key_ghost_t));
                    }

                    LOGINFO("Got CTRLM_RCU_IARM_EVENT_KEY_GHOST event, remoteId: %d, ghostCode: 0x%02X\n.",
                            remoteId, (unsigned int)ghostCode);

                    switch(ghostCode)
                    {
                        case CTRLM_RCU_GHOST_CODE_IR_POWER_TOGGLE:
                            ghostCode = GHOST_CODE_IR_POWER_TOGGLE;
                            break;
                        case CTRLM_RCU_GHOST_CODE_VOLUME_UP:
                            ghostCode = GHOST_CODE_VOLUME_UP;
                            break;
                        case CTRLM_RCU_GHOST_CODE_VOLUME_DOWN:
                            ghostCode = GHOST_CODE_VOLUME_DOWN;
                            break;
                        case CTRLM_RCU_GHOST_CODE_MUTE:
                            ghostCode = GHOST_CODE_MUTE;
                            break;
                        case CTRLM_RCU_GHOST_CODE_INPUT:
                            ghostCode = GHOST_CODE_TV_INPUT;
                            break;
                        default:
                            LOGWARN("WARNING: Unknown type CTRLM_RCU_IARM_EVENT_KEY_GHOST event, ghostCode: 0x%02X.  Ignoring!\n",
                                    (unsigned int)ghostCode);
                            return;
                    }

                    onControl(remoteId, ghostCode, source, type, data);
                }
                else
                {
                    LOGWARN("WARNING: NULL data from CTRLM_RCU_IARM_EVENT_KEY_GHOST event!\n");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE)
            {
                LOGINFO("Got a controlMgr battery milestone event!");
                if (len != sizeof(ctrlm_rcu_iarm_event_battery_t))
                {
                    LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE event with bad data length: %u, should be: %u!!\n",
                           len, sizeof(ctrlm_rcu_iarm_event_battery_t));
                    return;
                }
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_battery_t *msg = (ctrlm_rcu_iarm_event_battery_t*)data;
                    if (msg->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        int remoteId = msg->controller_id;
                        int value = msg->battery_event;

                        string source   = "RF";
                        string type     = "battery";
                        string data     = std::to_string((int)msg->percent) + "%";


                        LOGINFO("Got CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE event, remoteId: %d, battery_event: %d, percent: %d.\n.",
                                 remoteId, value, (int)msg->percent);

                        onControl(remoteId, value, source, type, data);
                    }
                    else
                    {
                        LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE event with bad ctrlm RCU API revision: %u, should be: %u!!\n",
                               (unsigned)msg->api_revision, CTRLM_RCU_IARM_BUS_API_REVISION);
                    }
                }
                else
                {
                    LOGERR("ERROR - NULL data from CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE event!\n");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT)
            {
                LOGINFO("Got a controlMgr remote reboot event!");
                if (len != sizeof(ctrlm_rcu_iarm_event_remote_reboot_t))
                {
                    LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT event with bad data length: %u, should be: %u!!\n",
                           len, sizeof(ctrlm_rcu_iarm_event_remote_reboot_t));
                    return;
                }
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_remote_reboot_t *msg = (ctrlm_rcu_iarm_event_remote_reboot_t*)data;
                    if (msg->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        int remoteId = msg->controller_id;
                        int value = msg->reason;

                        string source   = "RF";
                        string type     = "remoteReboot";
                        string data     = "";
                        if (msg->reason == CONTROLLER_REBOOT_ASSERT_NUMBER)
                        {
                            // For an assert reason ONLY, the data becomes the assert number, in string form.
                            data = std::to_string((int)msg->assert_number);
                        }

                        LOGINFO("Got CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT event, remoteId: %d, reason: %d, timestamp: %lu (seconds), assert_number: %s.\n.",
                                remoteId, value, msg->timestamp, ((msg->reason == CONTROLLER_REBOOT_ASSERT_NUMBER) ? data.c_str() : "NA"));

                        onControl(remoteId, value, source, type, data);
                    }
                    else
                    {
                        LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT event with bad ctrlm RCU API revision: %u, should be: %u!!\n",
                               (unsigned)msg->api_revision, CTRLM_RCU_IARM_BUS_API_REVISION);
                    }
                }
                else
                {
                    LOGERR("ERROR - NULL data from CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT event!\n");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END)
            {
                LOGINFO("Got a controlMgr reverse cmd end event!");
                // Allow for variable size event, dictated by this event type
                if (len < sizeof(ctrlm_rcu_iarm_event_reverse_cmd_t))
                {
                    LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END event with bad data length: %u, should be: %u!!\n",
                           len, sizeof(ctrlm_rcu_iarm_event_reverse_cmd_t));
                    return;
                }
                if (data != NULL)
                {
                    ctrlm_rcu_iarm_event_reverse_cmd_t *msg = (ctrlm_rcu_iarm_event_reverse_cmd_t*)data;
                    if (msg->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                    {
                        ctrlm_rcu_reverse_cmd_t cmd = msg->action;
                        if (cmd == CTRLM_RCU_REVERSE_CMD_FIND_MY_REMOTE) {

                            int remoteId = msg->controller_id;
                            int value = 0;

                            string source   = "RF";
                            string type     = "findMyRemote";
                            string data     = "";

                            // Lookup eventValue from the "result"
                            switch (msg->result) {
                                case CTRLM_RCU_REVERSE_CMD_SUCCESS:                 value = FMR_EVENT_VALUE_INTERACTION_SUCCESS;    break;
                                case CTRLM_RCU_REVERSE_CMD_FAILURE:                 value = FMR_EVENT_VALUE_SESSION_TIMEOUT;        break;
                                case CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_FOUND:    value = FMR_EVENT_VALUE_SESSION_TIMEOUT;        break;
                                case CTRLM_RCU_REVERSE_CMD_CONTROLLER_FOUND:        value = FMR_EVENT_VALUE_SESSION_TIMEOUT;        break;
                                case CTRLM_RCU_REVERSE_CMD_USER_INTERACTION:        value = FMR_EVENT_VALUE_INTERACTION_SUCCESS;    break;
                                default:                                            value = FMR_EVENT_VALUE_SESSION_TIMEOUT;        break;
                            }

                            LOGINFO("Got FIND_MY_REMOTE in CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END event, remoteId: %d, eventValue: %d. Event result: %d.\n.",
                                    remoteId, value, msg->result);

                            onControl(remoteId, value, source, type, data);
                        }
                        else
                        {
                            LOGWARN("Got CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END event for cmd: %d, instead of CTRLM_RCU_REVERSE_CMD_FIND_MY_REMOTE.\n",
                                    (int)cmd);
                        }
                    }
                    else
                    {
                        LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END event with bad ctrlm RCU API revision: %u, should be: %u!!\n",
                               (unsigned)msg->api_revision, CTRLM_RCU_IARM_BUS_API_REVISION);
                    }
                }
                else
                {
                    LOGERR("ERROR - NULL data from CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END event!\n");
                }
            }
            else if (eventId == CTRLM_RCU_IARM_EVENT_CONTROL)
            {
               LOGINFO("Got a controlMgr control event!");
               if (data != NULL)
               {
                   ctrlm_rcu_iarm_event_control_t *msg = (ctrlm_rcu_iarm_event_control_t*)data;
                   int remoteId     = msg->controller_id;
                   int value        = msg->event_value;
                   int spare_value  = msg->spare_value;
                   string source    = msg->event_source;
                   string type      = msg->event_type;
                   string data      = msg->event_data;

                   if (len != sizeof(ctrlm_rcu_iarm_event_control_t))
                   {
                       LOGERR("ERROR - Got CTRLM_RCU_IARM_EVENT_CONTROL event with bad data length: %u, should be: %u!!\n",
                              len, sizeof(ctrlm_rcu_iarm_event_control_t));
                   }

                   LOGINFO("Got CTRLM_RCU_IARM_EVENT_CONTROL event, remoteId: %d, source: %s, type: %s, data: %s, value: %d.\n.",
                           remoteId, source.c_str(), type.c_str(), data.c_str(), value);

                   if(type == "sfm")
                   {
                        switch (spare_value) {
                            //Ignore these sfm's
                            case CTRLM_RCU_FUNCTION_SETUP:
                            case CTRLM_RCU_FUNCTION_BLINK_SOFTWARE_VERSION:
                            case CTRLM_RCU_FUNCTION_BLINK_AVR_CODE:
                            case CTRLM_RCU_FUNCTION_BLINK_TV_CODE:
                            case CTRLM_RCU_FUNCTION_BLINK_IR_DB_VERSION:
                            case CTRLM_RCU_FUNCTION_BLINK_BATTERY_LEVEL:
                            case CTRLM_RCU_FUNCTION_INVALID_KEY_COMBO:
                            case CTRLM_RCU_FUNCTION_RESET_SOFT:
                            case CTRLM_RCU_FUNCTION_KEY_REMAPPING:
                            case CTRLM_RCU_FUNCTION_INVALID:
                                LOGWARN("Ignoring sfm <%s>\n", data.c_str());
                                return;
                        }
                   }

                   onControl(remoteId, value, source, type, data);
               }
               else
               {
                   LOGWARN("NULL data from CTRLM_RCU_IARM_EVENT_CONTROL event!\n");
               }
            }
            else
            {
                // We shouldn't be getting events that aren't handled here!
                LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
            }
        } // End of ctrlmHandler

        void ControlService::pairingHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
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
                                LOGINFO("Sending onXRValidationComplete event from VALIDATION_KEY_PRESS - wrong digit entered!");
                                m_enteredValDigits.clear();

                                // Send the Validation Complete (with a WRONG_CODE error) from here, because controlMgr won't send VALIDATION_END in this case
                                onXRValidationComplete((int)(valKey->controller_id), valKey->controller_type,
                                                       (int)(valKey->binding_type), (int)VALIDATION_WRONG_CODE);
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
                        onXRValidationComplete((int)(valEnd->controller_id), valEnd->controller_type,
                                               (int)(valEnd->binding_type), validationStatus);
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
                                                  (int)(cfgComplete->binding_type), (int)cfgComplete->result);
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
            else
            {
                LOGWARN("Ignoring unexpected pairing event - owner: %s, eventId: %d!!", owner, eventId);
            }
        } // End of pairingHandler
        // End event handlers


        // Begin methods
        uint32_t ControlService::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["version"] = m_apiVersionNumber;
            returnResponse(true);
        }

        uint32_t ControlService::getQuirks(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            JsonArray array;
            array.Add("DELIA-43686");
            array.Add("RDK-28767");
            array.Add("RDK-31263");
            array.Add("RDK-32347");
            response["quirks"] = array;
            returnResponse(true);
        }

        uint32_t ControlService::getAllRemoteDataWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = getAllRemoteData(response);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::getSingleRemoteDataWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            JsonObject remoteInfo;
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int remoteId = -1;

            if (!parameters.IsSet() || !parameters.HasLabel("remoteId"))
            {
                // There are either NO parameters, or no remoteId.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'remoteId' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the remoteId from the parameters
            paramKey = "remoteId";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The remoteId value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'remoteId' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                remoteId = value;
                LOGINFO("remoteId passed in is %d.", remoteId);
            }

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = getSingleRemoteData(remoteInfo, remoteId);

            if (status_code == STATUS_OK)
            {
                // Return the data for the specified remote
                response["remoteData"] = JsonValue(remoteInfo);
            }

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::getLastKeypressSourceWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = getLastKeypressSource(response);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::getLastPairedRemoteDataWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            JsonObject remoteInfo;
            StatusCode status_code = STATUS_OK;

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = getLastPairedRemoteData(remoteInfo);

            if (status_code == STATUS_OK)
            {
                // Return the data for the last paired remote
                response["remoteData"] = JsonValue(remoteInfo);
            }

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::setValuesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;

            if (!parameters.IsSet())
            {
                // There are NO parameters. We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - there are no parameters indicating what to set!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = setValues(parameters);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::getValuesWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = getValues(response);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::startPairingModeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int pairingMode = -1;
            int restrictPairing = -1;

            if (!parameters.IsSet() || !parameters.HasLabel("pairingMode") || !parameters.HasLabel("restrictPairing"))
            {
                // There are either NO parameters, or the two we want are missing.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires 'pairingMode' and 'restrictPairing' parameters!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the pairingMode from the parameters
            paramKey = "pairingMode";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value >= CTRLM_PAIRING_MODE_MAX))
                {
                    // The pairingMode value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'pairingMode' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                pairingMode = value;
                LOGINFO("pairingMode passed in is %d.", pairingMode);
            }

            // Get the restrictPairing value from the parameters
            paramKey = "restrictPairing";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value >= CTRLM_PAIRING_RESTRICT_MAX))
                {
                    // The restrictPairing value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'restrictPairing' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                restrictPairing = value;
                LOGINFO("restrictPairing passed in is %d.", restrictPairing);
            }

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = startPairingMode(pairingMode, restrictPairing);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::endPairingModeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode  status_code = STATUS_OK;
            int         bindStatus = -1;

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = endPairingMode(bindStatus);
            if (status_code == STATUS_OK)
            {
                response["bindStatus"] = bindStatus;
            }

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::canFindMyRemoteWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = false;

            std::lock_guard<std::mutex> guard(m_callMutex);

            result = canFindMyRemote();

            response["result"] = result;
            returnResponse(true);
        }

        uint32_t ControlService::findLastUsedRemoteWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int timeOutPeriod = -1;
            bool bOnlyLastUsed = true;

            if (!parameters.HasLabel("timeOutPeriod"))
            {

                LOGERR("ERROR - this method requires a 'timeOutPeriod' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the timeOutPeriod from the parameters
            paramKey = "timeOutPeriod";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 5) || (value > 30))    // These are expressed in seconds, not milliseconds!
                {
                    // The remoteId value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'timeOutPeriod' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                timeOutPeriod = value;
                LOGINFO("timeOutPeriod passed in is %d.", timeOutPeriod);
            }

            std::lock_guard<std::mutex> guard(m_callMutex);

            status_code = findMyRemote(timeOutPeriod, bOnlyLastUsed);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t ControlService::checkRf4ceChipConnectivityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            eCheckRf4ceChipConnectivity result = RF4CE_CHIP_CONNECTIVITY_NOT_SUPPORTED;

            std::lock_guard<std::mutex> guard(m_callMutex);

            result = checkRf4ceChipConnectivity();

            if(result == RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR)
            {
                LOGERR("IARM_CALL_RESULT_ERROR.");
                returnResponse(false);
            }

            response["rf4ceChipConnected"] = (int)result;
            returnResponse(true);
        }
        //End methods

        //Begin events
        void ControlService::onControl(int remoteId, int keyCode, string& source, string& type, string& data)
        {
            JsonObject params;

            params["remoteId"]    = JsonValue(remoteId);
            params["eventValue"]  = JsonValue(keyCode);
            params["eventSource"] = source;
            params["eventType"]   = type;
            params["eventData"]   = data;

            LOGINFO("remoteId <%d>, eventValue <0x%x>, eventSource <%s>, eventType <%s>, eventData <%s>.\n",
                    remoteId, keyCode, source.c_str(), type.c_str(), data.c_str());

            sendNotify("onControl", params);
        }

        void ControlService::onXRPairingStart(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits)
        {
            JsonObject params;

            params["remoteId"] = JsonValue(remoteId);
            params["remoteType"] = std::string(remoteType);
            params["bindingType"] = JsonValue(bindingType);
            params["validationDigit1"] = JsonValue(validationDigits.digit1);
            params["validationDigit2"] = JsonValue(validationDigits.digit2);
            params["validationDigit3"] = JsonValue(validationDigits.digit3);

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, validationDigit1 <%d>, validationDigit2 <%d>, validationDigit3 <%d>\n",
                    remoteId, remoteType, bindingType, validationDigits.digit1, validationDigits.digit2, validationDigits.digit3);

            sendNotify("onXRPairingStart", params);
        }

        void ControlService::onXRValidationUpdate(int remoteId, char* remoteType, int bindingType, threeDigits& validationDigits)
        {
            JsonObject params;
            string str1 = "n/a";
            string str2 = "n/a";
            string str3 = "n/a";

            params["remoteId"] = JsonValue(remoteId);
            params["remoteType"] = std::string(remoteType);
            params["bindingType"] = JsonValue(bindingType);

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

            sendNotify("onXRValidationUpdate", params);
        }

        void ControlService::onXRValidationComplete(int remoteId, char* remoteType, int bindingType, int validationStatus)
        {
            JsonObject params;

            params["remoteId"] = JsonValue(remoteId);
            params["remoteType"] = std::string(remoteType);
            params["bindingType"] = JsonValue(bindingType);
            params["validationStatus"] = JsonValue(validationStatus);

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, validationStatus <%d>\n",
                    remoteId, remoteType, bindingType, validationStatus);

            sendNotify("onXRValidationComplete", params);
        }

        void ControlService::onXRConfigurationComplete(int remoteId, char* remoteType, int bindingType, int configurationStatus)
        {
            JsonObject params;

            params["remoteId"] = JsonValue(remoteId);
            params["remoteType"] = std::string(remoteType);
            params["bindingType"] = JsonValue(bindingType);
            params["configurationStatus"] = JsonValue(configurationStatus);

            LOGINFO("remoteId <%d>, remoteType <%s>, bindingType <%d>, configurationStatus <%d>\n",
                    remoteId, remoteType, bindingType, configurationStatus);

            sendNotify("onXRConfigurationComplete", params);
        }
        // End events

        // Begin private method implementations
        StatusCode ControlService::getAllRemoteData(JsonObject& response)
        {
            JsonArray    infoArray;

            // The STB data items are directly part of the response - not nested.
            if (!getRf4ceStbData(response))
            {
                LOGERR("ERROR - attempt to get STB data failed!!");
                return STATUS_FAILURE;
            }
            // Get the remoteInfo array updated, and stored in the response as "remoteData".
            if (!getAllRf4ceBindRemotes())
            {
                LOGERR("ERROR - attempt to get STB data failed!!");
                return STATUS_FAILURE;

            }
            else if (m_numOfBindRemotes > 0)
            {
                LOGINFO("Number of bound remotes is %d.\n", m_numOfBindRemotes);

                for (int i = 0; i < m_numOfBindRemotes; i++)
                {
                    infoArray.Add((JsonObject)m_remoteInfo[i]);
                }
                response["remoteData"] = infoArray;
            }

            return STATUS_OK;
        }

        StatusCode ControlService::getSingleRemoteData(JsonObject& remoteInfo, int remoteId)
        {
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatus;
            ctrlm_network_id_t                      rf4ceId = CTRLM_MAIN_NETWORK_ID_INVALID;

            // Start by finding the network_id of the rf4ce network on this STB.
            if (!getRf4ceNetworkId(rf4ceId))
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return STATUS_INVALID_STATE;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
            ctrlStatus.api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
            ctrlStatus.network_id = rf4ceId;
            ctrlStatus.controller_id = remoteId;

            if (!getRf4ceBindRemote(remoteInfo, ctrlStatus))
            {
                LOGERR("ERROR - remoteInfo not found for remoteId %d!!", remoteId);
                return STATUS_INVALID_ARGUMENT;
            }

            return STATUS_OK;
        }

        StatusCode ControlService::getLastPairedRemoteData(JsonObject& remoteInfo)
        {
            if (!getLastPairedRf4ceBindRemote(remoteInfo))
            {
                LOGERR("ERROR - search for last paired remote failed!!");
                return STATUS_FAILURE;
            }

            return STATUS_OK;
        }

        StatusCode ControlService::getLastKeypressSource(JsonObject& keypressInfo)
        {
            ctrlm_main_iarm_call_last_key_info_t    lastKeyInfo;
            IARM_Result_t                           res;

            // Get the current lastKeyInfo from the ControlMgr, which tracks all the information.
            memset((void*)&lastKeyInfo, 0, sizeof(lastKeyInfo));
            lastKeyInfo.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET, (void*)&lastKeyInfo, sizeof(lastKeyInfo));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - LAST_KEY_INFO_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return STATUS_INVALID_STATE;
            }
            else
            {
                if (lastKeyInfo.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - LAST_KEY_INFO_GET FAILED, call_result: %d", (int)lastKeyInfo.result);
                    return STATUS_FAILURE;
                }
            }

            // If we get a zero timestamp, we treat it as a fatal error.
            if (lastKeyInfo.timestamp == 0LL)
            {
                LOGERR("ERROR - LAST_KEY_INFO_GET timestamp is ZERO!!");
                return STATUS_FAILURE;
            }

            keypressInfo["remoteId"]           = JsonValue((int)lastKeyInfo.controller_id);
            keypressInfo["timestamp"]          = JsonValue((long long)lastKeyInfo.timestamp);   // This timestamp is already in milliseconds
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

            LOGINFO("remoteId <%d>, key_code <*>, src_type <%d>, timestamp <%lldms>, isScreenBindMode <%d>, remoteKeypadConfig <%d>, sourceName <%s>\n",
                    (int)lastKeyInfo.controller_id, (int)lastKeyInfo.source_type, lastKeyInfo.timestamp,
                    (int)lastKeyInfo.is_screen_bind_mode, (int)lastKeyInfo.remote_keypad_config, lastKeyInfo.source_name);

            return STATUS_OK;
        }

        StatusCode ControlService::setValues(const JsonObject& parameters)  // The stupid "getXXXXParameter() macro REQUIRES the JsonObject to be named "parameters"
        {
            ctrlm_main_iarm_call_control_service_settings_t iarmSettings;
            IARM_Result_t                                   res;
            StatusCode                                      status_code = STATUS_OK;
            const char*                                     paramKey = NULL;

            memset((void*)&iarmSettings, 0, sizeof(iarmSettings));
            iarmSettings.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;

            // Extract the settings that were passed in, and set them in the IARM structure
            paramKey = "enableASB";
            if (parameters.HasLabel(paramKey))
            {
                bool value = false;
                getBoolParameter(paramKey, value);
                LOGINFO("ASB_ENABLED set to %s.", (value ? "True" : "False"));
                iarmSettings.asb_enabled = (unsigned char)(value ? 1 : 0);
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_ASB_ENABLED;
            }
            paramKey = "enableOpenChime";
            if (parameters.HasLabel(paramKey))
            {
                bool value = false;
                getBoolParameter(paramKey, value);
                LOGINFO("OPEN_CHIME_ENABLED set to %s.", (value ? "True" : "False"));
                iarmSettings.open_chime_enabled = (unsigned char)(value ? 1 : 0);
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_OPEN_CHIME_ENABLED;
            }
            paramKey = "enableCloseChime";
            if (parameters.HasLabel(paramKey))
            {
                bool value = false;
                getBoolParameter(paramKey, value);
                LOGINFO("CLOSE_CHIME_ENABLED set to %s.", (value ? "True" : "False"));
                iarmSettings.close_chime_enabled = (unsigned char)(value ? 1 : 0);
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_CLOSE_CHIME_ENABLED;
            }
            paramKey = "enablePrivacyChime";
            if (parameters.HasLabel(paramKey))
            {
                bool value = false;
                getBoolParameter(paramKey, value);
                LOGINFO("PRIVACY_CHIME_ENABLED set to %s.", (value ? "True" : "False"));
                iarmSettings.privacy_chime_enabled = (unsigned char)(value ? 1 : 0);
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_PRIVACY_CHIME_ENABLED;
            }
            paramKey = "conversationalMode";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if (value < CTRLM_MIN_CONVERSATIONAL_MODE) {
                    LOGWARN("CONVERSATIONAL_MODE value of %d is TOO SMALL -  setting to minimum.", value);
                    value = CTRLM_MIN_CONVERSATIONAL_MODE;
                } else if (value > CTRLM_MAX_CONVERSATIONAL_MODE) {
                    LOGWARN("CONVERSATIONAL_MODE value of %d is TOO LARGE -  setting to maximum.", value);
                    value = CTRLM_MAX_CONVERSATIONAL_MODE;
                }
                LOGINFO("CONVERSATIONAL_MODE set to %d.", value);
                iarmSettings.conversational_mode = (unsigned char)value;
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_CONVERSATIONAL_MODE;
            }
            paramKey = "chimeVolume";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if (value < CTRLM_CHIME_VOLUME_LOW) {
                    LOGWARN("CHIME_VOLUME value of %d is TOO SMALL -  setting to minimum.", value);
                    value = CTRLM_CHIME_VOLUME_LOW;
                } else if (value > CTRLM_CHIME_VOLUME_HIGH) {
                    LOGWARN("CHIME_VOLUME value of %d is TOO LARGE -  setting to maximum.", value);
                    value = CTRLM_CHIME_VOLUME_HIGH;
                }
                LOGINFO("CHIME_VOLUME set to %d.", value);
                iarmSettings.chime_volume = (ctrlm_chime_volume_t)value;
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_SET_CHIME_VOLUME;
            }
            paramKey = "irCommandRepeats";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if (value < CTRLM_MIN_IR_COMMAND_REPEATS) {
                    LOGWARN("IR_COMMAND_REPEATS value of %d is TOO SMALL -  setting to minimum.", value);
                    value = CTRLM_MIN_IR_COMMAND_REPEATS;
                } else if (value > CTRLM_MAX_IR_COMMAND_REPEATS) {
                    LOGWARN("IR_COMMAND_REPEATS value of %d is TOO LARGE -  setting to maximum.", value);
                    value = CTRLM_MAX_IR_COMMAND_REPEATS;
                }
                LOGINFO("IR_COMMAND_REPEATS set to %d.", value);
                iarmSettings.ir_command_repeats = (unsigned char)value;
                iarmSettings.available |= CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_SET_IR_COMMAND_REPEATS;
            }

            if (iarmSettings.available > 0)
            {
                // Make the IARM call to controlMgr
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_SET_VALUES, (void *)&iarmSettings, sizeof(iarmSettings));
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROL_SERVICE_SET_VALUES IARM_Bus_Call FAILED, res: %d.", (int)res);
                    status_code = STATUS_INVALID_STATE;
                }
                else
                {
                    if (iarmSettings.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                    {
                        LOGERR("ERROR - CONTROL_SERVICE_SET_VALUES FAILED, result: %d.", (int)iarmSettings.result);
                        status_code = STATUS_FAILURE;
                    }
                }
            }
            else
            {
                // No settings were found to make! We don't call controlMgr with nothing to set.
                LOGERR("ERROR - NO valid settings values were passed in!! Taking no action!");
                status_code = STATUS_INVALID_ARGUMENT;
            }

            return status_code;
        }

        StatusCode ControlService::getValues(JsonObject& settings)
        {
            ctrlm_main_iarm_call_control_service_settings_t iarmSettings;
            StatusCode                                      status_code = STATUS_OK;
            IARM_Result_t                                   res;

            memset((void*)&iarmSettings, 0, sizeof(iarmSettings));
            iarmSettings.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;

            // Make the IARM call to controlMgr to get the settings
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_GET_VALUES, (void *)&iarmSettings, sizeof(iarmSettings));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CONTROL_SERVICE_GET_VALUES IARM_Bus_Call FAILED, res: %d.", (int)res);
                status_code = STATUS_INVALID_STATE;
            }
            else
            {
                if (iarmSettings.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROL_SERVICE_GET_VALUES FAILED, result: %d.", (int)iarmSettings.result);
                    status_code = STATUS_FAILURE;
                }
                else
                {
                    // Translate the settings from the IARM structure, to the return JsonObject
                    settings["supportsASB"]         = JsonValue((bool)iarmSettings.asb_supported);
                    settings["enableASB"]           = JsonValue((bool)iarmSettings.asb_enabled);

                    settings["enableOpenChime"]     = JsonValue((bool)iarmSettings.open_chime_enabled);
                    settings["enableCloseChime"]    = JsonValue((bool)iarmSettings.close_chime_enabled);
                    settings["enablePrivacyChime"]  = JsonValue((bool)iarmSettings.privacy_chime_enabled);
                    settings["conversationalMode"]  = JsonValue((int)iarmSettings.conversational_mode);
                    settings["chimeVolume"]         = JsonValue((int)iarmSettings.chime_volume);
                    settings["irCommandRepeats"]    = JsonValue((int)iarmSettings.ir_command_repeats);
                }
            }

            return status_code;
        }

        StatusCode ControlService::startPairingMode(int mode, int restrictions)
        {
            ctrlm_main_iarm_call_control_service_pairing_mode_t iarmMode;
            ctrlm_network_id_t                                  rf4ceId;
            StatusCode                                          status_code = STATUS_OK;
            IARM_Result_t                                       res;

            // First, need to go get the network_id of the rf4ce network on this STB.
            if (!getRf4ceNetworkId(rf4ceId))
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return STATUS_INVALID_STATE;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            memset((void*)&iarmMode, 0, sizeof(iarmMode));
            iarmMode.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            iarmMode.network_id = rf4ceId;
            iarmMode.pairing_mode = (unsigned char)mode;
            iarmMode.restrict_by_remote = (unsigned char)restrictions;

            // Make the IARM call to controlMgr
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE, (void *)&iarmMode, sizeof(iarmMode));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CONTROL_SERVICE_START_PAIRING_MODE IARM_Bus_Call FAILED, res: %d.", (int)res);
                status_code = STATUS_INVALID_STATE;
            }
            else
            {
                if (iarmMode.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROL_SERVICE_START_PAIRING_MODE FAILED, result: %d.", (int)iarmMode.result);
                    status_code = STATUS_FAILURE;
                }
            }

            return status_code;
        }

        StatusCode ControlService::endPairingMode(int& bindStatus)
        {
            ctrlm_main_iarm_call_control_service_pairing_mode_t iarmMode;
            ctrlm_network_id_t                                  rf4ceId;
            StatusCode                                          status_code = STATUS_OK;
            IARM_Result_t                                       res;

            // First, need to go get the network_id of the rf4ce network on this STB.
            if (!getRf4ceNetworkId(rf4ceId))
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return STATUS_INVALID_STATE;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            memset((void*)&iarmMode, 0, sizeof(iarmMode));
            iarmMode.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            iarmMode.network_id = rf4ceId;

            // Make the IARM call to controlMgr
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_END_PAIRING_MODE, (void *)&iarmMode, sizeof(iarmMode));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CONTROL_SERVICE_END_PAIRING_MODE IARM_Bus_Call FAILED, res: %d.", (int)res);
                status_code = STATUS_INVALID_STATE;
            }
            else
            {
                if (iarmMode.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROL_SERVICE_END_PAIRING_MODE FAILED, result: %d.", (int)iarmMode.result);
                    status_code = STATUS_FAILURE;
                }
                else
                {
                    bindStatus = iarmMode.bind_status;
                }
            }

            return status_code;
        }

        StatusCode ControlService::findMyRemote(int timeOutPeriod, bool bOnlyLastUsed)
        {
            StatusCode      status_code = STATUS_OK;
            IARM_Result_t   res;
            // It is necessary to calculate, allocate, and free memory here, in order to deal with
            // the variable size nature of the rcu_reverse_cmd used in the bus call.
            int alert_flags_param_data_size = 1;
            int alert_duration_param_data_size = 2;
            int exsize = alert_flags_param_data_size + alert_duration_param_data_size - 1;      // minus 1 for the one param_data byte, already part of the structure.
            size_t totalsize = sizeof(ctrlm_main_iarm_call_rcu_reverse_cmd_t) + (size_t)exsize;
            unsigned char alert_flags = (CTRLM_RCU_ALERT_AUDIBLE | CTRLM_RCU_ALERT_VISUAL);     // Audio and visual flags both set
            unsigned short alert_duration = (unsigned short)timeOutPeriod * 1000;               // Alert duration (in milliseconds)

            ctrlm_main_iarm_call_rcu_reverse_cmd_t* pCmd = (ctrlm_main_iarm_call_rcu_reverse_cmd_t*)calloc(1, totalsize);
            if (pCmd == NULL)
            {
                LOGERR("ERROR - Failed to allocate bus call structure!");
                return STATUS_INVALID_STATE;
            }

            pCmd->api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
            pCmd->network_type = CTRLM_NETWORK_TYPE_RF4CE;
            pCmd->controller_id = (bOnlyLastUsed ? CTRLM_MAIN_CONTROLLER_ID_LAST_USED : CTRLM_MAIN_CONTROLLER_ID_ALL);
            pCmd->cmd = CTRLM_RCU_REVERSE_CMD_FIND_MY_REMOTE;
            pCmd->total_size = totalsize;
            pCmd->num_params = 2;

            pCmd->params_desc[0].param_id = CTRLM_RCU_FMR_ALERT_FLAGS_ID;
            pCmd->params_desc[0].size = alert_flags_param_data_size;
            memcpy(&(pCmd->param_data[0]), &alert_flags, sizeof(alert_flags));

            pCmd->params_desc[1].param_id = CTRLM_FIND_RCU_FMR_ALERT_DURATION_ID;
            pCmd->params_desc[1].size = alert_duration_param_data_size;
            memcpy(&(pCmd->param_data[1]), &alert_duration, sizeof(alert_duration));

            // Make the IARM bus call to controlMgr
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_REVERSE_CMD, (void *)pCmd, totalsize);
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_RCU_IARM_CALL_REVERSE_CMD IARM_Bus_Call FAILED, res: %d.", (int)res);
                status_code = STATUS_INVALID_STATE;
            }
            else
            {
                if (pCmd->result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_RCU_IARM_CALL_REVERSE_CMD FAILED, result: %d.", (int)pCmd->result);
                    if((pCmd->cmd_result == CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_CAPABLE) ||
                       (pCmd->cmd_result == CTRLM_RCU_REVERSE_CMD_DISABLED))
                    {
                        status_code = STATUS_FMR_NOT_SUPPORTED;
                        LOGINFO("CTRLM_RCU_IARM_CALL_REVERSE_CMD cmd_result: %d.", (int)pCmd->cmd_result);
                    }
                    else
                    {
                        status_code = STATUS_FAILURE;
                    }
                }
                else
                {
                    LOGINFO("CTRLM_RCU_IARM_CALL_REVERSE_CMD cmd_result: %d.", (int)pCmd->cmd_result);
                    // In case of failure cmd_result with success IARM result, change the status_code to error.
                    switch(pCmd->cmd_result)
                    {
                        case CTRLM_RCU_REVERSE_CMD_SUCCESS:                 status_code = STATUS_OK;                break;
                        case CTRLM_RCU_REVERSE_CMD_CONTROLLER_FOUND:        status_code = STATUS_OK;                break;

                        case CTRLM_RCU_REVERSE_CMD_FAILURE:                 status_code = STATUS_FAILURE;           break;
                        case CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_FOUND:    status_code = STATUS_INVALID_ARGUMENT;  break;
                        case CTRLM_RCU_REVERSE_CMD_USER_INTERACTION:        status_code = STATUS_FAILURE;           break;  // The bus call should NEVER return this!

                        case CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_CAPABLE:  status_code = STATUS_FMR_NOT_SUPPORTED; break;
                        case CTRLM_RCU_REVERSE_CMD_DISABLED:                status_code = STATUS_FMR_NOT_SUPPORTED; break;
                    }
                }
            }

            if (pCmd != NULL)
            {
                free(pCmd);
            }
            return status_code;
        }

        bool ControlService::canFindMyRemote()
        {
            bool result = false;

#if (CTRLM_MAIN_IARM_BUS_API_REVISION > 8)
            IARM_Result_t retval;
            ctrlm_main_iarm_call_control_service_can_find_my_remote_t call;
            call.network_type = CTRLM_NETWORK_TYPE_RF4CE;

            memset((void*)&call, 0, sizeof(call));
            call.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;

            // Make the IARM bus call to controlMgr
            retval = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE, (void *)&call, sizeof(call));
            if (retval != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE - IARM_Bus_Call FAILED, retval: %d.", (int)retval);
            }
            else
            {
                if (call.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE - FAILED, result: %d.", (int)call.result);
                }
                else
                {
                    result = (bool)call.is_supported;
                }
            }
#endif // CTRLM_MAIN_IARM_BUS_API_REVISION > 8

            return result;
        }

        eCheckRf4ceChipConnectivity ControlService::checkRf4ceChipConnectivity()
        {
            eCheckRf4ceChipConnectivity        result = RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR;

#if (CTRLM_MAIN_IARM_BUS_API_REVISION > 9)
            IARM_Result_t                      retval;
            ctrlm_main_iarm_call_chip_status_t call;
            ctrlm_network_id_t                 rf4ceId = CTRLM_MAIN_NETWORK_ID_INVALID;

            // Start by finding the network_id of the rf4ce network on this STB.
            if (!getRf4ceNetworkId(rf4ceId))
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            memset((void*)&call, 0, sizeof(call));
            call.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            call.network_id   = rf4ceId;

            // Make the IARM bus call to controlMgr
            retval = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET, (void *)&call, sizeof(call));
            if (retval != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET - IARM_Bus_Call FAILED, retval: <%d>.\n",
                         (int)retval);
                result = RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR;
            }
            else
            {
                if (call.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    if (call.result == CTRLM_IARM_CALL_RESULT_ERROR_NOT_SUPPORTED)
                    {
                        LOGWARN("CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET - Not Supported.\n");
                        result = RF4CE_CHIP_CONNECTIVITY_NOT_SUPPORTED;
                    }
                    else
                    {
                        LOGERR("ERROR - CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET - FAILED, result: <%d>.\n", (int)call.result);
                        result = RF4CE_CHIP_CONNECTIVITY_IARM_CALL_RESULT_ERROR;
                    }
                }
                else
                {
                    result = (eCheckRf4ceChipConnectivity)call.chip_connected;
                    LOGINFO("Chip connected <%d>.\n", (int)result);
                }
            }

#endif // CTRLM_MAIN_IARM_BUS_API_REVISION > 9

            return result;
        }
        // End private method implementations

        // Begin local private utility methods
        void ControlService::setApiVersionNumber(unsigned int apiVersionNumber)
        {
            LOGINFO("setting version: %d", (int)apiVersionNumber);
            m_apiVersionNumber = apiVersionNumber;
        }

        int ControlService::numericCtrlm2Int(ctrlm_key_code_t ctrlm_key)
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

        char* ControlService::getRemoteModel(char *remoteType)
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

        char* ControlService::getRemoteModelVersion(char *remoteType)
        {
            static char remoteModelVersion[3];
            char *temp;

            temp = strchr(remoteType, '-');
            remoteModelVersion[0] = 'v';
            if (temp != NULL)
            {
                remoteModelVersion[1] = temp[1];
            }
            else
            {
                remoteModelVersion[1] = '?';
            }
            remoteModelVersion[2] = '\0';

            return remoteModelVersion;
        }

        const char* ControlService::getPairingType(ctrlm_rcu_binding_type_t pairingType)
        {
            switch(pairingType)
            {
                case CTRLM_RCU_BINDING_TYPE_INTERACTIVE:
                    return "manual";
                case CTRLM_RCU_BINDING_TYPE_AUTOMATIC:
                    return "auto-bind";
                case CTRLM_RCU_BINDING_TYPE_BUTTON:
                    return "button-button";
                case CTRLM_RCU_BINDING_TYPE_SCREEN_BIND:
                    return "screen-bind";
                default:
                    return "invalid";
            }
        }

        bool ControlService::getIrRemoteUsage(ctrlm_main_iarm_call_ir_remote_usage_t&  irRemoteUsage)
        {
            IARM_Result_t                           res;

            // Get the all the IR remote use history from ControlMgr.
            memset((void*)&irRemoteUsage, 0, sizeof(irRemoteUsage));
            irRemoteUsage.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_IR_REMOTE_USAGE_GET, (void*)&irRemoteUsage, sizeof(irRemoteUsage));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - IR_REMOTE_USAGE_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (irRemoteUsage.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - IR_REMOTE_USAGE_GET FAILED, call_result: %d", (int)irRemoteUsage.result);
                    return false;
                }
            }

            return true;
        }

        bool ControlService::getRf4ceNetworkId(ctrlm_network_id_t& rf4ceId)
        {
            ctrlm_main_iarm_call_status_t   status;
            IARM_Result_t                   res;
            bool                            found = false;

            memset((void*)&status, 0, sizeof(status));
            status.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_STATUS_GET, (void*)&status, sizeof(status));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - STATUS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (status.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - STATUS_GET FAILED, call_result: %d", (int)status.result);
                    return false;
                }
            }
            for (int i = 0; i < status.network_qty; i++)
            {
                if (status.networks[i].type == CTRLM_NETWORK_TYPE_RF4CE)
                {
                    rf4ceId = status.networks[i].id;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return false;
            }
            else
            {
                LOGINFO("Found rf4ce network_id: %d.", (int)rf4ceId);
            }

            return true;
        }

        bool ControlService::getRf4ceNetworkStatus(ctrlm_main_iarm_call_network_status_t&  netStatus)
        {
            ctrlm_network_id_t              rf4ceId;
            IARM_Result_t                   res;

            // Start by finding the network_id of the rf4ce network on this STB.
            if (!getRf4ceNetworkId(rf4ceId))
            {
                LOGERR("ERROR - No RF4CE network_id found!!");
                return false;
            }

            // Now we can get the RF4CE network information.
            memset((void*)&netStatus, 0, sizeof(netStatus));
            netStatus.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            netStatus.network_id = rf4ceId;
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

        bool ControlService::getRf4ceStbData(JsonObject& stbData)
        {
            ctrlm_main_iarm_call_network_status_t   netStatus;
            ctrlm_main_iarm_call_pairing_metrics_t  pairMetrics;
            ctrlm_main_iarm_call_ir_remote_usage_t  irRemoteUsage;
            IARM_Result_t                           res;

            char strMACAddress[IEEE_MAC_ADDRESS_STR_MAX] = { 0 };
            char strRemoteType[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH] = { 0 };

            // At this time, stbData consists of RF4CE info and pairing statistics.
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
            sprintf(strMACAddress, "0x%016llX", netStatus.status.rf4ce.ieee_address);
            stbData["stbRf4ceMACAddress"]   = std::string(strMACAddress);
            stbData["stbRf4ceSocMfr"]       = std::string(netStatus.status.rf4ce.chipset);
            stbData["stbHALVersion"]        = std::string(netStatus.status.rf4ce.version_hal);
            stbData["stbRf4ceShortAddress"] = JsonValue((int)netStatus.status.rf4ce.short_address);
            stbData["stbPanId"]             = JsonValue((int)netStatus.status.rf4ce.pan_id);
            stbData["stbActiveChannel"]     = JsonValue((int)netStatus.status.rf4ce.rf_channel_active.number);
            stbData["stbNumPairedRemotes"]  = JsonValue((int)netStatus.status.rf4ce.controller_qty);

            // Get the ctrlm pairing metrics information, and add it to the stbData
            memset((void*)&pairMetrics, 0, sizeof(pairMetrics));
            pairMetrics.api_revision = CTRLM_MAIN_IARM_BUS_API_REVISION;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_CALL_PAIRING_METRICS_GET, (void*)&pairMetrics, sizeof(pairMetrics));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("ERROR - PAIRING_METRICS_GET IARM_Bus_Call FAILED, res: %d", (int)res);
                return false;
            }
            else
            {
                if (pairMetrics.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - PAIRING_METRICS_GET FAILED, call_result: %d", (int)pairMetrics.result);
                    return false;
                }
                else
                {
                    LOGINFO("STB Pairing Metrics - num_screenbind_failures: %lu, last_screenbind_error_timestamp: %lu, last_screenbind_error_code: %u, "
                            "last_screenbind_remote_type: %s, num_non_screenbind_failures: %lu, last_non_screenbind_error_timestamp: %lu, "
                            "last_non_screenbind_error_code: %u, last_non_screenbind_error_binding_type: %u, last_non_screenbind_remote_type: %s. \n",
                            pairMetrics.num_screenbind_failures, pairMetrics.last_screenbind_error_timestamp, pairMetrics.last_screenbind_error_code,
                            pairMetrics.last_screenbind_remote_type, pairMetrics.num_non_screenbind_failures, pairMetrics.last_non_screenbind_error_timestamp,
                            pairMetrics.last_non_screenbind_error_code, pairMetrics.last_non_screenbind_error_binding_type, pairMetrics.last_non_screenbind_remote_type);

                    stbData["stbNumScreenBindFailures"]         = JsonValue((int)pairMetrics.num_screenbind_failures);
                    stbData["stbLastScreenBindErrorCode"]       = JsonValue((int)pairMetrics.last_screenbind_error_code);
                    strncpy(strRemoteType, pairMetrics.last_screenbind_remote_type, CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH);
                    strRemoteType[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH - 1] = '\0';
                    stbData["stbLastScreenBindErrorRemoteType"] = std::string(strRemoteType);
                    stbData["stbLastScreenBindErrorTimestamp"]  = JsonValue((long long)(pairMetrics.last_screenbind_error_timestamp * 1000LL));

                    stbData["stbNumOtherBindFailures"]          = JsonValue((int)pairMetrics.num_non_screenbind_failures);
                    stbData["stbLastOtherBindErrorCode"]        = JsonValue((int)pairMetrics.last_non_screenbind_error_code);
                    strncpy(strRemoteType, pairMetrics.last_non_screenbind_remote_type, CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH);
                    strRemoteType[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH - 1] = '\0';
                    stbData["stbLastOtherBindErrorRemoteType"]  = std::string(strRemoteType);
                    stbData["stbLastOtherBindErrorBindType"]    = JsonValue((int)pairMetrics.last_non_screenbind_error_binding_type);
                    stbData["stbLastOtherBindErrorTimestamp"]   = JsonValue((long long)(pairMetrics.last_non_screenbind_error_timestamp * 1000LL));
                }
            }

            // Get the IR remote use information.  We boil this down to only generic remote data (API 6).
            if (!getIrRemoteUsage(irRemoteUsage))
            {
                LOGERR("ERROR - IR_REMOTE_USAGE_GET FAILED!!");
                return false;
            }
            else
            {
                bool bHasIrRemotePreviousDay = false;
                bool bHasIrRemoteCurrentDay = false;
                if ( irRemoteUsage.has_ir_xr2_yesterday || irRemoteUsage.has_ir_xr5_yesterday || irRemoteUsage.has_ir_xr11_yesterday ||
                     irRemoteUsage.has_ir_xr15_yesterday || irRemoteUsage.has_ir_remote_yesterday )
                {
                    bHasIrRemotePreviousDay = true;
                }
                if ( irRemoteUsage.has_ir_xr2_today || irRemoteUsage.has_ir_xr5_today || irRemoteUsage.has_ir_xr11_today ||
                     irRemoteUsage.has_ir_xr15_today || irRemoteUsage.has_ir_remote_today )
                {
                    bHasIrRemoteCurrentDay = true;
                }
                stbData["bHasIrRemotePreviousDay"]  = JsonValue(bHasIrRemotePreviousDay);
                stbData["bHasIrRemoteCurrentDay"]   = JsonValue(bHasIrRemoteCurrentDay);
            }

            return true;
        } // End getStbData()

        bool ControlService::getRf4ceBindRemote(JsonObject& remoteInfo, ctrlm_rcu_iarm_call_controller_status_t& ctrlStatus)
        {
            IARM_Result_t   res;
            char            strMACAddress[IEEE_MAC_ADDRESS_STR_MAX] = { 0 };

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
            remoteInfo["remoteId"]                  = JsonValue((int)ctrlStatus.controller_id);
            sprintf(strMACAddress, "0x%016llX", ctrlStatus.status.ieee_address);
            remoteInfo["remoteMACAddress"]          = std::string(strMACAddress);
            remoteInfo["remoteModel"]               = std::string(getRemoteModel(ctrlStatus.status.type));
            remoteInfo["remoteModelVersion"]        = std::string(getRemoteModelVersion(ctrlStatus.status.type));
            remoteInfo["howRemoteIsPaired"]         = std::string(getPairingType(ctrlStatus.status.binding_type));
            remoteInfo["pairingTimestamp"]          = JsonValue((long long)(ctrlStatus.status.time_binding * 1000LL));
            remoteInfo["batteryLevelLoaded"]        = JsonValue(std::to_string(ctrlStatus.status.battery_voltage_loaded));  // TODO: Fix problem with FP parameters
            remoteInfo["batteryLevelUnloaded"]      = JsonValue(std::to_string(ctrlStatus.status.battery_voltage_unloaded));  // TODO: Fix problem with FP parameters
            remoteInfo["batteryLevelPercentage"]    = JsonValue((int)ctrlStatus.status.battery_level_percent);
            remoteInfo["batteryLastEvent"]          = JsonValue((int)ctrlStatus.status.battery_event);
            remoteInfo["batteryLastEventTimestamp"] = JsonValue((long long)(ctrlStatus.status.time_battery_update * 1000LL));

            remoteInfo["numVoiceCommandsPreviousDay"]        = JsonValue((int)ctrlStatus.status.voice_cmd_count_yesterday);
            remoteInfo["numVoiceCommandsCurrentDay"]         = JsonValue((int)ctrlStatus.status.voice_cmd_count_today);
            remoteInfo["numVoiceShortUtterancesPreviousDay"] = JsonValue((int)ctrlStatus.status.voice_cmd_short_yesterday);
            remoteInfo["numVoiceShortUtterancesCurrentDay"]  = JsonValue((int)ctrlStatus.status.voice_cmd_short_today);

            remoteInfo["numVoicePacketsSentPreviousDay"]  = JsonValue((int)ctrlStatus.status.voice_packets_sent_yesterday);
            remoteInfo["numVoicePacketsSentCurrentDay"]   = JsonValue((int)ctrlStatus.status.voice_packets_sent_today);
            remoteInfo["numVoicePacketsLostPreviousDay"]  = JsonValue((int)ctrlStatus.status.voice_packets_lost_yesterday);
            remoteInfo["numVoicePacketsLostCurrentDay"]   = JsonValue((int)ctrlStatus.status.voice_packets_lost_today);
            remoteInfo["aveVoicePacketLossPreviousDay"]   = JsonValue(std::to_string(ctrlStatus.status.voice_packet_loss_average_yesterday));   // TODO: Fix problem with FP parameters
            remoteInfo["aveVoicePacketLossCurrentDay"]    = JsonValue(std::to_string(ctrlStatus.status.voice_packet_loss_average_today));       // TODO: Fix problem with FP parameters
            remoteInfo["numVoiceCmdsHighLossPreviousDay"] = JsonValue((int)ctrlStatus.status.utterances_exceeding_packet_loss_threshold_yesterday);
            remoteInfo["numVoiceCmdsHighLossCurrentDay"]  = JsonValue((int)ctrlStatus.status.utterances_exceeding_packet_loss_threshold_today);

            remoteInfo["lastRebootErrorCode"]   = JsonValue((int)ctrlStatus.status.reboot_reason);
            remoteInfo["lastRebootTimestamp"]   = JsonValue((long long)(ctrlStatus.status.reboot_timestamp * 1000LL));

            remoteInfo["versionInfoSw"]     = std::string(ctrlStatus.status.version_software);
            remoteInfo["versionInfoHw"]     = std::string(ctrlStatus.status.version_hardware);
            remoteInfo["versionInfoIrdb"]   = std::string(ctrlStatus.status.version_irdb);

            remoteInfo["irdbType"]              = JsonValue((int)ctrlStatus.status.ir_db_type);
            remoteInfo["irdbState"]             = JsonValue((int)ctrlStatus.status.ir_db_state);
            remoteInfo["programmedTvIRCode"]    = std::string(ctrlStatus.status.ir_db_code_tv);
            remoteInfo["programmedAvrIRCode"]   = std::string(ctrlStatus.status.ir_db_code_avr);

            remoteInfo["bHasRemoteBeenUpdated"] = JsonValue((bool)ctrlStatus.status.firmware_updated);
            remoteInfo["lastCommandTimeDate"]   = JsonValue((long long)(ctrlStatus.status.time_last_key * 1000LL));
            remoteInfo["rf4ceRemoteSocMfr"]     = std::string(ctrlStatus.status.chipset);
            remoteInfo["remoteMfr"]             = std::string(ctrlStatus.status.manufacturer);

            remoteInfo["signalStrengthPercentage"]  = JsonValue((int)ctrlStatus.status.link_quality_percent);
            remoteInfo["linkQuality"]               = JsonValue((int)ctrlStatus.status.link_quality);
            remoteInfo["bHasCheckedIn"]             = JsonValue((bool)ctrlStatus.status.checkin_for_device_update);
            remoteInfo["bIrdbDownloadSupported"]    = JsonValue((bool)ctrlStatus.status.ir_db_code_download_supported);
            remoteInfo["securityType"]              = JsonValue((int)ctrlStatus.status.security_type);

            remoteInfo["bHasBattery"]                       = JsonValue((bool)ctrlStatus.status.has_battery);
            if((bool)ctrlStatus.status.has_battery)
            {
                remoteInfo["batteryChangedTimestamp"]           = JsonValue((long long)(ctrlStatus.status.time_battery_changed * 1000LL));
                remoteInfo["batteryChangedActualPercentage"]    = JsonValue((int)ctrlStatus.status.battery_changed_actual_percentage);
                remoteInfo["batteryChangedUnloadedVoltage"]     = JsonValue(std::to_string(ctrlStatus.status.battery_changed_unloaded_voltage));
                remoteInfo["battery75PercentTimestamp"]         = JsonValue((long long)(ctrlStatus.status.time_battery_75_percent * 1000LL));
                remoteInfo["battery75PercentActualPercentage"]  = JsonValue((int)ctrlStatus.status.battery_75_percent_actual_percentage);
                remoteInfo["battery75PercentUnloadedVoltage"]   = JsonValue(std::to_string(ctrlStatus.status.battery_75_percent_unloaded_voltage));
                remoteInfo["battery50PercentTimestamp"]         = JsonValue((long long)(ctrlStatus.status.time_battery_50_percent * 1000LL));
                remoteInfo["battery50PercentActualPercentage"]  = JsonValue((int)ctrlStatus.status.battery_50_percent_actual_percentage);
                remoteInfo["battery50PercentUnloadedVoltage"]   = JsonValue(std::to_string(ctrlStatus.status.battery_50_percent_unloaded_voltage));
                remoteInfo["battery25PercentTimestamp"]         = JsonValue((long long)(ctrlStatus.status.time_battery_25_percent * 1000LL));
                remoteInfo["battery25PercentActualPercentage"]  = JsonValue((int)ctrlStatus.status.battery_25_percent_actual_percentage);
                remoteInfo["battery25PercentUnloadedVoltage"]   = JsonValue(std::to_string(ctrlStatus.status.battery_25_percent_unloaded_voltage));
                remoteInfo["battery5PercentTimestamp"]          = JsonValue((long long)(ctrlStatus.status.time_battery_5_percent * 1000LL));
                remoteInfo["battery5PercentActualPercentage"]   = JsonValue((int)ctrlStatus.status.battery_5_percent_actual_percentage);
                remoteInfo["battery5PercentUnloadedVoltage"]    = JsonValue(std::to_string(ctrlStatus.status.battery_5_percent_unloaded_voltage));
                remoteInfo["battery0PercentTimestamp"]          = JsonValue((long long)(ctrlStatus.status.time_battery_0_percent * 1000LL));
                remoteInfo["battery0PercentActualPercentage"]   = JsonValue((int)ctrlStatus.status.battery_0_percent_actual_percentage);
                remoteInfo["battery0PercentUnloadedVoltage"]    = JsonValue(std::to_string(ctrlStatus.status.battery_0_percent_unloaded_voltage));
                remoteInfo["batteryVoltageLargeJumpCounter"]    = JsonValue((int)ctrlStatus.status.battery_voltage_large_jump_counter);
                remoteInfo["batteryVoltageLargeDeclineDetected"] = JsonValue((bool)ctrlStatus.status.battery_voltage_large_decline_detected);
            }

            remoteInfo["bHasDSP"]                        = JsonValue((bool)ctrlStatus.status.has_dsp);
            if((bool)ctrlStatus.status.has_dsp)
            {
                remoteInfo["averageTimeInPrivacyMode"]       = JsonValue((long long)ctrlStatus.status.average_time_in_privacy_mode);
                remoteInfo["bInPrivacyMode"]                 = JsonValue((bool)ctrlStatus.status.in_privacy_mode);
                remoteInfo["averageSNR"]                     = JsonValue((int)ctrlStatus.status.average_snr);
                remoteInfo["averageKeywordConfidence"]       = JsonValue((int)ctrlStatus.status.average_keyword_confidence);
                remoteInfo["totalNumberOfMicsWorking"]       = JsonValue((int)ctrlStatus.status.total_number_of_mics_working);
                remoteInfo["totalNumberOfSpeakersWorking"]   = JsonValue((int)ctrlStatus.status.total_number_of_speakers_working);
                remoteInfo["endOfSpeechInitialTimeoutCount"] = JsonValue((int)ctrlStatus.status.end_of_speech_initial_timeout_count);
                remoteInfo["endOfSpeechTimeoutCount"]        = JsonValue((int)ctrlStatus.status.end_of_speech_timeout_count);
                remoteInfo["uptimeStartTime"]                = JsonValue((long long)ctrlStatus.status.time_uptime_start * 1000LL);
                remoteInfo["uptimeInSeconds"]                = JsonValue((long long)ctrlStatus.status.uptime_seconds);
                remoteInfo["privacyTimeInSeconds"]           = JsonValue((long long)ctrlStatus.status.privacy_time_seconds);
                remoteInfo["versionDSPBuildId"]              = std::string(ctrlStatus.status.version_dsp_build_id);
            }

            LOGINFO("controller_id: %d, type: %s, SW version: %s, HW version: %s, time_last_key: %lu.",
                    (int)ctrlStatus.controller_id, ctrlStatus.status.type,
                    ctrlStatus.status.version_software, ctrlStatus.status.version_hardware, ctrlStatus.status.time_last_key);

            return true;
        } // End getRf4ceBindRemote()

        bool ControlService::getAllRf4ceBindRemotes()   // Results are in m_remoteInfo array
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

            if (netStatus.status.rf4ce.controller_qty == 0)
            {
                LOGWARN("No RF4CE controllers found!");
                m_numOfBindRemotes = 0;
                return true;
            }
            // Make sure we don't overrrun the m_remoteInfo array.
            if (netStatus.status.rf4ce.controller_qty > CTRLM_MAIN_MAX_BOUND_CONTROLLERS)
            {
                LOGERR("LOGIC ERROR - controller_qty(%d) greater than maximum(%d) - limiting to max!!!",
                       (int)netStatus.status.rf4ce.controller_qty, CTRLM_MAIN_MAX_BOUND_CONTROLLERS);
                netStatus.status.rf4ce.controller_qty = CTRLM_MAIN_MAX_BOUND_CONTROLLERS;
            }

            // There are one or more controllers paired on the rf4ce network.
            // Get the status for each one, and put them in the m_remoteInfo array.
            m_numOfBindRemotes = 0;
            for (int i = 0; i < netStatus.status.rf4ce.controller_qty; i++)
            {
                memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
                ctrlStatus.api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
                ctrlStatus.network_id = netStatus.network_id;
                ctrlStatus.controller_id = netStatus.status.rf4ce.controllers[i];

                if (!getRf4ceBindRemote(m_remoteInfo[i], ctrlStatus))
                {
                    LOGERR("ERROR - controller_status for remoteId %d NOT found!", (int)ctrlStatus.controller_id);
                    return false;
                }
            }
            // Save how many bound remotes there are in the array.
            m_numOfBindRemotes = netStatus.status.rf4ce.controller_qty;

            return true;
        }

        bool ControlService::getLastPairedRf4ceBindRemote(JsonObject& remoteInfo)
        {
            ctrlm_main_iarm_call_network_status_t   netStatus;
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatus;
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatusLastPaired;
            IARM_Result_t                           res;
            unsigned long                           pairingTime = 0;

            // Get the status of all the paired remotes on the rf4ce network.
            // Start by getting the network status for the rf4ce network on this STB.
            if (!getRf4ceNetworkStatus(netStatus))
            {
                LOGERR("ERROR - getRf4ceNetworkStatus() failed!!");
                return false;
            }

            if (netStatus.status.rf4ce.controller_qty == 0)
            {
                LOGERR("ERROR - No RF4CE controllers found!");
                return false;
            }
            // Make sure we don't overrrun the m_remoteInfo array.
            if (netStatus.status.rf4ce.controller_qty > CTRLM_MAIN_MAX_BOUND_CONTROLLERS)
            {
                LOGERR("LOGIC ERROR - controller_qty(%d) greater than maximum(%d) - limiting to max!!!",
                       (int)netStatus.status.rf4ce.controller_qty, CTRLM_MAIN_MAX_BOUND_CONTROLLERS);
                netStatus.status.rf4ce.controller_qty = CTRLM_MAIN_MAX_BOUND_CONTROLLERS;
            }

            // There are one or more controllers paired on the rf4ce network.
            // Get the status for each one, and find the one that was most recently paired.
            for (int i = 0; i < netStatus.status.rf4ce.controller_qty; i++)
            {
                memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
                ctrlStatus.api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
                ctrlStatus.network_id = netStatus.network_id;
                ctrlStatus.controller_id = netStatus.status.rf4ce.controllers[i];

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
                        // Determine which controller_status has the most recent time_binding, and save it.
                        if (pairingTime < ctrlStatus.status.time_binding)
                        {
                            pairingTime = ctrlStatus.status.time_binding;
                            ctrlStatusLastPaired = ctrlStatus;
                        }
                    }
                }
            }

            if (!getRf4ceBindRemote(remoteInfo, ctrlStatusLastPaired))
            {
                LOGERR("LOGIC ERROR - unknown failure for remoteId %d!", (int)ctrlStatusLastPaired.controller_id);
                return false;
            }

            return true;
        } // End getLastPairedRf4ceBindRemote()

        //End local private utility methods
    } // namespace Plugin
} // namespace WPEFramework
