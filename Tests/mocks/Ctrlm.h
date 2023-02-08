/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2014 RDK Management
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
*/
#ifndef _CTRLM_H_
#define _CTRLM_H_

#include "libIARM.h"
#include "libIBusDaemon.h"
#include "ctrlm_ipc_key_codes.h"

/// @file ctrlm_ipc.h
///
/// @defgroup CTRLM_IPC_MAIN IARM API - Control Manager
/// @{
///
/// @defgroup CTRLM_IPC_MAIN_COMMS       Communication Interfaces
/// @defgroup CTRLM_IPC_MAIN_CALLS       IARM Remote Procedure Calls
/// @defgroup CTRLM_IPC_MAIN_EVENTS      IARM Events
/// @defgroup CTRLM_IPC_MAIN_DEFINITIONS Constants
/// @defgroup CTRLM_IPC_MAIN_ENUMS       Enumerations
/// @defgroup CTRLM_IPC_MAIN_STRUCTS     Structures
///
/// @mainpage Control Manager IARM Interface
/// This document describes the interfaces that Control Manager uses to communicate with other components in the system.  It exposes
/// functionality to other processes in the system via the IARM bus. The IARM interface provides inter-process communication via Calls
/// and Events.  Control Manager supports one or more network devices via a Hardware Abstraction Layer (HAL).  The HAL API specifies the
/// method by which a network device communicates with the Control Manager.
///
/// This manual is divided into the following sections:
/// - @subpage CTRLM_IPC_MAIN_INTRO
/// - @subpage CTRLM_IPC_MAIN_COMMS
/// - @subpage CTRL_MGR
/// - @subpage CTRL_MGR_HAL
/// - @subpage CTRL_MGR_HAL_RF4CE
/// - @subpage CTRL_MGR_HAL_IP
/// - @subpage CTRL_MGR_HAL_BLE
///
/// @page CTRLM_IPC_MAIN_INTRO Introduction
/// The state of the Control Manager is summarized in the following diagram.
///
/// @dot
/// digraph CTRLMGR_StateMachine {
///     "INIT"  [shape="ellipse", fontname=Helvetica, fontsize=10];
///     "READY" [shape="ellipse", fontname=Helvetica, fontsize=10];
///     "TERM"  [shape="ellipse", fontname=Helvetica, fontsize=10];
///
///     "INIT"  -> "INIT"  [fontname=Helvetica, fontsize=10,label="  IARM Connect Failed (delay)"];
///     "INIT"  -> "READY" [fontname=Helvetica, fontsize=10,label="    "headlabel="IARM\rConnect",labeldistance=4.5, labelangle=25];
///     "READY" -> "INIT"  [fontname=Helvetica, fontsize=10,label="  IARM\l  Disconnect"];
///     "READY" -> "TERM"  [fontname=Helvetica, fontsize=10,label="  Control Manager Exited"];
///     "READY" -> "READY" [fontname=Helvetica, fontsize=10,label="  Process events and service calls"];
/// }
/// \enddot
///
///
/// Initialization
/// --------------
///
/// During initialization, the Control Manager initializes the IARM bus by calling IARM_Bus_Init() using its well known bus name, CTRLM_MAIN_IARM_BUS_NAME.
/// Upon successful initialization, the Control Manager connects to the bus by calling IARM_Bus_Connect().\n
///  If initialization is not completed successfully, the process will sleep for X seconds and retry the initialization procedure.
///
/// For debug purposes, the Control Manager may register to receive log messages from the IARM bus calling the IARM_Bus_RegisterForLog() function.
///
/// Termination
/// -----------
///
/// Before the Control Manager exits, it unregisters for events, disconnects from the IARM bus and terminates the session by calling IARM_Bus_UnRegisterEventHandler(), IARM_Bus_Disconnect()
/// and IARM_Bus_Term() respectively.
///
/// Normal Operation
/// ----------------
///
/// After successful initialization, the Control Manager will begin processing and generating @link CTRLM_IPC_MAIN_EVENTS events @endlink and @link CTRLM_IPC_MAIN_CALLS remote process calls @endlink (RPC).  Periodically, the status of the IARM bus connection is
/// checked by calling IARM_Bus_IsConnected().  If the result is not IARM_RESULT_SUCCESS, the Control Manager will return to the pre-initialized state.
///
/// @addtogroup CTRLM_IPC_MAIN_DEFINITIONS
/// @{
/// @brief Macros for constant values used by Control Manager clients
/// @details The Control Manager API provides macros for some parameters which may change in the future.  Clients should use
/// these names to allow the client code to function correctly if the values change.

#define CTRLM_MAIN_IARM_BUS_NAME                                 "Ctrlm"                                ///< Control Manager's IARM Bus Name
#define CTRLM_MAIN_IARM_BUS_API_REVISION                         (16)                                   ///< Revision of the Control Manager Main IARM API

#define CTRLM_MAIN_IARM_CALL_STATUS_GET                          "Main_StatusGet"                       ///< Retrieves Control Manager's Status information
#define CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET                  "Main_NetworkStatusGet"                ///< Retrieves the network's Status information
#define CTRLM_MAIN_IARM_CALL_PROPERTY_SET                        "Main_PropertySet"                     ///< Sets a property of the Control Manager
#define CTRLM_MAIN_IARM_CALL_PROPERTY_GET                        "Main_PropertyGet"                     ///< Gets a property of the Control Manager
#define CTRLM_MAIN_IARM_CALL_DISCOVERY_CONFIG_SET                "Main_DiscoveryConfigSet"              ///< Sets the discovery settings
#define CTRLM_MAIN_IARM_CALL_AUTOBIND_CONFIG_SET                 "Main_AutobindConfigSet"               ///< Sets the autobind settings
#define CTRLM_MAIN_IARM_CALL_PRECOMMISSION_CONFIG_SET            "Main_PrecommissionConfigSet"          ///< Sets the pre-commission settings
#define CTRLM_MAIN_IARM_CALL_FACTORY_RESET                       "Main_FactoryReset"                    ///< Sets the configuration back to factory default
#define CTRLM_MAIN_IARM_CALL_CONTROLLER_UNBIND                   "Main_ControllerUnbind"                ///< Removes a binding between the target and the specified controller
#define CTRLM_MAIN_IARM_CALL_IR_REMOTE_USAGE_GET                 "Main_IrRemoteUsageGet"                ///< Retrieves the ir remote usage info
#define CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET                   "Main_LastKeyInfoGet"                  ///< Retrieves the last key info
#define CTRLM_MAIN_IARM_CALL_LAST_KEYPRESS_GET                   "Main_LastKeyPressGet"                 ///< Retrieves the last key press (TODO: replace CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET with this)
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_SET_VALUES          "Main_ControlService_SetValues"        ///< IARM Call to set control service values
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_GET_VALUES          "Main_ControlService_GetValues"        ///< IARM Call to get control service values
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE  "Main_ControlService_CanFindMyRemote"  ///< IARM Call to get control service find my remote
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE  "Main_ControlService_StartPairingMode" ///< IARM Call to set control service start pairing mode
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_END_PAIRING_MODE    "Main_ControlService_EndPairingMode"   ///< IARM Call to set control service end pairing mode
#define CTRLM_MAIN_IARM_CALL_PAIRING_METRICS_GET                 "Main_PairingMetricsGet"               ///< Retrieves the stb's pairing metrics
#define CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET                     "Main_ChipStatusGet"                   ///< get Chip status
#define CTRLM_MAIN_IARM_CALL_AUDIO_CAPTURE_START                 "Main_AudioCaptureStart"               ///< Sends message to xraudio to capture mic data, in specified container
#define CTRLM_MAIN_IARM_CALL_AUDIO_CAPTURE_STOP                  "Main_AudioCaptureStop"                ///< Sends message to xraudio to stop capturing mic data
#define CTRLM_MAIN_IARM_CALL_POWER_STATE_CHANGE                  "Main_PowerStateChange"                ///< Sends message to xr-speech-router to set power state, download DSP firmware, etc
// IARM calls for the IR Database
#define CTRLM_MAIN_IARM_CALL_IR_CODES                            "Main_IRCodes"           ///< IARM Call to retrieve IR Codes based on type, manufacturer, and model
#define CTRLM_MAIN_IARM_CALL_IR_MANUFACTURERS                    "Main_IRManufacturers"   ///< IARM Call to retrieve list of manufacturers, based on (partial) name
#define CTRLM_MAIN_IARM_CALL_IR_MODELS                           "Main_IRModels"          ///< IARM Call to retrieve list of models, based on (partial) name
#define CTRLM_MAIN_IARM_CALL_IR_AUTO_LOOKUP                      "Main_IRAutoLookup"      ///< IARM Call to retrieve IR Codes based on EDID, Infoframe, and CEC
#define CTRLM_MAIN_IARM_CALL_IR_SET_CODE                         "Main_IRSetCode"         ///< IARM Call to set an IR Code into a specified BLE remote
#define CTRLM_MAIN_IARM_CALL_IR_CLEAR_CODE                       "Main_IRClear"           ///< IARM Call to clear all IR Codes from a specified BLE remote
#define CTRLM_MAIN_IARM_CALL_IR_INITIALIZE                       "Main_IRInitialize"      ///< IARM Call to initialize the IR database

// For Remote Plugin, only used for BLE currently, refactoring needed in other networks to use this interface
#define CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS                      "Main_GetRcuStatus"            ///< IARM Call get the RCU status info (same as what's provided by CTRLM_RCU_IARM_EVENT_RCU_STATUS)
#define CTRLM_MAIN_IARM_CALL_START_PAIRING                       "Main_StartPairing"            ///< IARM Call to initiate searching for a remote to pair with
#define CTRLM_MAIN_IARM_CALL_START_PAIR_WITH_CODE                "Main_StartPairWithCode"       ///< IARM Call to initiate searching for a remote to pair with
#define CTRLM_MAIN_IARM_CALL_FIND_MY_REMOTE                      "Main_FindMyRemote"            ///< IARM Call to trigger the Find My Remote alarm on a specified remote
#define CTRLM_MAIN_IARM_CALL_WRITE_RCU_WAKEUP_CONFIG             "Main_WriteAdvertisingConfig"  ///< IARM Call to write the advertising configuration on all connected remotes


#define CTRLM_MAIN_NETWORK_ID_INVALID                          (0xFF) ///< An invalid network identifier
#define CTRLM_MAIN_CONTROLLER_ID_INVALID                       (0xFF) ///< An invalid controller identifier
#define CTRLM_MAIN_CONTROLLER_ID_DSP                           (0)    ///< Default voice controller identifier
#define CTRLM_MAIN_NETWORK_ID_DSP                              (0)    ///< Controllers start at 1 so 0 is available for DSP

#define CTRLM_MAIN_NETWORK_ID_ALL                              (0xFE) ///< Indicates that the command applies to all networks
#define CTRLM_MAIN_CONTROLLER_ID_ALL                           (0xFE) ///< Indicates that the command applies to all networks

#define CTRLM_MAIN_CONTROLLER_ID_LAST_USED                     (0xFD) ///< An last used controller identifier

#define CTRLM_MAIN_VERSION_LENGTH                                (20) ///< Maximum length of the version string
#define CTRLM_MAIN_MAX_NETWORKS                                   (4) ///< Maximum number of networks
#define CTRLM_MAIN_MAX_BOUND_CONTROLLERS                          (9) ///< Maximum number of bound controllers
#define CTRLM_MAIN_MAX_CHIPSET_LENGTH                            (16) ///< Maximum length of chipset name string (including null termination)
#define CTRLM_MAIN_COMMIT_ID_MAX_LENGTH                          (48) ///< Maximum length of commit ID string (including null termination)
#define CTRLM_MAIN_RECEIVER_ID_MAX_LENGTH                        (40) ///< Maximum length of receiver ID string (including null termination)
#define CTRLM_MAIN_DEVICE_ID_MAX_LENGTH                          (24) ///< Maximum length of device ID string (including null termination)

#define CTRLM_PROPERTY_ACTIVE_PERIOD_BUTTON_VALUE_MIN               (5000) ///< Minimum active period (in ms) for button binding.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_BUTTON_VALUE_MAX             (600000) ///< Maximum active period (in ms) for button binding.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_SCREENBIND_VALUE_MIN           (5000) ///< Minimum active period (in ms) for screen bind.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_SCREENBIND_VALUE_MAX         (600000) ///< Maximum active period (in ms) for screen bind.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_ONE_TOUCH_AUTOBIND_VALUE_MIN   (5000) ///< Minimum active period (in ms) for screen bind.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_ONE_TOUCH_AUTOBIND_VALUE_MAX (600000) ///< Maximum active period (in ms) for screen bind.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_LINE_OF_SIGHT_VALUE_MIN        (5000) ///< Minimum active period (in ms) for line of sight.
#define CTRLM_PROPERTY_ACTIVE_PERIOD_LINE_OF_SIGHT_VALUE_MAX       (60000) ///< Maximum active period (in ms) for line of sight.

#define CTRLM_PROPERTY_VALIDATION_TIMEOUT_MIN                  (1000) ///< Validation timeout value minimum (in ms)
#define CTRLM_PROPERTY_VALIDATION_TIMEOUT_MAX                 (45000) ///< Validation timeout value maximum (in ms)
#define CTRLM_PROPERTY_VALIDATION_MAX_ATTEMPTS_MAX               (20) ///< Maximum number of validation attempts

#define CTRLM_PROPERTY_CONFIGURATION_TIMEOUT_MIN               (1000) ///< Configuration timeout value minimum (in ms)
#define CTRLM_PROPERTY_CONFIGURATION_TIMEOUT_MAX              (60000) ///< Configuration timeout value maximum (in ms)

#define CTRLM_AUTOBIND_THRESHOLD_MIN                              (1) ///< Autobind threshold minimum value
#define CTRLM_AUTOBIND_THRESHOLD_MAX                              (7) ///< Autobind threshold maximum value

#define CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH                        (20) ///< Maximum length of source name string (including null termination)

// Bitmask defines for setting the available value in ctrlm_main_iarm_call_control_service_settings_t
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_ASB_ENABLED                 (0x01) ///< Setting to enable/disable asb
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_OPEN_CHIME_ENABLED          (0x02) ///< Setting to enable/disable open chime
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_CLOSE_CHIME_ENABLED         (0x04) ///< Setting to enable/disable close chime
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_PRIVACY_CHIME_ENABLED       (0x08) ///< Setting to enable/disable privacy chime
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_CONVERSATIONAL_MODE         (0x10) ///< Setting for conversational mode (0-6)
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_SET_CHIME_VOLUME            (0x20) ///< Setting to set the chime volume
#define CTRLM_MAIN_CONTROL_SERVICE_SETTINGS_SET_IR_COMMAND_REPEATS      (0x40) ///< Setting to set the ir command repeats

#define CTRLM_MIN_CONVERSATIONAL_MODE (0)
#define CTRLM_MAX_CONVERSATIONAL_MODE (6)
#define CTRLM_MIN_IR_COMMAND_REPEATS  (1)
#define CTRLM_MAX_IR_COMMAND_REPEATS  (10)

#ifdef ASB
#define CTRLM_ASB_ENABLED_DEFAULT                 (false)
#endif
#define CTRLM_OPEN_CHIME_ENABLED_DEFAULT          (false)
#define CTRLM_CLOSE_CHIME_ENABLED_DEFAULT         (true)
#define CTRLM_PRIVACY_CHIME_ENABLED_DEFAULT       (true)
#define CTRLM_CONVERSATIONAL_MODE_DEFAULT         (CTRLM_MAX_CONVERSATIONAL_MODE)
#define CTRLM_CHIME_VOLUME_DEFAULT                (CTRLM_CHIME_VOLUME_MEDIUM)
#define CTRLM_IR_COMMAND_REPEATS_DEFAULT          (3)

#ifdef ASB
#define CTRLM_ASB_ENABLED_LEN                     (1)
#endif
#define CTRLM_OPEN_CHIME_ENABLED_LEN              (1)
#define CTRLM_CLOSE_CHIME_ENABLED_LEN             (1)
#define CTRLM_PRIVACY_CHIME_ENABLED_LEN           (1)
#define CTRLM_CONVERSATIONAL_MODE_LEN             (1)
#define CTRLM_CHIME_VOLUME_LEN                    (1)
#define CTRLM_IR_COMMAND_REPEATS_LEN              (1)

#define CTRLM_MAX_NUM_REMOTES             (4)
#define CTRLM_IEEE_ADDR_LEN               (18)
#define CTRLM_MAX_PARAM_STR_LEN           (64)
#define CTRLM_MAX_IRDB_RESPONSE_STR_LEN   (10240)

#define CTRLM_WAKEUP_CONFIG_LIST_MAX_SIZE (256)

/// @}

/// @addtogroup CTRLM_IPC_MAIN_ENUMS
/// @{
/// @brief Enumerated Types
/// @details The Control Manager provides enumerated types for logical groups of values.

/// @brief Remote Procedure Call Results
/// @details The structure for each remote call has a result member which is populated with the result of the operation.  This field is only valid
/// when the IARM return code indicates a successful call.
typedef enum {
   CTRLM_IARM_CALL_RESULT_SUCCESS                 = 0, ///< The requested operation was completed successfully.
   CTRLM_IARM_CALL_RESULT_ERROR                   = 1, ///< An error occurred during the requested operation.
   CTRLM_IARM_CALL_RESULT_ERROR_READ_ONLY         = 2, ///< An error occurred trying to write to a read-only entity.
   CTRLM_IARM_CALL_RESULT_ERROR_INVALID_PARAMETER = 3, ///< An input parameter is invalid.
   CTRLM_IARM_CALL_RESULT_ERROR_API_REVISION      = 4, ///< The API revision is invalid or no longer supported
   CTRLM_IARM_CALL_RESULT_ERROR_NOT_SUPPORTED     = 5, ///< The requested operation is not supported
   CTRLM_IARM_CALL_RESULT_INVALID                 = 6, ///< Invalid call result value
} ctrlm_iarm_call_result_t;

/// @brief Control Manager Properties
/// @details The properties enumeration is used in calls to @link CTRLM_IPC_MAIN_CALLS PropertyGet @endlink and @link CTRLM_IPC_MAIN_CALLS PropertySet@endlink.  They are used to get/set values in Control Manager.
typedef enum {
   CTRLM_PROPERTY_BINDING_BUTTON_ACTIVE            =  0, ///< (RO) Boolean value indicating whether a front panel button was recently pressed (1) or not (0).
   CTRLM_PROPERTY_BINDING_SCREEN_ACTIVE            =  1, ///< (RW) Boolean value indicating whether the 'Pairing Description Screen' is being displayed (1) or not (0).
   CTRLM_PROPERTY_BINDING_LINE_OF_SIGHT_ACTIVE     =  2, ///< (RO) Boolean value indicating whether the STB has received the Line of Sight remote command and is within the active period.
   CTRLM_PROPERTY_AUTOBIND_LINE_OF_SIGHT_ACTIVE    =  3, ///< (RO) Boolean value indicating that the STB has received the Autobind Line of Sight remote code and is within the active period.
   CTRLM_PROPERTY_ACTIVE_PERIOD_BUTTON             =  4, ///< (RW) Active period (in ms) for button binding.
   CTRLM_PROPERTY_ACTIVE_PERIOD_LINE_OF_SIGHT      =  5, ///< (RW) Active period (in ms) for line of sight.
   CTRLM_PROPERTY_VALIDATION_TIMEOUT_INITIAL       =  6, ///< (RW) Timeout value (in ms) used for the start of the validation period.
   CTRLM_PROPERTY_VALIDATION_TIMEOUT_DURING        =  7, ///< (RW) Timeout value (in ms) used during the validation period.
   CTRLM_PROPERTY_CONFIGURATION_TIMEOUT            =  8, ///< (RW) Timeout value (in ms) used during the configuration period.
   CTRLM_PROPERTY_VALIDATION_MAX_ATTEMPTS          =  9, ///< (RW) Maximum number of validation attempts.
   CTRLM_PROPERTY_ACTIVE_PERIOD_SCREENBIND         = 10, ///< (RW) Active period (in ms) for screenbind.
   CTRLM_PROPERTY_ACTIVE_PERIOD_ONE_TOUCH_AUTOBIND = 11, ///< (RW) Active period (in ms) for one-touch autobind.
   CTRLM_PROPERTY_REMOTE_REVERSE_CMD_ACTIVE        = 12, ///< (RW) Boolean value indicating whether the 'Remote Reverse Command' feature is enabled (1) or not (0).
   CTRLM_PROPERTY_MAC_POLLING_INTERVAL             = 13, ///< (RW) MAC polling polling interval, in milliseconds.
   CTRLM_PROPERTY_RCU_REVERSE_CMD_TIMEOUT          = 14, ///< (RW) Find My Remote RC response timeout, Factor of CTRLM_PROPERTY_MAC_POLLING_INTERVAL, min 2
   CTRLM_PROPERTY_AUTO_ACK                         = 15, ///< (RW) Boolean value indicating whether the 'Automatic Packet Acknowledgment' feature is enabled (1) or not (0).
   CTRLM_PROPERTY_MAX                              = 16, ///< (NA) Maximum property enumeration value.
} ctrlm_property_t;

/// @brief Control Manager Events
/// @details The events enumeration defines a value for each event that can be generated by Control Manager.
typedef enum {
   CTRLM_MAIN_IARM_EVENT_BINDING_BUTTON             =  0, ///< Generated when a state change of the binding button status occurs
   CTRLM_MAIN_IARM_EVENT_BINDING_LINE_OF_SIGHT      =  1, ///< Generated when a state change of the line of sight status occurs
   CTRLM_MAIN_IARM_EVENT_AUTOBIND_LINE_OF_SIGHT     =  2, ///< Generated when a state change of the autobind line of sight status occurs
   CTRLM_MAIN_IARM_EVENT_CONTROLLER_UNBIND          =  3, ///< Generated when a controller binding is removed
   CTRLM_RCU_IARM_EVENT_KEY_PRESS                   =  4, ///< Generated each time a key event occurs (down, repeat, up)
   CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN            =  5, ///< Generated at the beginning of a validation attempt
   CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_PRESS        =  6, ///< Generated when the user enters a validation code digit/letter
   CTRLM_RCU_IARM_EVENT_VALIDATION_END              =  7, ///< Generated at the end of a validation attempt
   CTRLM_RCU_IARM_EVENT_CONFIGURATION_COMPLETE      =  8, ///< Generated upon completion of controller configuration
   CTRLM_RCU_IARM_EVENT_FUNCTION                    =  9, ///< Generated when a function is performed on a controller
   CTRLM_RCU_IARM_EVENT_KEY_GHOST                   = 10, ///< Generated when a ghost code is received from a controller
   CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER       = 11, ///< Generated when a controller accesses a RIB entry
   CTRLM_VOICE_IARM_EVENT_SESSION_BEGIN             = 12, ///< Voice session began
   CTRLM_VOICE_IARM_EVENT_SESSION_END               = 13, ///< Voice session ended
   CTRLM_VOICE_IARM_EVENT_SESSION_RESULT            = 14, ///< Result of a voice session
   CTRLM_VOICE_IARM_EVENT_SESSION_STATS             = 15, ///< Statistics from a voice session
   CTRLM_VOICE_IARM_EVENT_SESSION_ABORT             = 16, ///< Voice session was aborted (denied)
   CTRLM_VOICE_IARM_EVENT_SESSION_SHORT             = 17, ///< Voice session did not meet minimum duration
   CTRLM_VOICE_IARM_EVENT_MEDIA_SERVICE             = 18, ///< Voice session results in media service event
   CTRLM_DEVICE_UPDATE_IARM_EVENT_READY_TO_DOWNLOAD = 19, ///< Indicates that a device has an update available
   CTRLM_DEVICE_UPDATE_IARM_EVENT_DOWNLOAD_STATUS   = 20, ///< Provides status of a download that is in progress
   CTRLM_DEVICE_UPDATE_IARM_EVENT_LOAD_BEGIN        = 21, ///< Indicates that a device has started to load an image
   CTRLM_DEVICE_UPDATE_IARM_EVENT_LOAD_END          = 22, ///< Indicates that a device has completed an image load
   CTRLM_RCU_IARM_EVENT_BATTERY_MILESTONE           = 23, ///< Indicates that a battery milestone event occured
   CTRLM_RCU_IARM_EVENT_REMOTE_REBOOT               = 24, ///< Indicates that a remote reboot event occured
   CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_BEGIN       = 25, ///< Indicates that a RCU Reverse Command started
   CTRLM_RCU_IARM_EVENT_RCU_REVERSE_CMD_END         = 26, ///< Indicates that a RCU Reverse Command ended 
   CTRLM_RCU_IARM_EVENT_CONTROL                     = 27, ///< Generated when a control event is received from a controller
   CTRLM_VOICE_IARM_EVENT_JSON_SESSION_BEGIN        = 28, ///< Generated on voice session begin, payload is JSON for consumption by Thunder Plugin
   CTRLM_VOICE_IARM_EVENT_JSON_STREAM_BEGIN         = 29, ///< Generated on voice stream begin, payload is JSON for consumption by Thunder Plugin
   CTRLM_VOICE_IARM_EVENT_JSON_KEYWORD_VERIFICATION = 30, ///< Generated on voice keyword verification, payload is JSON for consumption by Thunder Plugin
   CTRLM_VOICE_IARM_EVENT_JSON_SERVER_MESSAGE       = 31, ///< Generated on voice server message, payload is JSON for consumption by Thunder Plugin
   CTRLM_VOICE_IARM_EVENT_JSON_STREAM_END           = 32, ///< Generated on voice stream end, payload is JSON for consumption by Thunder Plugin
   CTRLM_VOICE_IARM_EVENT_JSON_SESSION_END          = 33, ///< Generated on voice session end, payload is JSON for consumption by Thunder Plugin
   CTRLM_RCU_IARM_EVENT_RCU_STATUS                  = 34, ///< Generated when someting changes in the BLE remote
   CTRLM_RCU_IARM_EVENT_RF4CE_PAIRING_WINDOW_TIMEOUT = 35, ///< Indicates that a battery milestone event occured
   CTRLM_MAIN_IARM_EVENT_MAX                        = 36  ///< Placeholder for the last event (used in registration)
} ctrlm_main_iarm_event_t;

/// @brief Remote Control Key Status
/// @details The status of the key.
typedef enum {
   CTRLM_KEY_STATUS_DOWN    = 0, ///< Key down
   CTRLM_KEY_STATUS_UP      = 1, ///< Key up
   CTRLM_KEY_STATUS_REPEAT  = 2, ///< Key repeat
   CTRLM_KEY_STATUS_INVALID = 3, ///< Invalid key status
} ctrlm_key_status_t;

/// @brief Data Access Type
/// @details The type of permission for access to data in Control Manager.
typedef enum {
   CTRLM_ACCESS_TYPE_READ    = 0, ///< Read access
   CTRLM_ACCESS_TYPE_WRITE   = 1, ///< Write access
   CTRLM_ACCESS_TYPE_INVALID = 2  ///< Invalid access type
} ctrlm_access_type_t;

/// @brief Network Type
/// @details The Control Manager network type.
typedef enum {
   CTRLM_NETWORK_TYPE_RF4CE        = 0,   ///< RF4CE Network
   CTRLM_NETWORK_TYPE_BLUETOOTH_LE = 1,   ///< Bluetooth Low Energy Network
   CTRLM_NETWORK_TYPE_IP           = 2,   ///< IP Network
   CTRLM_NETWORK_TYPE_DSP          = 3,   ///< DSP Network
   CTRLM_NETWORK_TYPE_INVALID      = 255  ///< Invalid Network
} ctrlm_network_type_t;

/// @brief Controller Unbind Reason
/// @details When a controller binding is removed, the reason is defined by the value in this enumeration.
typedef enum {
   CTRLM_UNBIND_REASON_CONTROLLER         = 0, ///< The controller initiated the unbind
   CTRLM_UNBIND_REASON_TARGET_USER        = 1, ///< The target initiated the unbind due to user request
   CTRLM_UNBIND_REASON_TARGET_NO_SPACE    = 2, ///< The target initiated the unbind due to lack of space in the pairing table
   CTRLM_UNBIND_REASON_FACTORY_RESET      = 3, ///< The target performed a factory reset
   CTRLM_UNBIND_REASON_CONTROLLER_RESET   = 4, ///< The controller performed a factory reset or RF reset
   CTRLM_UNBIND_REASON_INVALID_VALIDATION = 5, ///< A controller with an invalid validation was imported and needs to be unbinded
   CTRLM_UNBIND_REASON_MAX                = 6  ///< Maximum unbind reason value
} ctrlm_unbind_reason_t;

/// @brief Pairing Restrictions
/// @details The Control Manager network type.
typedef enum {
   CTRLM_PAIRING_RESTRICT_NONE                = 0,   ///< No restrictions on pairing
   CTRLM_PAIRING_RESTRICT_TO_VOICE_REMOTES    = 1,   ///< Only pair voice remotes
   CTRLM_PAIRING_RESTRICT_TO_VOICE_ASSISTANTS = 2,   ///< Only pair voice assistants
   CTRLM_PAIRING_RESTRICT_MAX                 = 3    ///< Maximum restriction value
} ctrlm_pairing_restrict_by_remote_t;

/// @brief Pairing Restrictions
/// @details The Control Manager network type.
typedef enum {
   CTRLM_PAIRING_MODE_BUTTON_BUTTON_BIND    = 0,   ///< Button Button binding
   CTRLM_PAIRING_MODE_SCREEN_BIND           = 1,   ///< Screen binding
   CTRLM_PAIRING_MODE_ONE_TOUCH_AUTO_BIND   = 2,   ///< One Touch Auto binding
   CTRLM_PAIRING_MODE_MAX                   = 3,   ///< Maximum pairing mode value
} ctrlm_pairing_modes_t;

/// @brief Pairing Restrictions
/// @details The Control Manager network type.
typedef enum
{
   CTRLM_BIND_STATUS_SUCCESS,
   CTRLM_BIND_STATUS_NO_DISCOVERY_REQUEST,
   CTRLM_BIND_STATUS_NO_PAIRING_REQUEST,
   CTRLM_BIND_STATUS_HAL_FAILURE,
   CTRLM_BIND_STATUS_CTRLM_BLACKOUT,
   CTRLM_BIND_STATUS_ASB_FAILURE,
   CTRLM_BIND_STATUS_STD_KEY_EXCHANGE_FAILURE,
   CTRLM_BIND_STATUS_PING_FAILURE,
   CTRLM_BIND_STATUS_VALILDATION_FAILURE,
   CTRLM_BIND_STATUS_RIB_UPDATE_FAILURE,
   CTRLM_BIND_STATUS_BIND_WINDOW_TIMEOUT,
   CTRLM_BIND_STATUS_UNKNOWN_FAILURE,
} ctrlm_bind_status_t;

/// @brief chime volume settings
/// @details The Control Manager network type.
typedef enum
{
   CTRLM_CHIME_VOLUME_LOW,
   CTRLM_CHIME_VOLUME_MEDIUM,
   CTRLM_CHIME_VOLUME_HIGH,
   CTRLM_CHIME_VOLUME_INVALID,
} ctrlm_chime_volume_t;

/// @brief IR device types
/// @details Types of IR devices supported by Control Manager
typedef enum {
   CTRLM_IR_DEVICE_TV = 0,
   CTRLM_IR_DEVICE_AMP,
   CTRLM_IR_DEVICE_UNKNOWN
} ctrlm_ir_device_type_t;

typedef enum {
   CTRLM_BLE_STATE_INITIALIZING = 0,        // starting up, no paired remotes
   CTRLM_BLE_STATE_IDLE,                    // no activity
   CTRLM_BLE_STATE_SEARCHING,               // device is searching for RCUs
   CTRLM_BLE_STATE_PAIRING,                 // device is pairing to an RCU
   CTRLM_BLE_STATE_COMPLETE,                // device successfully paired to an RCU
   CTRLM_BLE_STATE_FAILED,                  // device failed to find or pair to an RCU
   CTRLM_BLE_STATE_UNKNOWN                  // unknown status
} ctrlm_ble_state_t;

typedef enum {
   CTRLM_IR_STATE_IDLE,                    // no activity
   CTRLM_IR_STATE_WAITING,                 // IR programming in progress
   CTRLM_IR_STATE_COMPLETE,                // IR programming completed successfully
   CTRLM_IR_STATE_FAILED,                  // IR programming failed
   CTRLM_IR_STATE_UNKNOWN                  // unknown status
} ctrlm_ir_state_t;

typedef enum {
   CTRLM_FMR_DISABLE = 0,
   CTRLM_FMR_LEVEL_MID = 1,
   CTRLM_FMR_LEVEL_HIGH = 2
} ctrlm_fmr_alarm_level_t;

/// @brief audio capture container settings
/// @details Audio container types
typedef enum {
   CTRLM_AUDIO_CONTAINER_WAV     = 0,
   CTRLM_AUDIO_CONTAINER_NONE    = 1,
   CTRLM_AUDIO_CONTAINER_INVALID = 2
} ctrlm_audio_container_t;

/// @brief Power State Type
/// @details Power Manager sends the current power state and the new power state. This type is used to track the state information.
typedef enum {
   CTRLM_POWER_STATE_STANDBY                = 0,
   CTRLM_POWER_STATE_ON                     = 1,
   CTRLM_POWER_STATE_LIGHT_SLEEP            = 2,
   CTRLM_POWER_STATE_DEEP_SLEEP             = 3,
   CTRLM_POWER_STATE_STANDBY_VOICE_SESSION  = 4,
   CTRLM_POWER_STATE_INVALID                = 5 
}ctrlm_power_state_t;

typedef enum {
   CTRLM_RCU_WAKEUP_CONFIG_ALL = 0,
   CTRLM_RCU_WAKEUP_CONFIG_CUSTOM,
   CTRLM_RCU_WAKEUP_CONFIG_NONE,
   CTRLM_RCU_WAKEUP_CONFIG_INVALID
} ctrlm_rcu_wakeup_config_t;

/// @brief Network Id Type
/// @details During initialization, of the HAL network, Control Manager will assign a unique id to the network.  It must be used in all
/// subsequent communication with the Control Manager.
typedef unsigned char ctrlm_network_id_t;
/// @brief Controller Id Type
/// @details When a controller is paired, the Control Manager will assign an id (typically 48/64 bit MAC address) to the controller.
typedef unsigned char ctrlm_controller_id_t;

/// @}
/// @addtogroup CTRLM_IPC_MAIN_STRUCTS
/// @{
/// @brief Structure definitions
/// @details The Control Manager provides structures that are used in IARM calls and events.

/// @brief Network Structure
/// @details The information for a network.
typedef struct {
   ctrlm_network_id_t   id;   ///< identifier of the network
   ctrlm_network_type_t type; ///< Type of network
} ctrlm_network_t;

/// @brief Control Manager Status Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_STATUS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                                       ///< Revision of this API
   ctrlm_iarm_call_result_t result;                                             ///< OUT - The result of the operation.
   unsigned char            network_qty;                                        ///< OUT - Number of networks connected to Control Manager
   ctrlm_network_t          networks[CTRLM_MAIN_MAX_NETWORKS];                  ///< OUT - List of networks
   char                     ctrlm_version[CTRLM_MAIN_VERSION_LENGTH];           ///< OUT - Software version of Control Manager
   char                     ctrlm_commit_id[CTRLM_MAIN_COMMIT_ID_MAX_LENGTH];   ///< OUT - Last commit ID of Control Manager
   char                     stb_device_id[CTRLM_MAIN_DEVICE_ID_MAX_LENGTH];     ///< OUT - Device ID obtained from the Set-Top Box
   char                     stb_receiver_id[CTRLM_MAIN_RECEIVER_ID_MAX_LENGTH]; ///< OUT - Receiver ID obtained from the Set-Top Box
} ctrlm_main_iarm_call_status_t;

/// @brief RF Channel Structure
/// @details The diagnostics information for an RF channel.
typedef struct {
   unsigned char number;  ///< RF channel number (15, 20 or 25 for RF4CE)
   unsigned char quality; ///< Quality indicator for this channel
} ctrlm_rf_channel_t;

/// @brief RF4CE Network Status Structure
/// @details The RF4CE Network Status structure provided detailed information about the network.
typedef struct {
   char                  version_hal[CTRLM_MAIN_VERSION_LENGTH];        ///< Software version of the HAL driver
   unsigned char         controller_qty;                                ///< Number of controllers bound to the target device
   ctrlm_controller_id_t controllers[CTRLM_MAIN_MAX_BOUND_CONTROLLERS]; ///< List of controllers bound to the target device
   unsigned short        pan_id;                                        ///< PAN Identifier
   ctrlm_rf_channel_t    rf_channel_active;                             ///< Current RF channel on which the target is operating
   unsigned long long    ieee_address;                                  ///< The 64-bit IEEE Address of the target device
   unsigned short        short_address;                                 ///< Short address (if applicable)
   char                  chipset[CTRLM_MAIN_MAX_CHIPSET_LENGTH];        ///< Chipset of the target
} ctrlm_network_status_rf4ce_t;

/// @brief Bluetooth LE Network Status Structure
/// @details The Bluetooth LE Network Status structure provided detailed information about the network.
typedef struct {
   char                  version_hal[CTRLM_MAIN_VERSION_LENGTH];        ///< Software version of the HAL driver
   unsigned char         controller_qty;                                ///< Number of controllers bound to the target device.
   ctrlm_controller_id_t controllers[CTRLM_MAIN_MAX_BOUND_CONTROLLERS]; ///< List of controllers bound to the target device
} ctrlm_network_status_ble_t;

/// @brief IP Network Status Structure
/// @details The IP Network Status structure provided detailed information about the network.
typedef struct {
   char                  version_hal[CTRLM_MAIN_VERSION_LENGTH];        ///< Software version of the HAL driver
   unsigned char         controller_qty;                                ///< Number of controllers bound to the target device.
   ctrlm_controller_id_t controllers[CTRLM_MAIN_MAX_BOUND_CONTROLLERS]; ///< List of controllers bound to the target device
} ctrlm_network_status_ip_t;

/// @brief Network Status Structure
/// @details The Network Status structure is used in the CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision; ///< Revision of this API
   ctrlm_iarm_call_result_t result;       ///< OUT - Result of the operation
   ctrlm_network_id_t       network_id;   ///< IN - identifier of network
   union {
      ctrlm_network_status_rf4ce_t rf4ce; ///< OUT - RF4CE network status
      ctrlm_network_status_ble_t   ble;   ///< OUT - BLE network status
      ctrlm_network_status_ip_t    ip;    ///< OUT - IP network status
   } status;                              ///< OUT - Union of network status types
} ctrlm_main_iarm_call_network_status_t;

/// @brief Property Structure
/// @details The property structure is used in PropertyGet and PropertySet calls. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision; ///< Revision of this API
   ctrlm_iarm_call_result_t result;       ///< Result of the operation
   ctrlm_network_id_t       network_id;   ///< IN - identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
   ctrlm_property_t         name;         ///< Property name on which this call will operate
   unsigned long            value;        ///< Value for this property
} ctrlm_main_iarm_call_property_t;

/// @brief Discovery Config Call Structure
/// @details The Discovery Config Call structure is used in the CTRLM_IARM_CALL_DISCOVERY_CONFIG_SET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                 ///< Revision of this API
   ctrlm_iarm_call_result_t result;                       ///< Result of the operation
   ctrlm_network_id_t       network_id;                   ///< IN - identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
   unsigned char            enable;                       ///< Enable (1) or disable (0) open discovery
   unsigned char            require_line_of_sight;        ///< Require (1) or do not require (0) line of sight to respond to discovery requests
} ctrlm_main_iarm_call_discovery_config_t;

/// @brief Autobind Config Call Structure
/// @details The Autobind Config Call structure is used in the CTRLM_IARM_CALL_AUTOBIND_CONFIG_SET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                 ///< Revision of this API
   ctrlm_iarm_call_result_t result;                       ///< Result of the operation
   ctrlm_network_id_t       network_id;                   ///< IN - identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
   unsigned char            enable;                       ///< Enable (1) or disable (0) autobinding.
   unsigned char            threshold_pass;               ///< Number of successful pairing attempts required to complete autobinding successfully
   unsigned char            threshold_fail;               ///< Number of unsuccessful pairing attempts required to complete autobinding unsuccessfully
} ctrlm_main_iarm_call_autobind_config_t;

/// @brief Precommision Config Call Structure
/// @details The Precommission Config Call structure is used in the CTRLM_IARM_CALL_PRECOMMISSION_CONFIG_SET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                                  ///< Revision of this API
   ctrlm_iarm_call_result_t result;                                        ///< Result of the operation
   ctrlm_network_id_t       network_id;                                    ///< IN - identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
   unsigned long            controller_qty;                                ///< Number of precommissioned controllers
   unsigned long long       controllers[CTRLM_MAIN_MAX_BOUND_CONTROLLERS]; ///< IEEE Address for precommissioned controllers
} ctrlm_main_iarm_call_precommision_config_t;

/// @brief Factory Reset Call Structure
/// @details The Factory Reset Call structure is used in the CTRLM_IARM_CALL_FACTORY_RESET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision; ///< Revision of this API
   ctrlm_iarm_call_result_t result;       ///< Result of the operation
   ctrlm_network_id_t       network_id;   ///< IN - identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
} ctrlm_main_iarm_call_factory_reset_t;

/// @brief Controller Unbind Call Structure
/// @details This structure provides a method to remove a binding between the target and a controller.  For RF4CE controllers, only the target will be unbound since
/// there is no method to wake up the controller to unbind it.
typedef struct {
   unsigned char            api_revision;                           ///< Revision of this API
   ctrlm_iarm_call_result_t result;                                 ///< Result of the IARM call
   ctrlm_network_id_t       network_id;                             ///< IN - identifier of network on which the controller is bound
   ctrlm_controller_id_t    controller_id;                          ///< IN - identifier of the controller
} ctrlm_main_iarm_call_controller_unbind_t;

/// @brief Structure of Control Manager's Binding Button IARM event
/// @details This event notifies listeners that a state change has occurred in the binding button status.
typedef struct {
   unsigned char api_revision; ///< Revision of this API
   unsigned char active;       ///< Indicates that the binding button status is active (1) or not active (0)
} ctrlm_main_iarm_event_binding_button_t;

/// @brief Structure of Control Manager's Binding Line of Sight IARM event
/// @details This event notifies listeners that a state change has occurred in the binding line of sight status.
typedef struct {
   unsigned char api_revision; ///< Revision of this API
   unsigned char active;       ///< Indicates that the binding line of sight status is active (1) or not active (0)
} ctrlm_main_iarm_event_binding_line_of_sight_t;

/// @brief Structure of Control Manager's Autobind Line of Sight IARM event
/// @details This event notifies listeners that a state change has occurred in the autobind line of sight status.
typedef struct {
   unsigned char api_revision; ///< Revision of this API
   unsigned char active;       ///< Indicates that the autobind line of sight status is active (1) or not active (0)
} ctrlm_main_iarm_event_autobind_line_of_sight_t;

/// @brief Structure of Control Manager's Unbind IARM event
/// @details The unbind Event is generated whenever a controller is removed from the binding table.
typedef struct {
   unsigned char         api_revision;  ///< Revision of this API
   ctrlm_network_id_t    network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t  network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t controller_id; ///< Identifier of the controller
   ctrlm_unbind_reason_t reason;        ///< Reason that the controller binding was removed
} ctrlm_main_iarm_event_controller_unbind_t;

/// @brief Control Manager IR Remote Usage Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_IR_REMOTE_USAGE_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;             ///< Revision of this API
   ctrlm_iarm_call_result_t result;                   ///< OUT - The result of the operation.
   unsigned long            today;                    ///< OUT - The current day
   unsigned char            has_ir_xr2_yesterday;     ///< OUT - Boolean value indicating XR2 in IR mode was used the previous day
   unsigned char            has_ir_xr5_yesterday;     ///< OUT - Boolean value indicating XR5 in IR mode was used the previous day
   unsigned char            has_ir_xr11_yesterday;    ///< OUT - Boolean value indicating XR11 in IR mode was used the previous day
   unsigned char            has_ir_xr15_yesterday;    ///< OUT - Boolean value indicating XR15 in IR mode was used the previous day
   unsigned char            has_ir_xr2_today;         ///< OUT - Boolean value indicating XR2 in IR mode was used the current day
   unsigned char            has_ir_xr5_today;         ///< OUT - Boolean value indicating XR5 in IR mode was used the current day
   unsigned char            has_ir_xr11_today;        ///< OUT - Boolean value indicating XR11 in IR mode was used the current day
   unsigned char            has_ir_xr15_today;        ///< OUT - Boolean value indicating XR15 in IR mode was used the current day
   unsigned char            has_ir_remote_yesterday;  ///< OUT - Boolean value indicating remote in IR mode was used the previous day
   unsigned char            has_ir_remote_today;      ///< OUT - Boolean value indicating remote in IR mode was used the current day
} ctrlm_main_iarm_call_ir_remote_usage_t;

/// @brief Control Manager Last Key Info Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                                       ///< Revision of this API
   ctrlm_network_id_t       network_id;                                         ///< IN - identifier of network on which the controller is bound
   ctrlm_iarm_call_result_t result;                                             ///< OUT - The result of the operation.
   int                      controller_id;                                      ///< OUT - The controller id of the last key press.
   unsigned char            source_type;                                        ///< OUT - The source type of the last key press.
   unsigned long            source_key_code;                                    ///< OUT - The keycode of the last key press.
   long long                timestamp;                                          ///< OUT - The timestamp of the last key press.
   unsigned char            is_screen_bind_mode;                                ///< OUT - Indicates if the last key press is from a remote is in screen bind mode.
   unsigned char            remote_keypad_config;                               ///< OUT - The remote keypad configuration (Has Setup/NumberKeys).
   char                     source_name[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH];     ///< OUT - The source name of the last key press.
} ctrlm_main_iarm_call_last_key_info_t;

/// @brief Control Manager Control Service Settings Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_SET_VALUES call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char                api_revision;                                   ///< The revision of this API.
   ctrlm_iarm_call_result_t     result;                                         ///< Result of the IARM call
   unsigned long                available;                                      ///< Bitmask indicating the settings that are available in this event
   unsigned char                asb_supported;                                  ///< Read only Boolean value to indicate asb supported enable (non-zero) or not supported (zero) asb
   unsigned char                asb_enabled;                                    ///< Boolean value to enable (non-zero) or disable (zero) asb
   unsigned char                open_chime_enabled;                             ///< Boolean value to enable (non-zero) or disable (zero) open chime
   unsigned char                close_chime_enabled;                            ///< Boolean value to enable (non-zero) or disable (zero) close chime
   unsigned char                privacy_chime_enabled;                          ///< Boolean value to enable (non-zero) or disable (zero) privacy chime
   unsigned char                conversational_mode;                            ///< Boolean value to set conversational mode (0-6)
   ctrlm_chime_volume_t         chime_volume;                                   ///< The chime volume
   unsigned char                ir_command_repeats;                             ///< The ir command repeats (1 - 10)
} ctrlm_main_iarm_call_control_service_settings_t;

/// @brief Control Manager Control Service Can Find My Remote Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char                api_revision;                                   ///< The revision of this API.
   ctrlm_iarm_call_result_t     result;                                         ///< Result of the IARM call
   unsigned char                is_supported;                                   ///< Read only Boolean value to indicate if findMyRemote is supported enable (non-zero) or not supported (zero)
   ctrlm_network_type_t         network_type;                                   ///< [in]  Type of network on which the controller is bound
} ctrlm_main_iarm_call_control_service_can_find_my_remote_t;

/// @brief Control Manager Control Service Pairing Mode Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.

typedef struct {
   unsigned char                api_revision;                                   ///< The revision of this API.
   ctrlm_iarm_call_result_t     result;                                         ///< Result of the IARM call
   ctrlm_network_id_t           network_id;                                     ///< Identifier of network or CTRLM_MAIN_NETWORK_ID_ALL for all networks
   unsigned char                pairing_mode;                                   ///< Indicates the pairing mode
   unsigned char                restrict_by_remote;                             ///< Indicates the remote bucket (no restrictions, only voice remotes, only voice assistants)
   unsigned int                 bind_status;                                    ///< OUT - The bind status of the pairing session
} ctrlm_main_iarm_call_control_service_pairing_mode_t;

/// @brief Control Manager Pairing Metrics Structure
/// @details The Control Manager Status structure is used in the CTRLM_MAIN_IARM_CALL_PAIRING_METRICS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;                                                       ///< Revision of this API
   ctrlm_iarm_call_result_t result;                                                             ///< OUT - The result of the operation.
   unsigned long            num_screenbind_failures;                                            ///< OUT - The total number of screenbind failures on this stb
   unsigned long            last_screenbind_error_timestamp;                                    ///< OUT - Timestamp of the last screenbind error
   ctrlm_bind_status_t      last_screenbind_error_code;                                         ///< OUT - The last screenbind error code
   char                     last_screenbind_remote_type[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH];     ///< OUT - The last screenbind error remote type
   unsigned long            num_non_screenbind_failures;                                        ///< OUT - The total number of screenbind failures on this stb
   unsigned long            last_non_screenbind_error_timestamp;                                ///< OUT - Timestamp of the last screenbind error
   ctrlm_bind_status_t      last_non_screenbind_error_code;                                     ///< OUT - The last screenbind error code
   unsigned char            last_non_screenbind_error_binding_type;                             ///< OUT - The last screenbind error binding type
   char                     last_non_screenbind_remote_type[CTRLM_MAIN_SOURCE_NAME_MAX_LENGTH]; ///< OUT - The last screenbind error remote type
} ctrlm_main_iarm_call_pairing_metrics_t;


typedef struct {
   ctrlm_controller_id_t      controller_id;                               ///< identifier of the controller, used for calls to a specific RCU
   char                       ieee_address_str[CTRLM_MAX_PARAM_STR_LEN];
   char                       serialno[CTRLM_MAX_PARAM_STR_LEN];
   int                        deviceid;
   char                       make[CTRLM_MAX_PARAM_STR_LEN];
   char                       model[CTRLM_MAX_PARAM_STR_LEN];
   char                       name[CTRLM_MAX_PARAM_STR_LEN];
   char                       btlswver[CTRLM_MAX_PARAM_STR_LEN];
   char                       hwrev[CTRLM_MAX_PARAM_STR_LEN];
   char                       rcuswver[CTRLM_MAX_PARAM_STR_LEN];
   char                       tv_code[CTRLM_MAX_PARAM_STR_LEN];
   char                       avr_code[CTRLM_MAX_PARAM_STR_LEN];
   unsigned char              connected;
   int                        batterylevel;
   int                        wakeup_key_code;
   ctrlm_rcu_wakeup_config_t  wakeup_config;
   int                        wakeup_custom_list[CTRLM_WAKEUP_CONFIG_LIST_MAX_SIZE];
   int                        wakeup_custom_list_size;
} ctrlm_rcu_data_t;

// This struct is used for the event (CTRLM_RCU_IARM_EVENT_RCU_STATUS) and get (CTRLM_MAIN_IARM_CALL_GET_RCU_STATUS)
typedef struct {
   unsigned char              api_revision;                    ///< Revision of this API
   ctrlm_network_id_t         network_id;                      ///< IN - Identifier of network
   ctrlm_ble_state_t          status;
   ctrlm_ir_state_t           ir_state;
   int                        num_remotes;
   ctrlm_rcu_data_t           remotes[CTRLM_MAX_NUM_REMOTES];
   ctrlm_iarm_call_result_t   result;
} ctrlm_iarm_RcuStatus_params_t;

typedef struct {
   unsigned char            api_revision;       ///< Revision of this API
   ctrlm_network_id_t       network_id;         ///< IN - Identifier of network
   unsigned int             timeout;            ///< IN - The timeout in seconds. Set to default value in the HAL if 0
   ctrlm_iarm_call_result_t result;             ///< OUT - return code of the operation
} ctrlm_iarm_call_StartPairing_params_t;

typedef struct {
   unsigned char            api_revision;       ///< Revision of this API
   ctrlm_network_id_t       network_id;         ///< IN - Identifier of network
   unsigned int             pair_code;          ///< IN - Pairing code from device
   ctrlm_iarm_call_result_t result;             ///< OUT - return code of the operation
} ctrlm_iarm_call_StartPairWithCode_params_t;

typedef struct {
   unsigned char            api_revision;                      ///< Revision of this API
   ctrlm_network_id_t       network_id;                        ///< IN - Identifier of network
   ctrlm_controller_id_t    controller_id;                     ///< IN - Identifier of the controller
   ctrlm_fmr_alarm_level_t  level;                             ///< IN - volume of the alarm
   unsigned int             duration;                          ///< IN - duration
   ctrlm_iarm_call_result_t result;                            ///< OUT - return code of the operation
} ctrlm_iarm_call_FindMyRemote_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   ctrlm_ir_device_type_t   type;                                       ///< IN - device type, e.g. TV or AVR
   char                     manufacturer[CTRLM_MAX_PARAM_STR_LEN];      ///< IN - complete manufacturer name
   char                     model[CTRLM_MAX_PARAM_STR_LEN];             ///< IN - optional model
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - list of IR 5 digit codes matching input manufacturer and model. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRCodes_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   ctrlm_ir_device_type_t   type;                                       ///< IN - device type, e.g. TV or AVR
   char                     manufacturer[CTRLM_MAX_PARAM_STR_LEN];      ///< IN - partial manufacturer search string, search matches from start of word only, case insensitive
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - list manufacturers present in IR database matching input search string. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRManufacturers_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   ctrlm_ir_device_type_t   type;                                       ///< IN - device type, e.g. TV or AVR
   char                     manufacturer[CTRLM_MAX_PARAM_STR_LEN];      ///< IN - complete manufacturer name
   char                     model[CTRLM_MAX_PARAM_STR_LEN];             ///< IN - partial model search string, search matches from start of word only, case insensitive
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - list models present in IR database matching input manufacturer name and search string. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRModels_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - list of IR codes matching connected devices. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRAutoLookup_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   ctrlm_controller_id_t    controller_id;                              ///< IN - Identifier of the controller
   ctrlm_ir_device_type_t   type;                                       ///< IN - device type, e.g. TV or AVR
   char                     code[CTRLM_MAX_PARAM_STR_LEN];              ///< IN - code set to use
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - result of the operation. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRSetCode_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   ctrlm_controller_id_t    controller_id;                              ///< IN - Identifier of the controller
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - result of the operation. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_IRClear_params_t;

typedef struct {
   unsigned char            api_revision;                               ///< Revision of this API
   ctrlm_network_id_t       network_id;                                 ///< IN - Identifier of network
   char                     response[CTRLM_MAX_IRDB_RESPONSE_STR_LEN];  ///< OUT - result of the operation. Formatted in JSON
   ctrlm_iarm_call_result_t result;                                     ///< OUT - return code of the operation
} ctrlm_iarm_call_initialize_irdb_params_t;

typedef struct {
   unsigned char                 api_revision;
   ctrlm_network_id_t            network_id;
   ctrlm_controller_id_t         controller_id;
   ctrlm_rcu_wakeup_config_t     config;
   int                           customList[CTRLM_WAKEUP_CONFIG_LIST_MAX_SIZE];
   int                           customListSize;
   ctrlm_iarm_call_result_t      result;
} ctrlm_iarm_call_WriteRcuWakeupConfig_params_t;

/// @brief Chip Status Structure
/// @details The Chip Status structure is used in the CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;   ///< Revision of this API
   ctrlm_iarm_call_result_t result;         ///< OUT - Result of the operation
   ctrlm_network_id_t       network_id;     ///< IN - identifier of network
   unsigned char            chip_connected; ///< OUT - 1 - chip connected, 0 - chip disconnected
} ctrlm_main_iarm_call_chip_status_t;

/// @brief Control Manager Audio Capture Structure
/// @details The Control Manager Audio Capture structure is used in the CTRLM_MAIN_IARM_CALL_AUDIO_CAPTURE_START call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;
   ctrlm_iarm_call_result_t result;
   ctrlm_audio_container_t  container;
   char                     file_path[128];
   unsigned char            raw_mic_enable;
} ctrlm_main_iarm_call_audio_capture_t;

typedef struct {
   unsigned char api_revision;
   ctrlm_iarm_call_result_t result;
   ctrlm_power_state_t new_state;
} ctrlm_main_iarm_call_power_state_change_t;

/// @}

/// @addtogroup CTRLM_IPC_MAIN_EVENTS Events
/// @{
/// @brief Broadcast Events accessible via IARM bus
/// @details The iARM bus uses events to broadcast information to interested clients. An event is sent separately to each client. There are no return values for an event and no
/// guarantee that a client will receive the event.  Each event has a different argument structure according to the information being transferred to the clients.  The
/// Control Manager generates and subscribes to are detailed below.
///
/// - - -
/// Event Generation (Broadcast)
/// ----------------------------
///
/// The Control Manager generates events that can be received by other processes connected to the IARM bus. The following events
/// are registered during initialization:
///
/// | Bus Name                 | Event Name                                   | Argument                                       | Description |
/// | :-------                 | :---------                                   | :-------                                       | :---------- |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_MAIN_IARM_EVENT_BINDING_BUTTON         | ctrlm_main_iarm_event_binding_button_t         | Generated when a state change of the binding button status occurs |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_MAIN_IARM_EVENT_BINDING_LINE_OF_SIGHT  | ctrlm_main_iarm_event_binding_line_of_sight_t  | Generated when a state change of the line of sight status occurs |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_MAIN_IARM_EVENT_AUTOBIND_LINE_OF_SIGHT | ctrlm_main_iarm_event_autobind_line_of_sight_t | Generated when a state change of the autobind line of sight status occurs |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_MAIN_IARM_EVENT_CONTROLLER_UNBIND      | ctrlm_main_iarm_event_controller_unbind_t      | Generated when a controller binding is removed |
///
/// IARM events are available on a subscription basis. In order to receive an event, a client must explicitly register to receive the event by calling
/// IARM_Bus_RegisterEventHandler() with the Control Manager bus name, event name and a @link IARM_EventHandler_t handler function @endlink. Events may be generated at any time by the
/// Control Manager. All events are asynchronous.
///
/// Examples:
///
/// Register for a Control Manager event:
///
///     IARM_Result_t result;
///
///     result = IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_MAIN_IARM_EVENT_KEY_PRESS, key_handler_func);
///     if(IARM_RESULT_SUCCESS == result) {
///         // Event registration was set successful
///     }
///
/// @}
///
/// @addtogroup CTRLM_IPC_MAIN_CALLS Remote Procedure Calls
/// @{
/// @brief Remote Calls accessible via IARM bus
/// @details IARM calls are synchronous calls to functions provided by other IARM bus members. Each bus member can register
/// calls using the IARM_Bus_RegisterCall() function. Other members can invoke the call using the IARM_Bus_Call() function.
///
/// - - -
/// Call Registration
/// -----------------
///
/// The Control Manager registers the following calls.
///
/// | Bus Name                 | Call Name                                | Argument                                   | Description |
/// | :-------                 | :--------                                | :-------                                   | :---------- |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_STATUS_GET               | ctrlm_main_iarm_call_status_t              | Retrieves Control Manager's Status information |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_NETWORK_STATUS_GET       | ctrlm_main_iarm_call_network_status_t      | Retrieves the specified network's Status information |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_PROPERTY_SET             | ctrlm_main_iarm_call_property_t            | Sets a property of the Control Manager |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_PROPERTY_GET             | ctrlm_main_iarm_call_property_t            | Gets a property of the Control Manager |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_UNIQUE_REFERENCE_ID_SET  | ctrlm_main_iarm_call_unique_reference_id_t | Sets the unique reference id for the STB |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_AUTOBIND_CONFIG_SET      | ctrlm_main_iarm_call_autobind_config_t     | Sets the configuration settings for autobind |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_PRECOMMISSION_CONFIG_SET | ctrlm_main_iarm_call_precommision_config_t | Sets the pre-commission settings |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_FACTORY_RESET            | ctrlm_main_iarm_call_factory_reset_t       | Resets settings to factory default for the specified network |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_IARM_CALL_CONTROLLER_UNBIND        | ctrlm_main_iarm_call_controller_unbind_t   | Removes a controller from the specified network |
///
/// Examples:
///
/// Set a Control Manager property:
///
///     IARM_Result_t    result;
///     ctrlm_property_t property;
///     property.name  = CTRLM_PROPERTY_CLASS_DESC_LINE_OF_SIGHT;
///     property.value = 1;
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_IARM_CALL_PROPERTY_SET, (void *)&property, sizeof(property));
///     if(IARM_RESULT_SUCCESS == result && CTRLMGR_RESULT_SUCCESS == property.result) {
///         // Property was set successfully
///     }
///
/// Get a Control Manager property:
///
///     IARM_Result_t    result;
///     ctrlm_property_t property;
///     property.name  = CTRLM_PROPERTY_VALIDATION_MAX_ATTEMPTS;
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_IARM_CALL_PROPERTY_GET, (void *)&property, sizeof(property));
///     if(IARM_RESULT_SUCCESS == result && CTRLMGR_RESULT_SUCCESS == property.result) {
///         // Property was retrieved successfully
///     }
///
/// Get Network Status information:
///
///     IARM_Result_t     result;
///     ctrlm_rf_status_t status;
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_IARM_CALL_NETWORK_STATUS_GET, (void *)&status, sizeof(status));
///     if(IARM_RESULT_SUCCESS == result && CTRLMGR_RESULT_SUCCESS == status.result) {
///         // Process Status
///     }
///
/// @}


/// @}

/// @file ctrlm_ipc_voice.h
///
/// @defgroup CTRLM_IPC_VOICE IARM API - Voice
/// @{
///
/// @defgroup CTRLM_IPC_VOICE_COMMS       Communication Interfaces
/// @defgroup CTRLM_IPC_VOICE_CALLS       IARM Remote Procedure Calls
/// @defgroup CTRLM_IPC_VOICE_EVENTS      IARM Events
/// @defgroup CTRLM_IPC_VOICE_DEFINITIONS Constants
/// @defgroup CTRLM_IPC_VOICE_ENUMS       Enumerations
/// @defgroup CTRLM_IPC_VOICE_STRUCTS     Structures

/// @addtogroup CTRLM_IPC_VOICE_DEFINITIONS
/// @{
/// @brief Constant Defintions
/// @details The Control Manager provides definitions for constant values.

#define CTRLM_VOICE_IARM_CALL_UPDATE_SETTINGS      "Voice_UpdateSettings"     ///< IARM Call to update settings
#define CTRLM_VOICE_IARM_CALL_SESSION_BEGIN        "Voice_SessionBegin"       ///< starts a voice streaming session
#define CTRLM_VOICE_IARM_CALL_SESSION_END          "Voice_SessionEnd"         ///< ends a voice streaming session

#define CTRLM_VOICE_IARM_CALL_STATUS               "Voice_Status"             ///< IARM Call to get status
#define CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE      "Voice_ConfigureVoice"     ///< IARM Call to set up voice with JSON payload
#define CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT       "Voice_SetVoiceInit"       ///< IARM Call to set application data with JSON payload in the voice server init message
#define CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE   "Voice_SendVoiceMessage"   ///< IARM Call to send JSON payload to voice server
#define CTRLM_VOICE_IARM_CALL_SESSION_TYPES        "Voice_SessionTypes"       ///< IARM Call to get voice session request types
#define CTRLM_VOICE_IARM_CALL_SESSION_REQUEST      "Voice_SessionRequest"     ///< IARM Call to request a voice session
#define CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE    "Voice_SessionTerminate"   ///< IARM Call to terminate a voice session

#define CTRLM_VOICE_IARM_CALL_RESULT_LEN_MAX       (2048) ///< IARM Call result length

#define CTRLM_VOICE_IARM_BUS_API_REVISION             (9) ///< Revision of the Voice IARM API
#define CTRLM_VOICE_MIME_MAX_LENGTH                  (64) ///< Audio mime string maximum length
#define CTRLM_VOICE_SUBTYPE_MAX_LENGTH               (64) ///< Audio subtype string maximum length
#define CTRLM_VOICE_LANG_MAX_LENGTH                  (16) ///< Audio language string maximum length
#define CTRLM_VOICE_SERVER_URL_MAX_LENGTH          (2048) ///< Server url string maximum length
#define CTRLM_VOICE_GUIDE_LANGUAGE_MAX_LENGTH        (16) ///< Guide language string maximum length
#define CTRLM_VOICE_ASPECT_RATIO_MAX_LENGTH          (16) ///< Aspect ratio string maximum length
#define CTRLM_VOICE_SESSION_ID_MAX_LENGTH            (64) ///< Session Id string maximum length
#define CTRLM_VOICE_SESSION_TEXT_MAX_LENGTH         (512) ///< Session text string maximum length
#define CTRLM_VOICE_SESSION_MSG_MAX_LENGTH          (128) ///< Session message string maximum length
#define CTRLM_VOICE_QUERY_STRING_MAX_LENGTH         (128) ///< Query string maximum name or value length
#define CTRLM_VOICE_QUERY_STRING_MAX_PAIRS           (16) ///< Query string maximum number of name/value pairs
#define CTRLM_VOICE_REQUEST_IP_MAX_LENGTH            (48) ///< cURL request primary IP address string maximum length (big enough for IPv6)

#define CTRLM_VOICE_MIN_UTTERANCE_DURATION_MAXIMUM  (600) ///< Maximum value of the utterance duration minimum setting (in milliseconds)

// Bitmask defines for setting the available value in vrex_mgr_iarm_bus_event_voice_settings_t
#define CTRLM_VOICE_SETTINGS_VOICE_ENABLED            (0x0001) ///< Setting to enable/disable voice control
#define CTRLM_VOICE_SETTINGS_VREX_SERVER_URL          (0x0002) ///< Setting to update the vrex server url string
#define CTRLM_VOICE_SETTINGS_GUIDE_LANGUAGE           (0x0004) ///< Setting to update the guide language string
#define CTRLM_VOICE_SETTINGS_ASPECT_RATIO             (0x0008) ///< Setting to update the aspect ratio string
#define CTRLM_VOICE_SETTINGS_UTTERANCE_DURATION       (0x0010) ///< Setting to update the minimum utterance duration value
#define CTRLM_VOICE_SETTINGS_VREX_SAT_ENABLED         (0x0020) ///< Setting to enable/disable Service Access Token (SAT)
#define CTRLM_VOICE_SETTINGS_QUERY_STRINGS            (0x0040) ///< Setting to update the request query strings
#define CTRLM_VOICE_SETTINGS_FARFIELD_VREX_SERVER_URL (0x0080) ///< Setting to update the farfield vrex server url string
#define CTRLM_VOICE_SETTINGS_MIC_TAP_SERVER_URL       (0x0100) ///< Setting to update the microphone tap server url string

/// @}

/// @addtogroup CTRLM_IPC_VOICE_ENUMS
/// @{
/// @brief Enumerated Types
/// @details The Control Manager provides enumerated types for logical groups of values.

/// @brief Voice Session Results
/// @details An enumeration of the overall result for a voice session.
typedef enum {
   CTRLM_VOICE_SESSION_RESULT_SUCCESS           = 0, ///< Voice session completed successfully
   CTRLM_VOICE_SESSION_RESULT_FAILURE           = 1, ///< Voice session completed unsuccessfully
   CTRLM_VOICE_SESSION_RESULT_MAX               = 2  ///< Voice session result maximum value
} ctrlm_voice_session_result_t;

/// @brief Voice Session End Reasons
/// @details An enumeration of the reasons that cause a voice session to end.
typedef enum {
   CTRLM_VOICE_SESSION_END_REASON_DONE                    = 0, ///< Session completed normally
   CTRLM_VOICE_SESSION_END_REASON_TIMEOUT_FIRST_PACKET    = 1, ///< Session ended due to timeout on the first audio sample
   CTRLM_VOICE_SESSION_END_REASON_TIMEOUT_INTERPACKET     = 2, ///< Session ended due to timeout on a subsequent audio sample
   CTRLM_VOICE_SESSION_END_REASON_TIMEOUT_MAXIMUM         = 3, ///< Session ended due to maximum duration
   CTRLM_VOICE_SESSION_END_REASON_ADJACENT_KEY_PRESSED    = 4, ///< Session ended due to adjacent key press
   CTRLM_VOICE_SESSION_END_REASON_OTHER_KEY_PRESSED       = 5, ///< Session ended due to any other key press
   CTRLM_VOICE_SESSION_END_REASON_OTHER_ERROR             = 6, ///< Session ended due to any other reason
   CTRLM_VOICE_SESSION_END_REASON_NEW_SESSION             = 7, ///< Session ended due to a new voice session request before previous session is ended
   CTRLM_VOICE_SESSION_END_REASON_MINIMUM_QOS             = 8, ///< Session ended due to low quality of service
   CTRLM_VOICE_SESSION_END_REASON_MAX                     = 9  ///< Session End Reason maximum value
} ctrlm_voice_session_end_reason_t;

/// @brief Voice Session Abort Reasons
/// @details An enumeration of the reasons that cause a voice session to be aborted.
typedef enum {
   CTRLM_VOICE_SESSION_ABORT_REASON_BUSY                  =  0, ///< Session aborted because another session in progress
   CTRLM_VOICE_SESSION_ABORT_REASON_SERVER_NOT_READY      =  1, ///< Session aborted because the server cannot be reached
   CTRLM_VOICE_SESSION_ABORT_REASON_AUDIO_FORMAT          =  2, ///< Session aborted due to failure to negotiate an audio format
   CTRLM_VOICE_SESSION_ABORT_REASON_FAILURE               =  3, ///< Session aborted for any other reason
   CTRLM_VOICE_SESSION_ABORT_REASON_VOICE_DISABLED        =  4, ///< Session aborted because the voice feature is disabled
   CTRLM_VOICE_SESSION_ABORT_REASON_DEVICE_UPDATE         =  5, ///< Session aborted due to device update in progress
   CTRLM_VOICE_SESSION_ABORT_REASON_NO_RECEIVER_ID        =  6, ///< Session aborted because there is no receiver id
   CTRLM_VOICE_SESSION_ABORT_REASON_NEW_SESSION           =  7, ///< Session aborted because the remote's previous session is still active
   CTRLM_VOICE_SESSION_ABORT_REASON_INVALID_CONTROLLER_ID =  8, ///< Session aborted because the controller id isn't valid
   CTRLM_VOICE_SESSION_ABORT_REASON_APPLICATION_RESTART   =  9, ///< Session aborted due to restarting controlMgr.
   CTRLM_VOICE_SESSION_ABORT_REASON_MAX                   = 10  ///< Session Abort Reason maximum value
} ctrlm_voice_session_abort_reason_t;

/// @brief Voice Internal Errors
/// @details An enumeration of the Voice component's internal error codes.
typedef enum {
   CTRLM_VOICE_INTERNAL_ERROR_NONE              = 0, ///< No internal error occurred
   CTRLM_VOICE_INTERNAL_ERROR_EXCEPTION         = 1, ///< An exception generated an internal error
   CTRLM_VOICE_INTERNAL_ERROR_THREAD_CREATE     = 2, ///< Failure to launch a session thread
   CTRLM_VOICE_INTERNAL_ERROR_MAX               = 3  ///< Internal error type maximum value
} ctrlm_voice_internal_error_t;

/// @brief Voice Reset Types
/// @details An enumeration of the types of reset that can occur on a controller.
typedef enum {
   CTRLM_VOICE_RESET_TYPE_POWER_ON                   = 0, ///< Normal power up by inserting batteries
   CTRLM_VOICE_RESET_TYPE_EXTERNAL                   = 1, ///< Reset due to an external condition
   CTRLM_VOICE_RESET_TYPE_WATCHDOG                   = 2, ///< Reset due to watchdog timer expiration
   CTRLM_VOICE_RESET_TYPE_CLOCK_LOSS                 = 3, ///< Reset due to loss of main clock
   CTRLM_VOICE_RESET_TYPE_BROWN_OUT                  = 4, ///< Reset due to a low voltage condition
   CTRLM_VOICE_RESET_TYPE_OTHER                      = 5, ///< Reset due to any other reason
   CTRLM_VOICE_RESET_TYPE_MAX                        = 6  ///< Reset type maximum value
} ctrlm_voice_reset_type_t;

/// @}

/// @addtogroup CTRLM_IPC_VOICE_STRUCTS
/// @{
/// @brief Structure Definitions
/// @details The Control Manager provides structures that are used in IARM calls and events.

typedef struct {
    char    name[CTRLM_VOICE_QUERY_STRING_MAX_LENGTH];      ///< The name (null terminated string) for a query
    char    value[CTRLM_VOICE_QUERY_STRING_MAX_LENGTH];     ///< The value (null terminated string) for a query
} ctrlm_voice_query_pair_t;

typedef struct {
    unsigned char               pair_count;                                         ///< The number of name/value pairs in the following array
    ctrlm_voice_query_pair_t    query_string[CTRLM_VOICE_QUERY_STRING_MAX_PAIRS];   ///< An array of name/value pairs to contruct query strings
} ctrlm_voice_query_strings_t;

typedef struct {
   unsigned char                api_revision;                                              ///< The revision of this API.
   ctrlm_iarm_call_result_t     result;                                                    ///< Result of the IARM call
   unsigned long                available;                                                 ///< Bitmask indicating the settings that are available in this event
   unsigned char                voice_control_enabled;                                     ///< Boolean value to enable (non-zero) or disable (zero) voice control
   unsigned char                vrex_sat_enabled;                                          ///< Boolean value to enable (non-zero) or disable (zero) Service Access Token in requests to vrex server
   char                         vrex_server_url[CTRLM_VOICE_SERVER_URL_MAX_LENGTH];        ///< The url for the vrex server (null terminated string)
   char                         guide_language[CTRLM_VOICE_GUIDE_LANGUAGE_MAX_LENGTH];     ///< The guide's language [pass-thru] (null terminated string)
   char                         aspect_ratio[CTRLM_VOICE_ASPECT_RATIO_MAX_LENGTH];         ///< The guide's aspect ratio [pass-thru] (null terminated string)
   unsigned long                utterance_duration_minimum;                                ///< The minimum duration of an utterance (in milliseconds).  A value of zero disables utterance duration checking.
   ctrlm_voice_query_strings_t  query_strings;                                             ///< Query string name/value pairs, for inclusion in the VREX request
   char                         server_url_vrex_src_ff[CTRLM_VOICE_SERVER_URL_MAX_LENGTH]; ///< The url for the farfield vrex server (null terminated string)
   char                         server_url_src_mic_tap[CTRLM_VOICE_SERVER_URL_MAX_LENGTH]; ///< The url for the microphone tap server (null terminated string)
} ctrlm_voice_iarm_call_settings_t;

typedef struct {
   unsigned char available;        ///< Boolean value indicating that statistics are available (1) or not (0)
   unsigned long rf_channel;       ///< The rf channel that the voice session used (typically 15, 20 or 25)
   unsigned long buffer_watermark; ///< The highest local buffer level (estimated) in packets
   unsigned long packets_total;    ///< Total number of voice packets in the transmission
   unsigned long packets_lost;     ///< Number of packets lost in the transmission
   unsigned long dropped_retry;    ///< Number of packets dropped by the remote due to retry limit
   unsigned long dropped_buffer;   ///< Number of packets dropped by the remote due to insufficient local buffering
   unsigned long retry_mac;        ///< Total number of MAC retries during the session
   unsigned long retry_network;    ///< Total number of network level retries during the transmission
   unsigned long cca_sense;        ///< Total number of times a packet was not send due to energy detected over the CCA threshold
   unsigned long link_quality;     ///< The average link quality of all the voice packets in the transmission.
} ctrlm_voice_stats_session_t;

typedef struct {
   unsigned char            available;             ///< Boolean value indicating that a remote control reset was detected (1) or not (0)
   ctrlm_voice_reset_type_t reset_type;            ///< The type of reset that occurred
   unsigned char            voltage;               ///< RCU's voltage from 0.0 V (0) to 4.0 V (0xFF).  The value 0xFF indicates that the voltage is not available.
   unsigned char            battery_percentage;    ///< RCU's battery percentage from 0-100.
} ctrlm_voice_stats_reboot_t;


typedef struct {
   unsigned char              api_revision;        ///< The revision of this API.
   ctrlm_network_id_t         network_id;          ///< Identifier of network
   ctrlm_controller_id_t      controller_id;       ///< A unique identifier of the remote
   unsigned long long         ieee_address;        ///< IEEE MAC address of the remote
   ctrlm_iarm_call_result_t   result;              ///< OUT - The result of the operation.
} ctrlm_voice_iarm_call_voice_session_t;

typedef struct {
   unsigned char         api_revision;                             ///< The revision of this API.
   ctrlm_network_id_t    network_id;                               ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t  network_type;                             ///< Type of network on which the controller is bound
   ctrlm_controller_id_t controller_id;                            ///< A unique identifier of the remote
   unsigned long         session_id;                               ///< A unique id for the voice session.
   unsigned char         mime_type[CTRLM_VOICE_MIME_MAX_LENGTH];   ///< The mime type of the data audio/vnd.wave;codec=1 for PCM or audio/x-adpcm for ADPCM. see http://www.isi.edu/in-notes/rfc2361.txt wrt mime types
   unsigned char         sub_type[CTRLM_VOICE_SUBTYPE_MAX_LENGTH]; ///< The subtype (using exising definitions such as PCM_16_16K, PCM_16_32K, PCM_16_22K)
   unsigned char         language[CTRLM_VOICE_LANG_MAX_LENGTH];    ///< The language code (ISO-639-1 format)
   unsigned char         is_voice_assistant;                       ///< Boolean indicating if the device is a far-field device (1) as opposed to a hand-held remote (0).
} ctrlm_voice_iarm_event_session_begin_t;

typedef struct {
   unsigned char                    api_revision;         ///< The revision of this API.
   ctrlm_network_id_t               network_id;           ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t             network_type;         ///< Type of network on which the controller is bound
   ctrlm_controller_id_t            controller_id;        ///< A unique identifier of the remote
   unsigned long                    session_id;           ///< A unique id for the voice session.
   ctrlm_voice_session_end_reason_t reason;               ///< The reason for ending
   unsigned char                    is_voice_assistant;   ///< Boolean indicating if the device is a far-field device (1) as opposed to a hand-held remote (0).
} ctrlm_voice_iarm_event_session_end_t;

typedef struct {
   unsigned char                   api_revision;                                             ///< The revision of this API.
   ctrlm_network_id_t              network_id;                                               ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t            network_type;                                             ///< Type of network on which the controller is bound
   ctrlm_controller_id_t           controller_id;                                            ///< A unique identifier of the remote
   unsigned long                   session_id;                                               ///< A unique id for the voice session.
   ctrlm_voice_session_result_t    result;                                                   ///< The overall result of the voice command
   long                            return_code_http;                                         ///< HTTP request return code
   long                            return_code_curl;                                         ///< Curl's return code
   long                            return_code_vrex;                                         ///< Vrex server's return code
   long                            return_code_internal;                                     ///< Internally generated return code
   char                            vrex_session_id[CTRLM_VOICE_SESSION_ID_MAX_LENGTH];       ///< Unique identifier for the vrex session (null terminated string)
   char                            vrex_session_text[CTRLM_VOICE_SESSION_TEXT_MAX_LENGTH];   ///< Text field returned by the vrex server
   char                            vrex_session_message[CTRLM_VOICE_SESSION_MSG_MAX_LENGTH]; ///< Message field returned by the vrex server
   char                            session_uuid[CTRLM_VOICE_SESSION_ID_MAX_LENGTH];          ///< Local UUID generated for this session
   char                            curl_request_ip[CTRLM_VOICE_REQUEST_IP_MAX_LENGTH];       ///< cURL request primary IP address string
   double                          curl_request_dns_time;                                    ///< cURL request name lookup time (in seconds)
   double                          curl_request_connect_time;                                ///< cURL request connect time (in seconds)
} ctrlm_voice_iarm_event_session_result_t;

typedef struct {
   unsigned char               api_revision;  ///< The revision of this API.
   ctrlm_network_id_t          network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t        network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t       controller_id; ///< A unique identifier of the remote
   unsigned long               session_id;    ///< A unique id for the voice session.
   ctrlm_voice_stats_session_t session;       ///< Statistics for the session
   ctrlm_voice_stats_reboot_t  reboot;        ///< Data associated with a reboot
} ctrlm_voice_iarm_event_session_stats_t;

typedef struct {
   unsigned char                      api_revision;  ///< The revision of this API.
   ctrlm_network_id_t                 network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t               network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t              controller_id; ///< A unique identifier of the remote
   unsigned long                      session_id;    ///< A unique id for the voice session.
   ctrlm_voice_session_abort_reason_t reason;        ///< The reason that the voice session was aborted
} ctrlm_voice_iarm_event_session_abort_t;

typedef struct {
   unsigned char                    api_revision;         ///< The revision of this API.
   ctrlm_network_id_t               network_id;           ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t             network_type;         ///< Type of network on which the controller is bound
   ctrlm_controller_id_t            controller_id;        ///< A unique identifier of the remote
   unsigned long                    session_id;           ///< A unique id for the voice session.
   ctrlm_voice_session_end_reason_t reason;               ///< The reason that the voice streaming ended
   long                             return_code_internal; ///< Internally generated return code
} ctrlm_voice_iarm_event_session_short_t;

typedef struct {
   unsigned char api_revision;            ///< The revision of this API
   char          media_service_url[2083]; ///< The url for the media service (null terminated string)
} ctrlm_voice_iarm_event_media_service_t;


// APIs for Thunder Plugin

// IARM Call JSON
// This structure is used for the following calls:
//   CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE
//   CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT
//   CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE
//
// The payload MUST be a NULL terminated JSON String.
typedef struct {
   unsigned char  api_revision;
   char           result[CTRLM_VOICE_IARM_CALL_RESULT_LEN_MAX];
   char           payload[];
} ctrlm_voice_iarm_call_json_t;

// IARM Event JSON
// This structure is used for the following calls:
//   CTRLM_VOICE_IARM_EVENT_SESSION_BEGIN_JSON 
//   CTRLM_VOICE_IARM_EVENT_STREAM_BEGIN_JSON  
//   CTRLM_VOICE_IARM_EVENT_SERVER_MESSAGE_JSON
//   CTRLM_VOICE_IARM_EVENT_STREAM_END_JSON    
//   CTRLM_VOICE_IARM_EVENT_SESSION_END_JSON   
//
// The payload MUST be a NULL terminated JSON String.
typedef struct {
   unsigned char  api_revision;
   char           payload[];
} ctrlm_voice_iarm_event_json_t;

// End APIs for Thunder Plugin

/// @}
///
/// @addtogroup CTRLM_IPC_VOICE_CALLS
/// @{
/// @brief Remote Calls accessible via IARM bus
/// @details IARM calls are synchronous calls to functions provided by other IARM bus members. Each bus member can register
/// calls using the IARM_Bus_RegisterCall() function. Other members can invoke the call using the IARM_Bus_Call() function.
///
/// -----------------
/// Call Registration
/// -----------------
///
/// The Voice component of Control Manager registers the following calls.
///
/// | Bus Name                 | Call Name                      | Argument                      | Description                                    |
/// | :-------                 | :--------                      | :-------                      | :----------                                    |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_CALL_SETTINGS | ctrlm_voice_iarm_settings_t * | Sets one or more voice configuration settings. |
///
/// Examples:
///
/// Get a controller's status:
///
///     IARM_Result_t               result;
///     ctrlm_voice_iarm_settings_t settings;
///     settings.available = (CTRLM_VOICE_SETTINGS_VOICE_ENABLED | CTRLM_VOICE_SETTINGS_VREX_SAT_ENABLED);
///     settings.voice_control_enabled = 1;
///     settings.vrex_sat_enabled      = 1;
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_CALL_SETTINGS, (void *)&settings, sizeof(settings));
///     if(IARM_RESULT_SUCCESS == result && CTRLM_IARM_CALL_RESULT_SUCCESS == settings.result) {
///         // Settings were successfully updated
///     }
///     }
///
/// @}
///
/// @addtogroup CTRLM_IPC_VOICE_EVENTS
/// @{
/// @brief Broadcast Events accessible via IARM bus
/// @details The IARM bus uses events to broadcast information to interested clients. An event is sent separately to each client. There are no return values for an event and no
/// guarantee that a client will receive the event.  Each event has a different argument structure according to the information being transferred to the clients.  The events that the
/// Remote Control component in Control Manager generates and subscribes to are detailed below.
///
/// ----------------------------
/// Event Generation (Broadcast)
/// ----------------------------
///
/// The Voice component generates events that can be received by other processes connected to the IARM bus. The following events
/// are registered during initialization:
///
/// | Bus Name                 | Event Name                            | Argument                                  | Description                                                      |
/// | :-------                 | :---------                            | :-------                                  | :----------                                                      |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_BEGIN  | ctrlm_voice_iarm_event_session_begin_t *  | Generated at the beginning of a voice session                    |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_END    | ctrlm_voice_iarm_event_session_end_t *    | Generated at the end of a voice session                          |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_RESULT | ctrlm_voice_iarm_event_session_result_t * | Generated when the result of the voice session is available      |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_STATS  | ctrlm_voice_iarm_event_session_stats_t *  | Generated when the statistics of the voice session are available |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_ABORT  | ctrlm_voice_iarm_event_session_abort_t *  | Generated when a voice session is aborted (denied)               |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_SESSION_SHORT  | ctrlm_voice_iarm_event_session_short_t *  | Generated when a short voice session is detected                 |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_VOICE_IARM_EVENT_MEDIA_SERVICE  | ctrlm_voice_iarm_event_media_service_t *  | Generated when a media service response is received              |
///
/// IARM events are available on a subscription basis. In order to receive an event, a client must explicitly register to receive the event by calling
/// IARM_Bus_RegisterEventHandler() with the bus name, event name and a @link IARM_EventHandler_t handler function@endlink. Events may be generated at any time by the
/// Voice component. All events are asynchronous.
///
/// Examples:
///
/// Register for a Voice event:
///
///     IARM_Result_t result;
///
///     result = IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_VOICE_IARM_EVENT_SESSION_RESULT, session_result_handler_func);
///     if(IARM_RESULT_SUCCESS == result) {
///         // Event registration was set successfully
///     }
///     }
///
/// @}
///
/// @addtogroup CTRLM_IPC_VOICE_COMMS
/// @{
/// @brief Communication Interfaces
/// @details The following diagrams detail the main communication paths for the Voice component.
/// ---------------
/// Voice Data Flow
/// ---------------
///
/// @dot
/// digraph CTRL_MGR_VREX {
///     rankdir=LR;
///     "VREX_IDLE"         [shape="ellipse", fontname=Helvetica, fontsize=10, label="IDLE"];
///     "VREX_OPEN"         [shape="ellipse", fontname=Helvetica, fontsize=10, label="OPEN"];
///     "VREX_CLOSE"        [shape="ellipse", fontname=Helvetica, fontsize=10, label="CLOSE"];
///     "VREX_IS_REM_VALID" [shape="diamond", fontname=Helvetica, fontsize=10, label="Is remote\nvalid?"];
///     "VREX_IS_SVR_AVAIL" [shape="diamond", fontname=Helvetica, fontsize=10, label="Is server\navailable?"];
///     "VREX_IS_SEND_OK"   [shape="diamond", fontname=Helvetica, fontsize=10, label="Is fragment transferred\nto the server?"];
///
///     "VREX_IDLE"         -> "VREX_IS_REM_VALID" [dir="forward", fontname=Helvetica, fontsize=10,label="  Voice Begin Event Rxd"];
///     "VREX_IDLE"         -> "VREX_IDLE"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Voice Fragment/End Event Rxd"];
///     "VREX_IS_REM_VALID" -> "VREX_IS_SVR_AVAIL" [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes"];
///     "VREX_IS_REM_VALID" -> "VREX_IDLE"         [dir="forward", fontname=Helvetica, fontsize=10,label="  No"];
///     "VREX_IS_SVR_AVAIL" -> "VREX_OPEN"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, open session"];
///     "VREX_IS_SVR_AVAIL" -> "VREX_IDLE"         [dir="forward", fontname=Helvetica, fontsize=10,label="  No"];
///     "VREX_OPEN"         -> "VREX_IS_SEND_OK"   [dir="forward", fontname=Helvetica, fontsize=10,label="  Voice Fragment\nEvent Rxd"];
///     "VREX_IS_SEND_OK"   -> "VREX_OPEN"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes"];
///     "VREX_IS_SEND_OK"   -> "VREX_CLOSE"        [dir="forward", fontname=Helvetica, fontsize=10,label="  No, end session"];
///     "VREX_OPEN"         -> "VREX_CLOSE"        [dir="forward", fontname=Helvetica, fontsize=10,label="  Voice End Event Rxd.\nTransfer status\ncode to server."];
///     "VREX_CLOSE"        -> "VREX_IDLE"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Close session."];
///     { rank=same; "VREX_IS_REM_VALID"; "VREX_IS_SVR_AVAIL"; }
///     { rank=same; "VREX_OPEN"; "VREX_IS_SEND_OK"; }
/// }
/// \enddot
/// @}
///
/// @}

/// @file ctrlm_ipc_key_codes.h

/// @brief Control Manager Key Codes
/// @details An enumeration of the key codes that are used in Control Manager.
typedef enum {
   CTRLM_KEY_CODE_OK               = 0x00,
   CTRLM_KEY_CODE_UP_ARROW         = 0x01,
   CTRLM_KEY_CODE_DOWN_ARROW       = 0x02,
   CTRLM_KEY_CODE_LEFT_ARROW       = 0x03,
   CTRLM_KEY_CODE_RIGHT_ARROW      = 0x04,
   CTRLM_KEY_CODE_MENU             = 0x09,
   CTRLM_KEY_CODE_DVR              = 0x0B,
   CTRLM_KEY_CODE_FAV              = 0x0C,
   CTRLM_KEY_CODE_EXIT             = 0x0D,
   CTRLM_KEY_CODE_HOME             = 0x10,
   CTRLM_KEY_CODE_DIGIT_0          = 0x20,
   CTRLM_KEY_CODE_DIGIT_1          = 0x21,
   CTRLM_KEY_CODE_DIGIT_2          = 0x22,
   CTRLM_KEY_CODE_DIGIT_3          = 0x23,
   CTRLM_KEY_CODE_DIGIT_4          = 0x24,
   CTRLM_KEY_CODE_DIGIT_5          = 0x25,
   CTRLM_KEY_CODE_DIGIT_6          = 0x26,
   CTRLM_KEY_CODE_DIGIT_7          = 0x27,
   CTRLM_KEY_CODE_DIGIT_8          = 0x28,
   CTRLM_KEY_CODE_DIGIT_9          = 0x29,
   CTRLM_KEY_CODE_PERIOD           = 0x2A,
   CTRLM_KEY_CODE_RETURN           = 0x2B,
   CTRLM_KEY_CODE_CH_UP            = 0x30,
   CTRLM_KEY_CODE_CH_DOWN          = 0x31,
   CTRLM_KEY_CODE_LAST             = 0x32,
   CTRLM_KEY_CODE_LANG             = 0x33,
   CTRLM_KEY_CODE_INPUT_SELECT     = 0x34,
   CTRLM_KEY_CODE_INFO             = 0x35,
   CTRLM_KEY_CODE_HELP             = 0x36,
   CTRLM_KEY_CODE_PAGE_UP          = 0x37,
   CTRLM_KEY_CODE_PAGE_DOWN        = 0x38,
   CTRLM_KEY_CODE_MOTION           = 0x3B,
   CTRLM_KEY_CODE_SEARCH           = 0x3C,
   CTRLM_KEY_CODE_LIVE             = 0x3D,
   CTRLM_KEY_CODE_HD_ZOOM          = 0x3E,
   CTRLM_KEY_CODE_SHARE            = 0x3F,
   CTRLM_KEY_CODE_TV_POWER         = 0x40,
   CTRLM_KEY_CODE_VOL_UP           = 0x41,
   CTRLM_KEY_CODE_VOL_DOWN         = 0x42,
   CTRLM_KEY_CODE_MUTE             = 0x43,
   CTRLM_KEY_CODE_PLAY             = 0x44,
   CTRLM_KEY_CODE_STOP             = 0x45,
   CTRLM_KEY_CODE_PAUSE            = 0x46,
   CTRLM_KEY_CODE_RECORD           = 0x47,
   CTRLM_KEY_CODE_REWIND           = 0x48,
   CTRLM_KEY_CODE_FAST_FORWARD     = 0x49,
   CTRLM_KEY_CODE_30_SEC_SKIP      = 0x4B,
   CTRLM_KEY_CODE_REPLAY           = 0x4C,
   CTRLM_KEY_CODE_TV_POWER_ON      = 0x4D,
   CTRLM_KEY_CODE_TV_POWER_OFF     = 0x4E,
   CTRLM_KEY_CODE_SWAP             = 0x51,
   CTRLM_KEY_CODE_ON_DEMAND        = 0x52,
   CTRLM_KEY_CODE_GUIDE            = 0x53,
   CTRLM_KEY_CODE_PUSH_TO_TALK     = 0x57,
   CTRLM_KEY_CODE_PIP_ON_OFF       = 0x58,
   CTRLM_KEY_CODE_PIP_MOVE         = 0x59,
   CTRLM_KEY_CODE_PIP_CH_UP        = 0x5A,
   CTRLM_KEY_CODE_PIP_CH_DOWN      = 0x5B,
   CTRLM_KEY_CODE_LOCK             = 0x5C,
   CTRLM_KEY_CODE_DAY_PLUS         = 0x5D,
   CTRLM_KEY_CODE_DAY_MINUS        = 0x5E,
   CTRLM_KEY_CODE_PLAY_PAUSE       = 0x61,
   CTRLM_KEY_CODE_STOP_VIDEO       = 0x64,
   CTRLM_KEY_CODE_MUTE_MIC         = 0x65,
   CTRLM_KEY_CODE_AVR_POWER_TOGGLE = 0x68,
   CTRLM_KEY_CODE_AVR_POWER_OFF    = 0x69,
   CTRLM_KEY_CODE_AVR_POWER_ON     = 0x6A,
   CTRLM_KEY_CODE_POWER_TOGGLE     = 0x6B,
   CTRLM_KEY_CODE_POWER_OFF        = 0x6C,
   CTRLM_KEY_CODE_POWER_ON         = 0x6D,
   CTRLM_KEY_CODE_OCAP_B           = 0x71,
   CTRLM_KEY_CODE_OCAP_C           = 0x72,
   CTRLM_KEY_CODE_OCAP_D           = 0x73,
   CTRLM_KEY_CODE_OCAP_A           = 0x74,
   CTRLM_KEY_CODE_CC               = 0x90,
   CTRLM_KEY_CODE_PROFILE          = 0xA0,
   CTRLM_KEY_CODE_CALL             = 0xA1,
   CTRLM_KEY_CODE_HOLD             = 0xA2,
   CTRLM_KEY_CODE_END              = 0xA3,
   CTRLM_KEY_CODE_VIEWS            = 0xA4,
   CTRLM_KEY_CODE_SELF_VIEW        = 0xA5,
   CTRLM_KEY_CODE_ZOOM_IN          = 0xA6,
   CTRLM_KEY_CODE_ZOOM_OUT         = 0xA7,
   CTRLM_KEY_CODE_BACKSPACE        = 0xA8,
   CTRLM_KEY_CODE_LOCK_UNLOCK      = 0xA9,
   CTRLM_KEY_CODE_CAPS             = 0xAA,
   CTRLM_KEY_CODE_ALT              = 0xAB,
   CTRLM_KEY_CODE_SPACE            = 0xAC,
   CTRLM_KEY_CODE_WWW_DOT          = 0xAD,
   CTRLM_KEY_CODE_DOT_COM          = 0xAE,
   CTRLM_KEY_CODE_UPPER_A          = 0xB0,
   CTRLM_KEY_CODE_UPPER_B          = 0xB1,
   CTRLM_KEY_CODE_UPPER_C          = 0xB2,
   CTRLM_KEY_CODE_UPPER_D          = 0xB3,
   CTRLM_KEY_CODE_UPPER_E          = 0xB4,
   CTRLM_KEY_CODE_UPPER_F          = 0xB5,
   CTRLM_KEY_CODE_UPPER_G          = 0xB6,
   CTRLM_KEY_CODE_UPPER_H          = 0xB7,
   CTRLM_KEY_CODE_UPPER_I          = 0xB8,
   CTRLM_KEY_CODE_UPPER_J          = 0xB9,
   CTRLM_KEY_CODE_UPPER_K          = 0xBA,
   CTRLM_KEY_CODE_UPPER_L          = 0xBB,
   CTRLM_KEY_CODE_UPPER_M          = 0xBC,
   CTRLM_KEY_CODE_UPPER_N          = 0xBD,
   CTRLM_KEY_CODE_UPPER_O          = 0xBE,
   CTRLM_KEY_CODE_UPPER_P          = 0xBF,
   CTRLM_KEY_CODE_UPPER_Q          = 0xC0,
   CTRLM_KEY_CODE_UPPER_R          = 0xC1,
   CTRLM_KEY_CODE_UPPER_S          = 0xC2,
   CTRLM_KEY_CODE_UPPER_T          = 0xC3,
   CTRLM_KEY_CODE_UPPER_U          = 0xC4,
   CTRLM_KEY_CODE_UPPER_V          = 0xC5,
   CTRLM_KEY_CODE_UPPER_W          = 0xC6,
   CTRLM_KEY_CODE_UPPER_X          = 0xC7,
   CTRLM_KEY_CODE_UPPER_Y          = 0xC8,
   CTRLM_KEY_CODE_UPPER_Z          = 0xC9,
   CTRLM_KEY_CODE_LOWER_A          = 0xCA,
   CTRLM_KEY_CODE_LOWER_B          = 0xCB,
   CTRLM_KEY_CODE_LOWER_C          = 0xCC,
   CTRLM_KEY_CODE_LOWER_D          = 0xCD,
   CTRLM_KEY_CODE_LOWER_E          = 0xCE,
   CTRLM_KEY_CODE_LOWER_F          = 0xCF,
   CTRLM_KEY_CODE_LOWER_G          = 0xD0,
   CTRLM_KEY_CODE_LOWER_H          = 0xD1,
   CTRLM_KEY_CODE_LOWER_I          = 0xD2,
   CTRLM_KEY_CODE_LOWER_J          = 0xD3,
   CTRLM_KEY_CODE_LOWER_K          = 0xD4,
   CTRLM_KEY_CODE_LOWER_L          = 0xD5,
   CTRLM_KEY_CODE_LOWER_M          = 0xD6,
   CTRLM_KEY_CODE_LOWER_N          = 0xD7,
   CTRLM_KEY_CODE_LOWER_O          = 0xD8,
   CTRLM_KEY_CODE_LOWER_P          = 0xD9,
   CTRLM_KEY_CODE_LOWER_Q          = 0xDA,
   CTRLM_KEY_CODE_LOWER_R          = 0xDB,
   CTRLM_KEY_CODE_LOWER_S          = 0xDC,
   CTRLM_KEY_CODE_LOWER_T          = 0xDD,
   CTRLM_KEY_CODE_LOWER_U          = 0xDE,
   CTRLM_KEY_CODE_LOWER_V          = 0xDF,
   CTRLM_KEY_CODE_LOWER_W          = 0xE0,
   CTRLM_KEY_CODE_LOWER_X          = 0xE1,
   CTRLM_KEY_CODE_LOWER_Y          = 0xE2,
   CTRLM_KEY_CODE_LOWER_Z          = 0xE3,
   CTRLM_KEY_CODE_QUESTION         = 0xE4,
   CTRLM_KEY_CODE_EXCLAMATION      = 0xE5,
   CTRLM_KEY_CODE_POUND            = 0xE6,
   CTRLM_KEY_CODE_DOLLAR           = 0xE7,
   CTRLM_KEY_CODE_PERCENT          = 0xE8,
   CTRLM_KEY_CODE_AMPERSAND        = 0xE9,
   CTRLM_KEY_CODE_ASTERISK         = 0xEA,
   CTRLM_KEY_CODE_LEFT_PAREN       = 0xEB,
   CTRLM_KEY_CODE_RIGHT_PAREN      = 0xEC,
   CTRLM_KEY_CODE_PLUS             = 0xED,
   CTRLM_KEY_CODE_MINUS            = 0xEE,
   CTRLM_KEY_CODE_EQUAL            = 0xEF,
   CTRLM_KEY_CODE_FORWARD_SLASH    = 0xF0,
   CTRLM_KEY_CODE_UNDERSCORE       = 0xF1,
   CTRLM_KEY_CODE_DOUBLE_QUOTE     = 0xF2,
   CTRLM_KEY_CODE_COLON            = 0xF3,
   CTRLM_KEY_CODE_SEMICOLON        = 0xF4,
   CTRLM_KEY_CODE_COMMERCIAL_AT    = 0xF5,
   CTRLM_KEY_CODE_APOSTROPHE       = 0xF6,
   CTRLM_KEY_CODE_COMMA            = 0xF7,
   CTRLM_KEY_CODE_INVALID          = 0xFF
} ctrlm_key_code_t;

/// @file ctrlm_ipc_rcu.h
///
/// @defgroup CTRLM_IPC_RCU IARM API - Remote Control
/// @{
///
/// @defgroup CTRLM_IPC_RCU_COMMS       Communication Interfaces
/// @defgroup CTRLM_IPC_RCU_CALLS       IARM Remote Procedure Calls
/// @defgroup CTRLM_IPC_RCU_EVENTS      IARM Events
/// @defgroup CTRLM_IPC_RCU_DEFINITIONS Constants
/// @defgroup CTRLM_IPC_RCU_ENUMS       Enumerations
/// @defgroup CTRLM_IPC_RCU_STRUCTS     Structures
///
/// @addtogroup CTRLM_IPC_RCU_DEFINITIONS
/// @{
/// @brief Constant Defintions
/// @details The Control Manager provides definitions for constant values.

#define CTRLM_RCU_IARM_CALL_VALIDATION_FINISH            "Rcu_ValidationFinish"     ///< IARM Call to complete controller validation
#define CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS            "Rcu_ControllerStatus"     ///< IARM Call to get controller information
#define CTRLM_RCU_IARM_CALL_CONTROLLER_LINK_KEY          "Rcu_ControllerLinkKey"    ///< IARM Call to get controller link key
#define CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET              "Rcu_RibRequestGet"        ///< IARM Call to retrieves an attribute from the controller's RIB
#define CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET              "Rcu_RibRequestSet"        ///< IARM Call to set an attribute in the controller's RIB
#define CTRLM_RCU_IARM_CALL_REVERSE_CMD                  "Rcu_ReverseCmd"           ///< IARM Call to Trigger Remote Controller Action
#define CTRLM_RCU_IARM_CALL_RF4CE_POLLING_ACTION         "Rcu_Rf4cePollingAction"   ///< IARM Call to Send Remote Heartbeat Response Polling Action

#define CTRLM_RCU_IARM_BUS_API_REVISION                  (13)    ///< Revision of the RCU IARM API
#define CTRLM_RCU_VALIDATION_KEY_QTY                      (3)    ///< Number of validation keys used in internal validation
#define CTRLM_RCU_MAX_SETUP_COMMAND_SIZE                  (5)    ///< Maximum setup command size (numbers entered after setup key)
#define CTRLM_RCU_VERSION_LENGTH                         (18)    ///< Maximum length of the version string
#define CTRLM_RCU_BUILD_ID_LENGTH                        (93)    ///< Maximum length of the build id string
#define CTRLM_RCU_DSP_BUILD_ID_LENGTH                    (93)    ///< Maximum length of the dsp build id string
#define CTRLM_RCU_MAX_USER_STRING_LENGTH                  (9)    ///< Maximum length of remote user string (including null termination)
#define CTRLM_RCU_MAX_IR_DB_CODE_LENGTH                   (7)    ///< Maximum length of an IR DB code string (including null termination)
#define CTRLM_RCU_MAX_MANUFACTURER_LENGTH                (16)    ///< Maximum length of manufacturer name string (including null termination)
#define CTRLM_RCU_MAX_CHIPSET_LENGTH                     (16)    ///< Maximum length of chipset name string (including null termination)
#define CTRLM_RCU_CALL_RCU_REVERSE_CMD_PARAMS_MAX        (10)    ///< Maximum number of parameters for CTRLM_RCU_IARM_CALL_REVERSE_CMD call
#define CTRLM_RCU_MAX_EVENT_SOURCE_LENGTH                (10)    ///< Maximum length of the event source (including null termination)
#define CTRLM_RCU_MAX_EVENT_TYPE_LENGTH                  (20)    ///< Maximum length of the event type (including null termination)
#define CTRLM_RCU_MAX_EVENT_DATA_LENGTH                  (50)    ///< Maximum length of the event data (including null termination)

#define CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE                 (92)    ///< Maximum size of a RIB attribute (in bytes)
#define CTRLM_RCU_RIB_ATTR_LEN_PERIPHERAL_ID              (4)    ///< RIB Attribute Length - Peripheral Id
#define CTRLM_RCU_RIB_ATTR_LEN_RF_STATISTICS             (16)    ///< RIB Attribute Length - RF Statistics
#define CTRLM_RCU_RIB_ATTR_LEN_VERSIONING                 (4)    ///< RIB Attribute Length - Versioning
#define CTRLM_RCU_RIB_ATTR_LEN_VERSIONING_BUILD_ID       (92)    ///< RIB Attribute Length - Versioning (Build ID)
#define CTRLM_RCU_RIB_ATTR_LEN_BATTERY_STATUS            (11)    ///< RIB Attribute Length - Battery Status
#define CTRLM_RCU_RIB_ATTR_LEN_SHORT_RF_RETRY_PERIOD      (4)    ///< RIB Attribute Length - Short RF Retry Period
#define CTRLM_RCU_RIB_ATTR_LEN_POLLING_METHODS            (1)    ///< RIB Attribute Length - Polling Methods
#define CTRLM_RCU_RIB_ATTR_LEN_POLLING_CONFIGURATION      (8)    ///< RIB Attribute Length - Polling Configuration
#define CTRLM_RCU_RIB_ATTR_LEN_PRIVACY                    (1)    ///< RIB Attribute Length - Privacy
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_COMMAND_STATUS       (1)    ///< RIB Attribute Length - Voice Command Status
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_COMMAND_LENGTH       (1)    ///< RIB Attribute Length - Voice Command Length
#define CTRLM_RCU_RIB_ATTR_LEN_MAXIMUM_UTTERANCE_LENGTH   (2)    ///< RIB Attribute Length - Maximum Utterance Length
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_COMMAND_ENCRYPTION   (1)    ///< RIB Attribute Length - Voice Command Encryption
#define CTRLM_RCU_RIB_ATTR_LEN_MAX_VOICE_DATA_RETRY       (1)    ///< RIB Attribute Length - Voice Data Retry
#define CTRLM_RCU_RIB_ATTR_LEN_MAX_VOICE_CSMA_BACKOFF     (1)    ///< RIB Attribute Length - Maximum Voice CSMA Backoff
#define CTRLM_RCU_RIB_ATTR_LEN_MIN_VOICE_DATA_BACKOFF     (1)    ///< RIB Attribute Length - Minimum Voice Data Backoff
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_CTRL_AUDIO_PROFILES  (2)    ///< RIB Attribute Length - Voice Controller Audio Profiles
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_TARG_AUDIO_PROFILES  (2)    ///< RIB Attribute Length - Voice Target Audio Profiles
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_STATISTICS           (8)    ///< RIB Attribute Length - Voice Statistics
#define CTRLM_RCU_RIB_ATTR_LEN_OPUS_ENCODING_PARAMS       (5)    ///< RIB Attribute Length - OPUS Encoding Params
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_SESSION_QOS          (7)    ///< RIB Attribute Length - Voice Session QOS
#define CTRLM_RCU_RIB_ATTR_LEN_RIB_ENTRIES_UPDATED        (1)    ///< RIB Attribute Length - Entries Updated
#define CTRLM_RCU_RIB_ATTR_LEN_RIB_UPDATE_CHECK_INTERVAL  (2)    ///< RIB Attribute Length - Update Check Interval
#define CTRLM_RCU_RIB_ATTR_LEN_VOICE_SESSION_STATISTICS  (16)    ///< RIB Attribute Length - Voice Session Statistics
#define CTRLM_RCU_RIB_ATTR_LEN_UPDATE_VERSIONING          (4)    ///< RIB Attribute Length - Update Versioning
#define CTRLM_RCU_RIB_ATTR_LEN_PRODUCT_NAME              (20)    ///< RIB Attribute Length - Product Name
#define CTRLM_RCU_RIB_ATTR_LEN_DOWNLOAD_RATE              (1)    ///< RIB Attribute Length - Download Rate
#define CTRLM_RCU_RIB_ATTR_LEN_UPDATE_POLLING_PERIOD      (2)    ///< RIB Attribute Length - Update Polling Period
#define CTRLM_RCU_RIB_ATTR_LEN_DATA_REQUEST_WAIT_TIME     (2)    ///< RIB Attribute Length - Data Request Wait Time
#define CTRLM_RCU_RIB_ATTR_LEN_IR_RF_DATABASE_STATUS      (1)    ///< RIB Attribute Length - IR RF Database Status
#define CTRLM_RCU_RIB_ATTR_LEN_IR_RF_DATABASE            (92)    ///< RIB Attribute Length - IR RF Database
#define CTRLM_RCU_RIB_ATTR_LEN_VALIDATION_CONFIGURATION   (4)    ///< RIB Attribute Length - Validation Configuration
#define CTRLM_RCU_RIB_ATTR_LEN_CONTROLLER_IRDB_STATUS    (15)    ///< RIB Attribute Length - Controller IRDB Status
#define CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS        (13)    ///< RIB Attribute Length - Target IRDB Status
#define CTRLM_RCU_RIB_ATTR_LEN_MFG_TEST                   (8)    ///< RIB Attribute Length - MFG Test
#define CTRLM_RCU_RIB_ATTR_LEN_MFG_TEST_HAPTICS          (12)    ///< RIB Attribute Length - MFG Test with haptics
#define CTRLM_RCU_RIB_ATTR_LEN_MFG_TEST_RESULT            (1)    ///< RIB Attribute Length - MFG Security Key Test Rib Result

#define CTRLM_RCU_POLLING_RESPONSE_DATA_LEN               (5)
/// @}

/// @addtogroup CTRLM_IPC_RCU_ENUMS
/// @{
/// @brief Enumerated Types
/// @details The Control Manager provides enumerated types for logical groups of values.

/// @brief RCU Binding Validation Results
/// @details The process of binding a remote control will terminate with one of the results in this enumeration.
typedef enum {
   CTRLM_RCU_VALIDATION_RESULT_SUCCESS          =  0, ///< The validation completed successfully.
   CTRLM_RCU_VALIDATION_RESULT_PENDING          =  1, ///< The validation is still pending.
   CTRLM_RCU_VALIDATION_RESULT_TIMEOUT          =  2, ///< The validation has exceeded the timeout period.
   CTRLM_RCU_VALIDATION_RESULT_COLLISION        =  3, ///< The validation resulted in a collision (key was received from a different remote than the one being validated).
   CTRLM_RCU_VALIDATION_RESULT_FAILURE          =  4, ///< The validation did not complete successfully (communication failures).
   CTRLM_RCU_VALIDATION_RESULT_ABORT            =  5, ///< The validation was aborted (infinity key from remote being validated).
   CTRLM_RCU_VALIDATION_RESULT_FULL_ABORT       =  6, ///< The validation was fully aborted (exit key from remote being validated).
   CTRLM_RCU_VALIDATION_RESULT_FAILED           =  7, ///< The validation has failed (ie. did not put in correct code, etc).
   CTRLM_RCU_VALIDATION_RESULT_BIND_TABLE_FULL  =  8, ///< The validation has failed due to lack of space in the binding table.
   CTRLM_RCU_VALIDATION_RESULT_IN_PROGRESS      =  9, ///< The validation has failed because another validation is in progress.
   CTRLM_RCU_VALIDATION_RESULT_CTRLM_RESTART    = 10, ///< The validation has failed because of restarting controlMgr.
   CTRLM_RCU_VALIDATION_RESULT_MAX              = 11  ///< Maximum validation result value
} ctrlm_rcu_validation_result_t;

/// @brief RCU Configuration Results
/// @details The process of configuring a remote control will terminate with one of the results in this enumeration.
typedef enum {
   CTRLM_RCU_CONFIGURATION_RESULT_SUCCESS = 0, ///< The configuration completed successfully.
   CTRLM_RCU_CONFIGURATION_RESULT_PENDING = 1, ///< The configuration is still pending.
   CTRLM_RCU_CONFIGURATION_RESULT_TIMEOUT = 2, ///< The configuration has exceeded the timeout period.
   CTRLM_RCU_CONFIGURATION_RESULT_FAILURE = 4, ///< The configuration did not complete successfully.
   CTRLM_RCU_CONFIGURATION_RESULT_MAX     = 5  ///< Maximum validation result value
} ctrlm_rcu_configuration_result_t;

/// @brief RCU RIB Attribute Id's
/// @details The attribute identifier of all supported RIB entries.
typedef enum {
   CTRLM_RCU_RIB_ATTR_ID_PERIPHERAL_ID             = 0x00, ///< RIB Attribute - Peripheral Id
   CTRLM_RCU_RIB_ATTR_ID_RF_STATISTICS             = 0x01, ///< RIB Attribute - RF Statistics
   CTRLM_RCU_RIB_ATTR_ID_VERSIONING                = 0x02, ///< RIB Attribute - Versioning
   CTRLM_RCU_RIB_ATTR_ID_BATTERY_STATUS            = 0x03, ///< RIB Attribute - Battery Status
   CTRLM_RCU_RIB_ATTR_ID_SHORT_RF_RETRY_PERIOD     = 0x04, ///< RIB Attribute - Short RF Retry Period
   CTRLM_RCU_RIB_ATTR_ID_TARGET_ID_DATA            = 0x05, ///< RIB Attribute - Target ID Data
   CTRLM_RCU_RIB_ATTR_ID_POLLING_METHODS           = 0x08, ///< RIB Attribute - Polling Methods
   CTRLM_RCU_RIB_ATTR_ID_POLLING_CONFIGURATION     = 0x09, ///< RIB Attribute - Polling Configuration
   CTRLM_RCU_RIB_ATTR_ID_PRIVACY                   = 0x0B, ///< RIB Attribute - Privacy
   CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_CAPABILITIES   = 0x0C, ///< RIB Attribute - Controller Capabilities
   CTRLM_RCU_RIB_ATTR_ID_RESPONSE_TIME             = 0x0D, ///< RIB Attribute - Response Time
   CTRLM_RCU_RIB_ATTR_ID_VOICE_COMMAND_STATUS      = 0x10, ///< RIB Attribute - Voice Command Status
   CTRLM_RCU_RIB_ATTR_ID_VOICE_COMMAND_LENGTH      = 0x11, ///< RIB Attribute - Voice Command Length
   CTRLM_RCU_RIB_ATTR_ID_MAXIMUM_UTTERANCE_LENGTH  = 0x12, ///< RIB Attribute - Maximum Utterance Length
   CTRLM_RCU_RIB_ATTR_ID_VOICE_COMMAND_ENCRYPTION  = 0x13, ///< RIB Attribute - Voice Command Encryption
   CTRLM_RCU_RIB_ATTR_ID_MAX_VOICE_DATA_RETRY      = 0x14, ///< RIB Attribute - Voice Data Retry
   CTRLM_RCU_RIB_ATTR_ID_MAX_VOICE_CSMA_BACKOFF    = 0x15, ///< RIB Attribute - Maximum Voice CSMA Backoff
   CTRLM_RCU_RIB_ATTR_ID_MIN_VOICE_DATA_BACKOFF    = 0x16, ///< RIB Attribute - Minimum Voice Data Backoff
   CTRLM_RCU_RIB_ATTR_ID_VOICE_CTRL_AUDIO_PROFILES = 0x17, ///< RIB Attribute - Voice Controller Audio Profiles
   CTRLM_RCU_RIB_ATTR_ID_VOICE_TARG_AUDIO_PROFILES = 0x18, ///< RIB Attribute - Voice Target Audio Profiles
   CTRLM_RCU_RIB_ATTR_ID_VOICE_STATISTICS          = 0x19, ///< RIB Attribute - Voice Statistics
   CTRLM_RCU_RIB_ATTR_ID_RIB_ENTRIES_UPDATED       = 0x1A, ///< RIB Attribute - Entries Updated
   CTRLM_RCU_RIB_ATTR_ID_RIB_UPDATE_CHECK_INTERVAL = 0x1B, ///< RIB Attribute - Update Check Interval
   CTRLM_RCU_RIB_ATTR_ID_VOICE_SESSION_STATISTICS  = 0x1C, ///< RIB Attribute - Voice Session Statistics
   CTRLM_RCU_RIB_ATTR_ID_OPUS_ENCODING_PARAMS      = 0x1D, ///< RIB Attribute - OPUS Encoding Params
   CTRLM_RCU_RIB_ATTR_ID_VOICE_SESSION_QOS         = 0x1E, ///< RIB Attribute - Voice Session QOS
   CTRLM_RCU_RIB_ATTR_ID_UPDATE_VERSIONING         = 0x31, ///< RIB Attribute - Update Versioning
   CTRLM_RCU_RIB_ATTR_ID_PRODUCT_NAME              = 0x32, ///< RIB Attribute - Product Name
   CTRLM_RCU_RIB_ATTR_ID_DOWNLOAD_RATE             = 0x33, ///< RIB Attribute - Download Rate
   CTRLM_RCU_RIB_ATTR_ID_UPDATE_POLLING_PERIOD     = 0x34, ///< RIB Attribute - Update Polling Period
   CTRLM_RCU_RIB_ATTR_ID_DATA_REQUEST_WAIT_TIME    = 0x35, ///< RIB Attribute - Data Request Wait Time
   CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS     = 0xDA, ///< RIB Attribute - IR RF Database Status
   CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE            = 0xDB, ///< RIB Attribute - IR RF Database
   CTRLM_RCU_RIB_ATTR_ID_VALIDATION_CONFIGURATION  = 0xDC, ///< RIB Attribute - Validation Configuration
   CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS    = 0xDD, ///< RIB Attribute - Controller IRDB Status
   CTRLM_RCU_RIB_ATTR_ID_TARGET_IRDB_STATUS        = 0xDE, ///< RIB Attribute - Target IRDB Status
   CTRLM_RCU_RIB_ATTR_ID_FAR_FIELD_CONFIGURATION   = 0xE0, ///< RIB Attribute - Far Field Configuration
   CTRLM_RCU_RIB_ATTR_ID_FAR_FIELD_METRICS         = 0xE1, ///< RIB Attribute - Far Field Metrics
   CTRLM_RCU_RIB_ATTR_ID_DSP_CONFIGURATION         = 0xE2, ///< RIB Attribute - DSP Configuration
   CTRLM_RCU_RIB_ATTR_ID_DSP_METRICS               = 0xE3, ///< RIB Attribute - DSP Metrics
   CTRLM_RCU_RIB_ATTR_ID_MFG_TEST                  = 0xFB, ///< RIB Attribute - MFG Test
   CTRLM_RCU_RIB_ATTR_ID_MEMORY_DUMP               = 0xFE, ///< RIB Attribute - Memory Dump
   CTRLM_RCU_RIB_ATTR_ID_GENERAL_PURPOSE           = 0xFF, ///< RIB Attribute - General Purpose
} ctrlm_rcu_rib_attr_id_t;

/// @brief RCU Binding Type
/// @details The type of binding that can be performed between a controller and target.
typedef enum {
   CTRLM_RCU_BINDING_TYPE_INTERACTIVE = 0, ///< User initiated binding method
   CTRLM_RCU_BINDING_TYPE_AUTOMATIC   = 1, ///< Automatic binding method
   CTRLM_RCU_BINDING_TYPE_BUTTON      = 2, ///< Button binding method
   CTRLM_RCU_BINDING_TYPE_SCREEN_BIND = 3, ///< Screen bind method
   CTRLM_RCU_BINDING_TYPE_INVALID     = 4  ///< Invalid binding type
} ctrlm_rcu_binding_type_t;

/// @brief RCU Validation Type
/// @details The type of validation that can be performed between a controller and target.
typedef enum {
   CTRLM_RCU_VALIDATION_TYPE_APPLICATION   = 0, ///< Application based validation
   CTRLM_RCU_VALIDATION_TYPE_INTERNAL      = 1, ///< Control Manager based validation
   CTRLM_RCU_VALIDATION_TYPE_AUTOMATIC     = 2, ///< Autobinding validation
   CTRLM_RCU_VALIDATION_TYPE_BUTTON        = 3, ///< Button based validation
   CTRLM_RCU_VALIDATION_TYPE_PRECOMMISSION = 4, ///< Precommissioned controller
   CTRLM_RCU_VALIDATION_TYPE_SCREEN_BIND   = 5, ///< Screen bind based validation
   CTRLM_RCU_VALIDATION_TYPE_INVALID       = 6  ///< Invalid validation type
} ctrlm_rcu_validation_type_t;

/// @brief RCU Binding Security Type
/// @details The type of binding security used between the controller and target.
typedef enum {
   CTRLM_RCU_BINDING_SECURITY_TYPE_NORMAL   = 0, ///< Normal Security
   CTRLM_RCU_BINDING_SECURITY_TYPE_ADVANCED = 1  ///< Advanced Security
} ctrlm_rcu_binding_security_type_t;

/// @brief RCU Ghost Codes
/// @details The enumeration of ghost codes that can be produced by the controller. XRC spec 11.2.10.1
typedef enum {
   CTRLM_RCU_GHOST_CODE_VOLUME_UNITY_GAIN = 0, ///< Volume unity gain (vol +/- or mute pressed)
   CTRLM_RCU_GHOST_CODE_POWER_OFF         = 1, ///< Power off pressed
   CTRLM_RCU_GHOST_CODE_POWER_ON          = 2, ///< Power on pressed
   CTRLM_RCU_GHOST_CODE_IR_POWER_TOGGLE   = 3, ///< Power toggled
   CTRLM_RCU_GHOST_CODE_IR_POWER_OFF      = 4, ///< Power off pressed
   CTRLM_RCU_GHOST_CODE_IR_POWER_ON       = 5, ///< Power on pressed
   CTRLM_RCU_GHOST_CODE_VOLUME_UP         = 6, ///< Volume up pressed
   CTRLM_RCU_GHOST_CODE_VOLUME_DOWN       = 7, ///< Volume down pressed
   CTRLM_RCU_GHOST_CODE_MUTE              = 8, ///< Mute pressed
   CTRLM_RCU_GHOST_CODE_INPUT             = 9, ///< TV input pressed
   CTRLM_RCU_GHOST_CODE_FIND_MY_REMOTE    = 10,///< User pressed any button in Find My Remote mode
   CTRLM_RCU_GHOST_CODE_INVALID           = 11 ///< Invalid ghost code
} ctrlm_rcu_ghost_code_t;

/// @brief RCU Functions
/// @details The enumeration of functions that can be produced by the controller.
typedef enum {
   CTRLM_RCU_FUNCTION_SETUP                  =  0, ///< <setup> Setup key held for 3 seconds
   CTRLM_RCU_FUNCTION_BACKLIGHT              =  1, ///< <setup><92X> Backlight time where X is on time in seconds
   CTRLM_RCU_FUNCTION_POLL_FIRMWARE          =  2, ///< <setup><964> Poll for a firmware update
   CTRLM_RCU_FUNCTION_POLL_AUDIO_DATA        =  3, ///< <setup><965> Poll for an audio data update
   CTRLM_RCU_FUNCTION_RESET_SOFT             =  4, ///< <setup><980> Soft Reset on the controller
   CTRLM_RCU_FUNCTION_RESET_FACTORY          =  5, ///< <setup><981> Factory Reset on the controller (mode is changed to clip discovery)
   CTRLM_RCU_FUNCTION_BLINK_SOFTWARE_VERSION =  6, ///< <setup><983> Software version
   CTRLM_RCU_FUNCTION_BLINK_AVR_CODE         =  7, ///< <setup><985> AVR Code
   CTRLM_RCU_FUNCTION_RESET_IR               =  8, ///< <setup><986> Reset IR only
   CTRLM_RCU_FUNCTION_RESET_RF               =  9, ///< <setup><987> RF Reset on the controller  (mode is changed to clip discovery)
   CTRLM_RCU_FUNCTION_BLINK_TV_CODE          = 10, ///< <setup><990> Blink the TV code on the LED's
   CTRLM_RCU_FUNCTION_IR_DB_TV_SEARCH        = 11, ///< <setup><991> Library Search for TV's
   CTRLM_RCU_FUNCTION_IR_DB_AVR_SEARCH       = 12, ///< <setup><992> Library Search for AVR's
   CTRLM_RCU_FUNCTION_KEY_REMAPPING          = 13, ///< <setup><994> Key remapping
   CTRLM_RCU_FUNCTION_BLINK_IR_DB_VERSION    = 14, ///< <setup><995> IR DB version
   CTRLM_RCU_FUNCTION_BLINK_BATTERY_LEVEL    = 15, ///< <setup><999> Blink the battery level
   CTRLM_RCU_FUNCTION_DISCOVERY              = 16, ///< <setup><XFINITY> Discovery Request
   CTRLM_RCU_FUNCTION_MODE_IR_CLIP           = 17, ///< <setup><A> Clip mode
   CTRLM_RCU_FUNCTION_MODE_IR_MOTOROLA       = 18, ///< <setup><B> Motorola
   CTRLM_RCU_FUNCTION_MODE_IR_CISCO          = 19, ///< <setup><C> Cisco Mode
   CTRLM_RCU_FUNCTION_MODE_CLIP_DISCOVERY    = 20, ///< <setup><D> Clip Discovery mode
   CTRLM_RCU_FUNCTION_IR_DB_TV_SELECT        = 21, ///< <setup><1####> TV code select
   CTRLM_RCU_FUNCTION_IR_DB_AVR_SELECT       = 22, ///< <setup><3####> AVR code select
   CTRLM_RCU_FUNCTION_INVALID_KEY_COMBO      = 23, ///< Invalid key combo (key combo for XR16, not XR11 or XR15)
   CTRLM_RCU_FUNCTION_INVALID                = 24  ///< Invalid function
} ctrlm_rcu_function_t;

/// @brief RCU Controller Type
/// @details The type of controller.
typedef enum {
   CTRLM_RCU_CONTROLLER_TYPE_XR2     = 0,
   CTRLM_RCU_CONTROLLER_TYPE_XR5     = 1,
   CTRLM_RCU_CONTROLLER_TYPE_XR11    = 2,
   CTRLM_RCU_CONTROLLER_TYPE_XR15    = 3,
   CTRLM_RCU_CONTROLLER_TYPE_XR15V2  = 4,
   CTRLM_RCU_CONTROLLER_TYPE_XR16    = 5,
   CTRLM_RCU_CONTROLLER_TYPE_XR18    = 6,
   CTRLM_RCU_CONTROLLER_TYPE_XR19    = 7,
   CTRLM_RCU_CONTROLLER_TYPE_XRA     = 8,
   CTRLM_RCU_CONTROLLER_TYPE_UNKNOWN = 9,
   CTRLM_RCU_CONTROLLER_TYPE_INVALID = 10
} ctrlm_rcu_controller_type_t;

/// @brief RCU IR DB Type
/// @details The type of IR DB in the controller.
typedef enum {
   CTRLM_RCU_IR_DB_TYPE_UEI     = 0,
   CTRLM_RCU_IR_DB_TYPE_REMOTEC = 1,
   CTRLM_RCU_IR_DB_TYPE_INVALID = 2
} ctrlm_rcu_ir_db_type_t;

/// @brief RCU IR DB State
/// @details The state of IR DB in the controller.
typedef enum {
   CTRLM_RCU_IR_DB_STATE_NO_CODES       = 0,
   CTRLM_RCU_IR_DB_STATE_TV_CODE        = 1,
   CTRLM_RCU_IR_DB_STATE_AVR_CODE       = 2,
   CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES   = 3,
   CTRLM_RCU_IR_DB_STATE_IR_RF_DB_CODES = 4,
   CTRLM_RCU_IR_DB_STATE_INVALID        = 5
} ctrlm_rcu_ir_db_state_t;


/// @brief RCU Battery Event
/// @details The events associate with the battery in the controller.
typedef enum {
   CTRLM_RCU_BATTERY_EVENT_NONE         = 0,
   CTRLM_RCU_BATTERY_EVENT_REPLACED     = 1,
   CTRLM_RCU_BATTERY_EVENT_CHARGING     = 2,
   CTRLM_RCU_BATTERY_EVENT_PENDING_DOOM = 3,
   CTRLM_RCU_BATTERY_EVENT_75_PERCENT   = 4,
   CTRLM_RCU_BATTERY_EVENT_50_PERCENT   = 5,
   CTRLM_RCU_BATTERY_EVENT_25_PERCENT   = 6,
   CTRLM_RCU_BATTERY_EVENT_0_PERCENT    = 7, 
   CTRLM_RCU_BATTERY_EVENT_INVALID      = 8,
} ctrlm_rcu_battery_event_t;

/// @brief RCU Reverse Command Type
/// @details An enumeration of the reverse command types.
typedef enum {
   CTRLM_RCU_REVERSE_CMD_FIND_MY_REMOTE = 0,
   CTRLM_RCU_REVERSE_CMD_REBOOT         = 1,
} ctrlm_rcu_reverse_cmd_t;

/// @brief RCU Reverse Command Event
/// @details The events associate with the Action performed on the controller.
typedef enum {
   CTRLM_RCU_REVERSE_CMD_SUCCESS                = 0,
   CTRLM_RCU_REVERSE_CMD_FAILURE                = 1,
   CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_CAPABLE = 2,
   CTRLM_RCU_REVERSE_CMD_CONTROLLER_NOT_FOUND   = 3,
   CTRLM_RCU_REVERSE_CMD_CONTROLLER_FOUND       = 4,
   CTRLM_RCU_REVERSE_CMD_USER_INTERACTION       = 5,
   CTRLM_RCU_REVERSE_CMD_DISABLED               = 6,
   CTRLM_RCU_REVERSE_CMD_INVALID                = 7,
} ctrlm_rcu_reverse_cmd_result_t;

typedef enum {
   CTRLM_RCU_FMR_ALERT_FLAGS_ID = 1,           // combination of flags defined in ctrlm_rcu_find_my_remote_alert_flag_t
   CTRLM_FIND_RCU_FMR_ALERT_DURATION_ID = 2,   // unsigned integer alert duration in msec
} ctrlm_rcu_find_my_remote_parameter_id_t;

/// @brief RCU Alert flags
typedef enum {
  CTRLM_RCU_ALERT_AUDIBLE = 0x01,
  CTRLM_RCU_ALERT_VISUAL  = 0x02
} ctrlm_rcu_alert_flags_t;

/// @brief RCU DSP Event
/// @details The events associate with the dsp in the controller.
typedef enum {
   CTRLM_RCU_DSP_EVENT_MIC_FAILURE      = 1,
   CTRLM_RCU_DSP_EVENT_SPEAKER_FAILURE  = 2,
   CTRLM_RCU_DSP_EVENT_INVALID          = 3,
} ctrlm_rcu_dsp_event_t;

typedef enum {
   CONTROLLER_REBOOT_POWER_ON      = 0,
   CONTROLLER_REBOOT_EXTERNAL      = 1,
   CONTROLLER_REBOOT_WATCHDOG      = 2,
   CONTROLLER_REBOOT_CLOCK_LOSS    = 3,
   CONTROLLER_REBOOT_BROWN_OUT     = 4,
   CONTROLLER_REBOOT_OTHER         = 5,
   CONTROLLER_REBOOT_ASSERT_NUMBER = 6
} controller_reboot_reason_t;

typedef enum {
   RCU_POLLING_ACTION_NONE                  = 0x00,
   RCU_POLLING_ACTION_REBOOT                = 0x01,
   RCU_POLLING_ACTION_REPAIR                = 0x02,
   RCU_POLLING_ACTION_CONFIGURATION         = 0x03,
   RCU_POLLING_ACTION_OTA                   = 0x04,
   RCU_POLLING_ACTION_ALERT                 = 0x05,
   RCU_POLLING_ACTION_IRDB_STATUS           = 0x06,
   RCU_POLLING_ACTION_POLL_CONFIGURATION    = 0x07,
   RCU_POLLING_ACTION_VOICE_CONFIGURATION   = 0x08,
   RCU_POLLING_ACTION_DSP_CONFIGURATION     = 0x09,
   RCU_POLLING_ACTION_METRICS               = 0x0A,
   RCU_POLLING_ACTION_EOS                   = 0x0B,
   RCU_POLLING_ACTION_SETUP_COMPLETE        = 0x0C,
   RCU_POLLING_ACTION_BATTERY_STATUS        = 0x0D,
   RCU_POLLING_ACTION_PROFILE_CONFIGURATION = 0x0E,
   RCU_POLLING_ACTION_IRRF_STATUS           = 0x10
} ctrlm_rcu_polling_action_t;

/// @}

/// @addtogroup CTRLM_IPC_RCU_STRUCTS
/// @{
/// @brief Structure Definitions
/// @details The Control Manager provides structures that are used in IARM calls and events.

/// @brief Structure of Remote Controls's Validation Finish IARM call
/// @details This structure provides information about the completion of a controller validation.
typedef struct {
   unsigned char                 api_revision;      ///< Revision of this API
   ctrlm_iarm_call_result_t      result;            ///< Result of the IARM call
   ctrlm_network_id_t            network_id;        ///< The identifier of network on which the controller is bound
   ctrlm_controller_id_t         controller_id;     ///< The identifier of the controller
   ctrlm_rcu_validation_result_t validation_result; ///< Result of the validation attempt
} ctrlm_rcu_iarm_call_validation_finish_t;

/// @brief Controller Status Structure
/// @details This structure contains a controller's diagnostic information.
typedef struct {
   unsigned long long                ieee_address;                                          ///< The 64-bit IEEE Address
   unsigned short                    short_address;                                         ///< Short address (if applicable)
   unsigned long                     time_binding;                                          ///< Time that the controller was bound (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   ctrlm_rcu_binding_type_t          binding_type;                                          ///< Type of binding that was performed
   ctrlm_rcu_validation_type_t       validation_type;                                       ///< Type of validation that was performed
   ctrlm_rcu_binding_security_type_t security_type;                                         ///< Security type for the binding
   unsigned long                     command_count;                                         ///< Amount of commands received from this controller
   ctrlm_key_code_t                  last_key_code;                                         ///< Last key code received
   ctrlm_key_status_t                last_key_status;                                       ///< Key status of last key code
   unsigned char                     link_quality_percent;                                  ///< Link quality percentage (0-100)
   unsigned char                     link_quality;                                          ///< Link quality indicator
   char                              manufacturer[CTRLM_RCU_MAX_MANUFACTURER_LENGTH];       ///< Manufacturer of the controller
   char                              chipset[CTRLM_RCU_MAX_CHIPSET_LENGTH];                 ///< Chipset of the controller
   char                              version_software[CTRLM_RCU_VERSION_LENGTH];            ///< Software version of controller
   char                              version_dsp[CTRLM_RCU_VERSION_LENGTH];                 ///< DSP version of controller
   char                              version_keyword_model[CTRLM_RCU_VERSION_LENGTH];       ///< Keyword model version of controller
   char                              version_arm[CTRLM_RCU_VERSION_LENGTH];                 ///< ARM version of controller
   char                              version_hardware[CTRLM_RCU_VERSION_LENGTH];            ///< Hardware version of controller
   char                              version_irdb[CTRLM_RCU_VERSION_LENGTH];                ///< IR database version (if available)
   char                              version_build_id[CTRLM_RCU_BUILD_ID_LENGTH];           ///< Build ID of software on controller
   char                              version_dsp_build_id[CTRLM_RCU_DSP_BUILD_ID_LENGTH];   ///< DSP Build ID of software on controller
   char                              version_bootloader[CTRLM_RCU_VERSION_LENGTH];          ///< Bootloader
   char                              version_golden[CTRLM_RCU_VERSION_LENGTH];              ///< Golden Software version
   char                              version_audio_data[CTRLM_RCU_VERSION_LENGTH];          ///< Audio Data version (if available)
   unsigned char                     firmware_updated;                                      ///< Boolean value indicating that the controller's firmware has been updated (1) or not (0)
   unsigned char                     has_battery;                                           ///< Boolean value indicating that the controller has a battery (1) or not (0)
   unsigned char                     battery_level_percent;                                 ///< Battery Level percentage (0-100)
   float                             battery_voltage_loaded;                                ///< Battery Voltage under load
   float                             battery_voltage_unloaded;                              ///< Battery Voltage not under load
   unsigned long                     time_last_key;                                         ///< Time of last key code received (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned long                     time_battery_update;                                   ///< Time of battery update (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned long                     time_battery_changed;                                  ///< Time of battery changed (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_changed_actual_percentage;                     ///< Actual percentage of the batteries when the were "changed"
   float                             battery_changed_unloaded_voltage;                      ///< Actual voltage of the batteries when the were "changed"
   unsigned long                     time_battery_75_percent;                               ///< Time of battery at 75% (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_75_percent_actual_percentage;                  ///< Actual percentage of the batteries when they hit 75% or below
   float                             battery_75_percent_unloaded_voltage;                   ///< Actual voltage of the batteries when they hit 75% or below
   unsigned long                     time_battery_50_percent;                               ///< Time of battery at 50% (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_50_percent_actual_percentage;                  ///< Actual percentage of the batteries when they hit 50% or below
   float                             battery_50_percent_unloaded_voltage;                   ///< Actual voltage of the batteries when they hit 50% or below
   unsigned long                     time_battery_25_percent;                               ///< Time of battery at 25% (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_25_percent_actual_percentage;                  ///< Actual percentage of the batteries when they hit 25% or below
   float                             battery_25_percent_unloaded_voltage;                   ///< Actual voltage of the batteries when they hit 25% or below
   unsigned long                     time_battery_5_percent;                                ///< Time of battery at 5% (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_5_percent_actual_percentage;                   ///< Actual percentage of the batteries when they hit 5% or below
   float                             battery_5_percent_unloaded_voltage;                    ///< Actual voltage of the batteries when they hit 5% or below
   unsigned long                     time_battery_0_percent;                                ///< Time of battery at 0% (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   unsigned char                     battery_0_percent_actual_percentage;                   ///< Actual percentage of the batteries when they hit 0% or below
   float                             battery_0_percent_unloaded_voltage;                    ///< Actual voltage of the batteries when they hit 0% or below
   ctrlm_rcu_battery_event_t         battery_event;                                         ///< Last battery event
   unsigned long                     time_battery_event;                                    ///< Time of the last battery event (number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC))
   char                              type[CTRLM_RCU_MAX_USER_STRING_LENGTH];                ///< Remote control's type string
   ctrlm_rcu_ir_db_type_t            ir_db_type;                                            ///< Type of IR Database in the controller
   ctrlm_rcu_ir_db_state_t           ir_db_state;                                           ///< State of IR Database in the controller
   char                              ir_db_code_tv[CTRLM_RCU_MAX_IR_DB_CODE_LENGTH];        ///< Current TV Code programmed in the controller
   char                              ir_db_code_avr[CTRLM_RCU_MAX_IR_DB_CODE_LENGTH];       ///< Current AVR Code programmed in the controller
   unsigned long                     voice_cmd_count_today;                                 ///< Number of normal voice commands received today
   unsigned long                     voice_cmd_count_yesterday;                             ///< Number of normal voice commands received yesterday
   unsigned long                     voice_cmd_short_today;                                 ///< Number of short voice commands received today
   unsigned long                     voice_cmd_short_yesterday;                             ///< Number of short voice commands received yesterday
   unsigned long                     voice_packets_sent_today;                              ///< Number of voice packets sent today
   unsigned long                     voice_packets_sent_yesterday;                          ///< Number of voice packets sent yesterday
   unsigned long                     voice_packets_lost_today;                              ///< Number of voice packets lost today
   unsigned long                     voice_packets_lost_yesterday;                          ///< Number of voice packets lost yesterday
   float                             voice_packet_loss_average_today;                       ///< The average packet loss for today (sent-received)/sent
   float                             voice_packet_loss_average_yesterday;                   ///< The average packet loss for yesterday (sent-received)/sent
   unsigned long                     utterances_exceeding_packet_loss_threshold_today;      ///< Number utterances exceeding the packet loss threshold today
   unsigned long                     utterances_exceeding_packet_loss_threshold_yesterday;  ///< Number utterances exceeding the packet loss threshold yesterday
   unsigned char                     checkin_for_device_update;                             ///< Boolean value indicating that the controller has checked in in the last x hours for device update
   unsigned char                     ir_db_code_download_supported;                         ///< Boolean value indicating that the controller supports irdb code download
   unsigned char                     has_dsp;                                               ///< Boolean value indicating that the controller has a dsp chip (1) or not (0)
   unsigned long                     average_time_in_privacy_mode;                          ///< Average time in privacy mode
   unsigned char                     in_privacy_mode;                                       ///< Boolean value indicating whether the controller is currently in privacy mode
   unsigned char                     average_snr;                                           ///< Average signal to noise ratio
   unsigned char                     average_keyword_confidence;                            ///< Average keyword confidence
   unsigned char                     total_number_of_mics_working;                          ///< Total number of mics that are working
   unsigned char                     total_number_of_speakers_working;                      ///< Total number of speakers that are working
   unsigned int                      end_of_speech_initial_timeout_count;                   ///< End of speech initial timeout count
   unsigned int                      end_of_speech_timeout_count;                           ///< End of speech timeout count
   unsigned long                     time_uptime_start;                                     ///< The time uptime started counting
   unsigned long                     uptime_seconds;                                        ///< The uptime of the remote in seconds
   unsigned long                     privacy_time_seconds;                                  ///< Total amount of time remote is in privacy mode
   unsigned char                     reboot_reason;                                         ///< The last remote reboot reason
   unsigned char                     reboot_voltage;                                        ///< The last remote reboot voltage
   unsigned int                      reboot_assert_number;                                  ///< The last remote assert_number 
   unsigned long                     reboot_timestamp;                                      ///< Time of the last remote reboot
   unsigned long                     time_last_heartbeat;                                   ///< The time of the last heartbeat
   char                              irdb_entry_id_name_tv[CTRLM_MAX_PARAM_STR_LEN];        ///< The TV irdb code name
   char                              irdb_entry_id_name_avr[CTRLM_MAX_PARAM_STR_LEN];       ///< The AVR irdb code name
   unsigned char                     battery_voltage_large_jump_counter;                    ///< The large jump counter for battery voltage
   unsigned char                     battery_voltage_large_decline_detected;                ///< The large decline detected flag for battery voltage
} ctrlm_controller_status_t;

/// @brief Structure of Remote Controls's Controller Status IARM call
/// @details This structure provides information about a controller.
typedef struct {
   unsigned char             api_revision;  ///< Revision of this API
   ctrlm_iarm_call_result_t  result;        ///< Result of the IARM call
   ctrlm_network_id_t        network_id;    ///< IN The identifier of network on which the controller is bound
   ctrlm_controller_id_t     controller_id; ///< IN
   ctrlm_controller_status_t status;        ///< Status of the controller
} ctrlm_rcu_iarm_call_controller_status_t;

/// @brief Structure of Remote Controls's RIB Request get and set IARM calls
/// @details This structure provides information about a controller's RIB entry.
typedef struct {
   unsigned char            api_revision;                           ///< Revision of this API
   ctrlm_iarm_call_result_t result;                                 ///< Result of the IARM call
   ctrlm_network_id_t       network_id;                             ///< IN - identifier of network on which the controller is bound
   ctrlm_controller_id_t    controller_id;                          ///< IN - identifier of the controller
   ctrlm_rcu_rib_attr_id_t  attribute_id;                           ///< RIB attribute identifier
   unsigned char            attribute_index;                        ///< RIB attribute index
   unsigned char            length;                                 ///< RIB data length
   char                     data[CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE]; ///< RIB entry's data
} ctrlm_rcu_iarm_call_rib_request_t;

/// @brief Structure of Remote Controls's Controller Link Key IARM call
/// @details This stucture provides the controllers link key
typedef struct {
   unsigned char             api_revision;  ///< Revision of this API
   ctrlm_iarm_call_result_t  result;        ///< Result of the IARM call
   ctrlm_network_id_t        network_id;    ///< IN The identifier of network on which the controller is bound
   ctrlm_controller_id_t     controller_id; ///< IN
   unsigned char             link_key[16];  ///< OUT The link key for the controller
} ctrlm_rcu_iarm_call_controller_link_key_t;

/// @brief Structure of Remote Control's Key Press IARM event
/// @details This event notifies listeners that a key event has occurred.
typedef struct {
   unsigned char            api_revision;                                      ///< Revision of this API
   ctrlm_network_id_t       network_id;                                        ///< identifier of network on which the controller is bound
   ctrlm_network_type_t     network_type;                                      ///< type of network on which the controller is bound
   ctrlm_controller_id_t    controller_id;                                     ///< identifier of the controller on which the key was pressed
   ctrlm_key_status_t       key_status;                                        ///< status of the key press (down, repeat, up)
   ctrlm_key_code_t         key_code;                                          ///< received key code
   ctrlm_rcu_binding_type_t binding_type;                                      ///< Type of binding that was performed
   char                     controller_type[CTRLM_RCU_MAX_USER_STRING_LENGTH]; ///< Remote control's type string
} ctrlm_rcu_iarm_event_key_press_t;

/// @brief Structure of Remote Control's Validation Begin IARM event
/// @details This event notifies listeners that a validation attempt has begun.
typedef struct {
   unsigned char               api_revision;                                      ///< Revision of this API
   ctrlm_network_id_t          network_id;                                        ///< identifier of network on which the controller is bound
   ctrlm_network_type_t        network_type;                                      ///< type of network on which the controller is bound
   ctrlm_controller_id_t       controller_id;                                     ///< identifier of the controller on which the validation is being performed
   ctrlm_rcu_binding_type_t    binding_type;                                      ///< Type of binding that is being performed
   ctrlm_rcu_validation_type_t validation_type;                                   ///< Type of validation that is being performed
   ctrlm_key_code_t            validation_keys[CTRLM_RCU_VALIDATION_KEY_QTY];     ///< Validation keys to be displayed for internal validation
   char                        controller_type[CTRLM_RCU_MAX_USER_STRING_LENGTH]; ///< Remote control's type string
} ctrlm_rcu_iarm_event_validation_begin_t;

/// @brief Structure of Remote Control's Validation End IARM event
/// @details This event notifies listeners that a validation attempt has completed.
typedef struct {
   unsigned char                 api_revision;                                      ///< Revision of this API
   ctrlm_network_id_t            network_id;                                        ///< identifier of network on which the controller is bound
   ctrlm_network_type_t          network_type;                                      ///< type of network on which the controller is bound
   ctrlm_controller_id_t         controller_id;                                     ///< identifier of the controller on which the validation was performed
   ctrlm_rcu_binding_type_t      binding_type;                                      ///< Type of binding that was performed
   ctrlm_rcu_validation_type_t   validation_type;                                   ///< Type of validation that was performed
   ctrlm_rcu_validation_result_t result;                                            ///< Result of the validation attempt
   char                          controller_type[CTRLM_RCU_MAX_USER_STRING_LENGTH]; ///< Remote control's type string
} ctrlm_rcu_iarm_event_validation_end_t;

/// @brief Structure of Remote Control's Configuration Complete IARM event
/// @details This event notifies listeners that a validation attempt has completed.
typedef struct {
   unsigned char                    api_revision;                                      ///< Revision of this API
   ctrlm_network_id_t               network_id;                                        ///< identifier of network on which the controller is bound
   ctrlm_network_type_t             network_type;                                      ///< type of network on which the controller is bound
   ctrlm_controller_id_t            controller_id;                                     ///< identifier of the controller on which the validation was performed
   ctrlm_rcu_configuration_result_t result;                                            ///< Result of the configuration attempt
   ctrlm_rcu_binding_type_t         binding_type;                                      ///< Type of binding that was performed
   char                             controller_type[CTRLM_RCU_MAX_USER_STRING_LENGTH]; ///< Remote control's type string
   ctrlm_controller_status_t        status;                                            ///< Remote control's status
} ctrlm_rcu_iarm_event_configuration_complete_t;

/// @brief Structure of Remote Control's Setup Key IARM event
/// @details This event notifies listeners that a remote control function event has occurred.
typedef struct {
   unsigned char         api_revision;                                  ///< Revision of this API
   ctrlm_network_id_t    network_id;                                    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t  network_type;                                  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t controller_id;                                 ///< Identifier of the controller on which the key was pressed
   ctrlm_rcu_function_t  function;                                      ///< Function that was performed on the controller
   unsigned long         value;                                         ///< Value associated with the function (if applicable)
} ctrlm_rcu_iarm_event_function_t;

/// @brief Structure of Remote Control's Ghost Key IARM event
/// @details This event notifies listeners that a ghost code event has occurred.
typedef struct {
   unsigned char          api_revision;          ///< Revision of this API
   ctrlm_network_id_t     network_id;            ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t   network_type;          ///< Type of network on which the controller is bound
   ctrlm_controller_id_t  controller_id;         ///< Identifier of the controller on which the key was pressed
   ctrlm_rcu_ghost_code_t ghost_code;            ///< Ghost code
   unsigned char          remote_keypad_config;  /// The remote keypad configuration (Has Setup/NumberKeys).
} ctrlm_rcu_iarm_event_key_ghost_t;

/// @brief Structure of Remote Control's Ghost Key IARM event
/// @details This event notifies listeners that a ghost code event has occurred.
typedef struct {
   unsigned char          api_revision;                                    ///< Revision of this API
   int                    controller_id;                                   ///< Identifier of the controller on which the key was pressed
   char                   event_source[CTRLM_RCU_MAX_EVENT_SOURCE_LENGTH]; ///< The key source
   char                   event_type[CTRLM_RCU_MAX_EVENT_TYPE_LENGTH];     ///< The control type
   char                   event_data[CTRLM_RCU_MAX_EVENT_DATA_LENGTH];     ///< The data
   int                    event_value;                                     ///< The value
   int                    spare_value;                                     ///< A spare value (sfm needs this extra one)
} ctrlm_rcu_iarm_event_control_t;

/// @brief Structure of Remote Control's RIB Entry Access IARM event
/// @details The RIB Entry Access Event uses this structure. See the @link CTRLM_IPC_RCU_EVENTS Events@endlink section for more details on registering to receive this event.
typedef struct {
   unsigned char           api_revision;  ///< Revision of this API
   ctrlm_network_id_t      network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t    network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t   controller_id; ///< Identifier of the controller
   ctrlm_rcu_rib_attr_id_t identifier;    ///< RIB attribute identifier
   unsigned char           index;         ///< RIB attribute index
   ctrlm_access_type_t     access_type;   ///< RIB access type (read/write)
} ctrlm_rcu_iarm_event_rib_entry_access_t;

/// @brief Structure of Remote Control's remote reboot IARM event
/// @details This event notifies listeners that a remote rebooot event has occurred.
typedef struct {
   unsigned char              api_revision;  ///< Revision of this API
   ctrlm_network_id_t         network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t       network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t      controller_id; ///< Identifier of the controller
   unsigned char              voltage;       ///< Voltage when reboot reason is CONTROLLER_REBOOT_ASSERT_NUMBER
   controller_reboot_reason_t reason;        ///< Remote reboot reason
   unsigned long              timestamp;     ///< Reboot timestamp
   unsigned int               assert_number; ///< Assert Number when reboot reason is CONTROLLER_REBOOT_ASSERT_NUMBER
} ctrlm_rcu_iarm_event_remote_reboot_t;

/// @brief Structure of Remote Control's Battery IARM event
/// @details This event notifies listeners that a DSP event has occurred.
typedef struct {
   unsigned char              api_revision;  ///< Revision of this API
   ctrlm_network_id_t         network_id;    ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t       network_type;  ///< Type of network on which the controller is bound
   ctrlm_controller_id_t      controller_id; ///< Identifier of the controller
   ctrlm_rcu_battery_event_t  battery_event; ///< Battery event
   unsigned char              percent;       ///< Battery percentage
} ctrlm_rcu_iarm_event_battery_t;

/// @brief Structure of Remote Control's remote reboot IARM event
/// @details This event notifies listeners that a remote rebooot event has occurred.
typedef struct {
   unsigned char                 api_revision;       ///< Revision of this API
   ctrlm_rcu_validation_result_t validation_result;  ///< Result of the validation
} ctrlm_rcu_iarm_event_rf4ce_pairing_window_timeout_t;

typedef struct {
   time_t        time_uptime_start;
   unsigned long uptime_seconds;
   unsigned long privacy_time_seconds;
} uptime_privacy_info_t;

/// @brief Remote Controller Reverse Command Structure
/// @details Remote Controller Reverse Command structure is used in the CTRLM_MAIN_IARM_CALL_RCU_REVERSE_CMD call.
/// ctrlm_main_iarm_call_rcu_reverse_cmd_t memory structure:
///
/// size of structure: sizeof (struct ctrlm_main_iarm_call_rcu_reverse_cmd_t) + (param data size) - 1
///
/// Find My Remote
/// cmd = CTRLM_RCU_REVERSE_CMD_FIND_MY_REMOTE
/// num_params = 2
///
/// parameter id = CTRLM_RCU_FMR_ALERT_FLAGS_ID
/// parameter length 1 byte
/// parameter values CTRLM_RCU_ALERT_AUDIBLE | CTRLM_RCU_ALERT_VISUAL
///
/// parameter id = CTRLM_FIND_RCU_FMR_ALERT_DURATION_ID
/// parameter length 2 bytes unsigned short
/// parameter values chime diration in milliseconds
/// memory needed: sizeof (ctrlm_main_iarm_call_rcu_reverse_cmd_t) + 2


typedef struct {
   unsigned char  param_id; ///< parameter id
   unsigned long  size;     ///< parameter size, in bytes
} ctrlm_rcu_reverse_cmd_param_descriptor_t;

typedef struct {
   unsigned char            api_revision;     ///< [in]  Revision of this API
   ctrlm_iarm_call_result_t result;           ///< [out] Result of the IARM call
   ctrlm_network_type_t     network_type;     ///< [in]  Type of network on which the controller is bound
   ctrlm_controller_id_t    controller_id;    ///< [in]  Identifier of the controller. controller ID, CTRLM_MAIN_CONTROLLER_ID_ALL or CTRLM_MAIN_CONTROLLER_ID_LAST_USED
   ctrlm_rcu_reverse_cmd_t  cmd;              ///< [in]  command ID
   ctrlm_rcu_reverse_cmd_result_t cmd_result; ///< [out] Reverse Command result
   unsigned long            total_size;       ///< [in]  ctrlm_main_iarm_call_rcu_reverse_cmd_t + data size
   unsigned char            num_params;       ///< [in]  number of parameters
   ctrlm_rcu_reverse_cmd_param_descriptor_t params_desc[CTRLM_RCU_CALL_RCU_REVERSE_CMD_PARAMS_MAX];   ///< command parameter descriptor
   unsigned char            param_data[1];    ///< parameters data
} ctrlm_main_iarm_call_rcu_reverse_cmd_t;

typedef struct {
   unsigned char                  api_revision;     ///< Revision of this API
   ctrlm_network_id_t             network_id;       ///< Identifier of network on which the controller is bound
   ctrlm_network_type_t           network_type;     ///< Type of network on which the controller is bound
   ctrlm_controller_id_t          controller_id;    ///< Identifier of the controller on which the key was pressed
   ctrlm_rcu_reverse_cmd_t        action;           ///< Reverse Command that was performed on the controller
   ctrlm_rcu_reverse_cmd_result_t result;           ///< Reverse Command result
   int                            result_data_size; ///< Result Data Size
   unsigned char                  result_data[1];   ///< Result Data buffer
} ctrlm_rcu_iarm_event_reverse_cmd_t;

/// @brief Structure of Remote HeartBeat response polling action IARM call
/// @details This structure provides information about a controller's HeartBeat response polling action.
typedef struct {
   unsigned char              api_revision;                               ///< Revision of this API
   ctrlm_iarm_call_result_t   result;                                     ///< Result of the IARM call
   ctrlm_network_id_t         network_id;                                 ///< IN - identifier of network on which the controller is bound
   ctrlm_controller_id_t      controller_id;                              ///< IN - identifier of the controller
   unsigned char              action;                                     ///< IN - Polling action performed on the controller
   char                       data[CTRLM_RCU_POLLING_RESPONSE_DATA_LEN];  ///< IN - Polling data
} ctrlm_rcu_iarm_call_rf4ce_polling_action_t;

/// @addtogroup CTRLM_IPC_RCU_CALLS
/// @{
/// @brief Remote Calls accessible via IARM bus
/// @details IARM calls are synchronous calls to functions provided by other IARM bus members. Each bus member can register
/// calls using the IARM_Bus_RegisterCall() function. Other members can invoke the call using the IARM_Bus_Call() function.
///
/// - - -
/// Call Registration
/// -----------------
///
/// The Device Update component of Control Manager registers the following calls.
///
/// | Bus Name                 | Call Name                             | Argument                             | Description |
/// | :-------                 | :--------                             | :-------                             | :---------- |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_CALL_VALIDATION_FINISH | ctrlm_rcu_iarm_validation_finish_t * | Completes the controller validation |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS | ctrlm_rcu_iarm_controller_status_t * | Provides information about a controller |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_CALL_IR_CODE_SET       | ctrlm_rcu_iarm_ir_code_t *           | Sets an IR code in the controller |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET   | ctrlm_rcu_iarm_rib_request_t *       | Retrieves an attribute from the controller's RIB |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET   | ctrlm_rcu_iarm_rib_request_t *       | Sets an attribute in the controller's RIB  |
///
/// Examples:
///
/// Get a controller's status:
///
///     IARM_Result_t                      result;
///     ctrlm_rcu_iarm_controller_status_t status;
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS, (void *)&status, sizeof(status));
///     if(IARM_RESULT_SUCCESS == result && CTRLM_IARM_CALL_RESULT_SUCCESS == status.result) {
///         // Status was successfully retrieved
///     }
///
/// Set an attribute in the controller's RIB:
///
///     IARM_Result_t                result;
///     ctrlm_rcu_iarm_rib_request_t request;
///     request.network_id    = 0x00;                                 // The network ID
///     request.controller_id = 0x01;                                 // The controller ID
///     request.attribute     = CTRLM_RCU_RIB_ATTR_ID_DOWNLOAD_RATE;  // The RIB attribute
///     request.index         = 0;                                    // The index into the attribute
///     request.length        = CTRLM_RCU_RIB_ATTR_LEN_DOWNLOAD_RATE; // The attribute data length
///     request.data[0]       = 0;                                    // The attribute data
///
///     result = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&request, sizeof(request));
///     if(IARM_RESULT_SUCCESS == result && CTRLM_IARM_CALL_RESULT_SUCCESS == request.result) {
///         // RIB attribute was set successfully
///     }
///     }
///
/// @}

/// @addtogroup CTRLM_IPC_RCU_EVENTS
/// @{
/// @brief Broadcast Events accessible via IARM bus
/// @details The IARM bus uses events to broadcast information to interested clients. An event is sent separately to each client. There are no return values for an event and no
/// guarantee that a client will receive the event.  Each event has a different argument structure according to the information being transferred to the clients.  The events that the
/// Remote Control component in Control Manager generates and subscribes to are detailed below.
///
/// - - -
/// Event Generation (Broadcast)
/// ----------------------------
///
/// The Remote Control component generates events that can be received by other processes connected to the IARM bus. The following events
/// are registered during initialization:
///
/// | Bus Name                 | Event Name                                 | Argument                                  | Description |
/// | :-------                 | :---------                                 | :-------                                  | :---------- |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_KEY_PRESS             | ctrlm_rcu_iarm_event_key_press_t *        | Generated each time a key event occurs (down, repeat, up) |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_VALIDATION_BEGIN      | ctrlm_rcu_iarm_event_validation_begin_t * | Generated at the beginning of a validation attempt |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_VALIDATION_KEY_UPDATE | ctrlm_rcu_iarm_event_key_press_t *        | Generated when the user enters a validation code digit/letter |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_VALIDATION_END        | ctrlm_rcu_iarm_event_validation_end_t *   | Generated at the end of a validation attempt |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_KEY_SETUP             | ctrlm_rcu_iarm_event_key_setup_t *        | Generated when the setup key combo is entered on a controller |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_KEY_GHOST             | ctrlm_rcu_iarm_event_key_ghost_t *        | Generated when a ghost code is received from a controller |
/// | CTRLM_MAIN_IARM_BUS_NAME | CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER | ctrlm_rcu_iarm_event_rib_entry_access_t * | Generated when a controller accesses the RIB |
///
/// IARM events are available on a subscription basis. In order to receive an event, a client must explicitly register to receive the event by calling
/// IARM_Bus_RegisterEventHandler() with the bus name, event name and a @link IARM_EventHandler_t handler function@endlink. Events may be generated at any time by the
/// Remote Control component. All events are asynchronous.
///
/// Examples:
///
/// Register for a Remote Control event:
///
///     IARM_Result_t result;
///
///     result = IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_KEY_PRESS, key_press_handler_func);
///     if(IARM_RESULT_SUCCESS == result) {
///         // Event registration was set successfully
///     }
///     }
///
/// @}
///
/// @addtogroup CTRLM_IPC_RCU_COMMS
/// @{
/// @brief Communication Interfaces
/// @details The following diagrams detail the main communication paths for the RCU component.
///
/// ---------------------------------
/// RF4CE Controller Binding - Normal
/// ---------------------------------
///
/// The binding procedure between the controller and target (STB) is initiated on the controller.  The controller will send out a discovery request
/// and collect the responses.  The responses are prioritized (ordered) according to the likelihood that it is the intended target.  The controller
/// will attempt to pair and bind with each target once until it has successfully bound to a target, the operation is aborted, or it has exhausted
/// the candidate list.  The flow of the binding procedure is shown below.
///
/// Controller View
///
/// @dot
/// digraph BINDING_COMMS_CONTROLLER {
///     "UNBOUND"         [shape="ellipse", fontname=Helvetica, fontsize=10, label="UNBOUND"];
///     "IS_DISC_OK"      [shape="diamond", fontname=Helvetica, fontsize=10, label="Successful\ndiscovery?"];
///     "IS_LIST_EMPTY"   [shape="diamond", fontname=Helvetica, fontsize=10, label="Is another\ntarget available?"];
///     "IS_PAIR_OK"      [shape="diamond", fontname=Helvetica, fontsize=10, label="Pairing\nsuccess?"];
///     "IS_VAL_OK"       [shape="diamond", fontname=Helvetica, fontsize=10, label="Validation\nsuccess?"];
///     "BOUND"           [shape="ellipse", fontname=Helvetica, fontsize=10, label="BOUND"];
///
///     node[shape=none, width=0, height=0, label=""];
///     {rank=same;  a3; a4; a5;}
///
///     "UNBOUND"       -> "IS_DISC_OK"    [dir="forward", fontname=Helvetica, fontsize=10,label="  Discovery initiated\non controller"];
///     "IS_DISC_OK"    -> "IS_LIST_EMPTY" [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, sort pairing\ntarget list."];
///     "IS_LIST_EMPTY" -> "IS_PAIR_OK"    [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, perform\npairing procedure."];
///     "IS_PAIR_OK"    -> "IS_VAL_OK"     [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, perform\nvalidation procedure."];
///     "IS_VAL_OK"     -> "BOUND"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes"];
///
///     {rank=same; UNBOUND; IS_DISC_OK; IS_LIST_EMPTY; IS_PAIR_OK; IS_VAL_OK; BOUND;}
///     {rank=same; b1; b2; b3; b4; b5;}
///
///     edge[dir=none];
///     a3 -> a4 -> a5
///     b1 -> b2 -> b3
///     a4 -> "IS_PAIR_OK"    [fontname=Helvetica, fontsize=10,label="  No, go to next target."];
///     a5 -> "IS_VAL_OK"     [fontname=Helvetica, fontsize=10,label="  No, unpair and go\n  to next target."];
///     b2 -> "IS_DISC_OK"    [fontname=Helvetica, fontsize=10,label="  No"];
///     "IS_LIST_EMPTY" -> b3 [fontname=Helvetica, fontsize=10,label="  No"];
///
///     edge[dir=forward];
///     b1 -> "UNBOUND";
///     a3 -> "IS_LIST_EMPTY";
/// }
/// \enddot
///
/// Target View (Control Manager)
///
/// @dot
/// digraph BINDING_COMMS_TARGET {
///     node[fontname=Helvetica, fontsize=10];
///     b1 [shape="ellipse", label="IDLE"];
///     b2 [shape="diamond", label="Is bind table\lfull or binding\lin progress?"];
///     b3 [shape="ellipse", label="VALIDATION\lIN PROGRESS"];
///     b4 [shape="ellipse", label="VALIDATION\lCOMPLETE"];
///
///     node[shape=none, width=0, height=0, label=""];
///     {rank=same; a1, a2, a3; a4;}
///     {rank=same; b1; b2; b3; b4;}
///     {rank=same; c1; c2; c3; c4;}
///
///     edge[dir=none];
///     c1 -> c2 -> c3 -> c4
///
///     edge[dir=forward, fontname=Helvetica, fontsize=10];
///     b1 -> b2 [label="  HAL bind validation\l  start called"];
///     b3 -> b3 [label="  Other key pressed\lon this remote"];
///     b2 -> b3 [label="  No, send IARM validation\l  start event"];
///     a3 -> b3 [dir=none, label="  IARM validation\l  complete called"]
///     a3 -> a4 [dir=none]
///     a4 -> b4
///
///     edge[dir=none, fontname=Helvetica, fontsize=10];
///     b1 -> c1 [dir=back];
///     b2 -> c2 [dir=none, label="  Yes, call HAL bind validation finish (full, pending)"];
///     b3 -> c3 [dir=none, label="  Infinity or exit key pressed on this remote or collision occurred.\l  Broadcast IARM validation complete event\lCall HAL bind validation finish (abort, collision)"];
///     b4 -> c4 [dir=none, label="  Broadcast IARM validation complete event\l  Call HAL validation finish (success, failed, timeout, error)"];
///
/// }
/// \enddot
///
/// ------------------------------------
/// RF4CE Controller Binding - Automatic
/// ------------------------------------
///
/// When the remote control is set to CLIP Discovery mode (setup + D), the controller will attempt to automatically bind without any user interaction required.  Several keys on the remote control
/// are designated as autobinding keys (replay, exit, mic, menu, guide, last, info, A, B, C, D).  When one of these keys are released, the controller will send the IR code for that key followed by the
/// autobind ghost code.  After a brief delay, it will issue a discovery request using a vendor specific device id.  If the controller receives more than the required number of responses from a single
/// target, it will attempt to bind with this target.
///
/// @dot
/// digraph AUTOBIND_COMMS {
///     "UNBOUND"         [shape="ellipse", fontname=Helvetica, fontsize=10, label="UNBOUND"];
///     "DISC_RSP_RXD"    [shape="diamond", fontname=Helvetica, fontsize=10, label="Exactly one discovery\nresponse received?"];
///     "FAIL_LIMIT"      [shape="diamond", fontname=Helvetica, fontsize=10, label="Failure count greater\nthan fail threshold?"];
///     "SUCCESS_LIMIT"   [shape="diamond", fontname=Helvetica, fontsize=10, label="Success count greater\nthan pair threshold?"];
///     "BOUND"       [shape="ellipse", fontname=Helvetica, fontsize=10, label="BOUND"];
///
///     node[shape=none, width=0, height=0, label=""];
///     {rank=same;  a1; a2; a3;}
///
///     "UNBOUND"          -> "DISC_RSP_RXD"  [dir="forward", fontname=Helvetica, fontsize=10,label="  Autobinding key is released.\nSend IR ghost code.\nDelay. Start discovery."];
///     "DISC_RSP_RXD"     -> "SUCCESS_LIMIT" [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, increment\nsuccess count"];
///     "SUCCESS_LIMIT"    -> "BOUND"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, complete\nbinding procedure."];
///
///     {rank=same; UNBOUND; DISC_RSP_RXD; SUCCESS_LIMIT;  BOUND;}
///     {rank=same; b1; FAIL_LIMIT; }
///     {rank=same; c1; c2; }
///
///     edge[dir=none];
///     a1 -> a2 -> a3
///     c1 -> c2
///     a3 -> SUCCESS_LIMIT [fontname=Helvetica, fontsize=10,label="  No"];
///     b1 -> FAIL_LIMIT    [fontname=Helvetica, fontsize=10,label="  No."];
///     FAIL_LIMIT -> c2    [fontname=Helvetica, fontsize=10,label="  Yes, set success and\n  failure counters to zero."];
///     c1 -> b1
///
///     edge[dir=forward];
///     DISC_RSP_RXD -> FAIL_LIMIT [fontname=Helvetica, fontsize=10,label="  No, increment\nfailure count."];
///     b1 -> UNBOUND;
///     a1 -> UNBOUND;
/// }
/// \enddot
///
/// -------------------------
/// IP Remote Control Binding
/// -------------------------
///
/// The binding procedure between the IP based controller and target (STB) is initiated on the target.  The user must navigate to the IP Remote Control
/// configuration screen to listen for incoming pairing requests.  While on this screen, ...
/// The flow of the binding procedure is shown below.
///
/// @dot
/// digraph BINDING_COMMS {
///     "UNBOUND"         [shape="ellipse", fontname=Helvetica, fontsize=10, label="UNBOUND"];
///     "IS_DISC_OK"      [shape="diamond", fontname=Helvetica, fontsize=10, label="Devices\nfound?"];
///     "IS_USER_DONE"    [shape="diamond", fontname=Helvetica, fontsize=10, label="Pair\nanother\ncontroller?"];
///     "IS_PAIR_OK"      [shape="diamond", fontname=Helvetica, fontsize=10, label="Pairing\nsuccess?"];
///     "IS_VAL_OK"       [shape="diamond", fontname=Helvetica, fontsize=10, label="Validation\nsuccess?"];
///     "BOUND"           [shape="ellipse", fontname=Helvetica, fontsize=10, label="BOUND"];
///
///     node[shape=none, width=0, height=0, label=""];
///     {rank=same;  a3; a4; a5;}
///
///     "UNBOUND"       -> "IS_DISC_OK"    [dir="forward", fontname=Helvetica, fontsize=10,label="  User navigates to IP RCU\nconfig screen. Discovery\nrequest sent by controller."];
///     "IS_DISC_OK"    -> "IS_USER_DONE"  [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, user is presented\nwith controller list."];
///     "IS_USER_DONE"  -> "IS_PAIR_OK"    [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, perform\npairing procedure\non selected controller."];
///     "IS_PAIR_OK"    -> "IS_VAL_OK"     [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes, perform\nvalidation procedure."];
///     "IS_VAL_OK"     -> "BOUND"         [dir="forward", fontname=Helvetica, fontsize=10,label="  Yes"];
///
///     {rank=same; UNBOUND; IS_DISC_OK; IS_USER_DONE; IS_PAIR_OK; IS_VAL_OK; BOUND;}
///     {rank=same; b1; b2; b3; b4; b5;}
///
///     edge[dir=none];
///     a3 -> a4 -> a5
//     b1 -> b2 -> b3
///     a4 -> "IS_PAIR_OK"    [fontname=Helvetica, fontsize=10,label="  No"];
///     a5 -> "IS_VAL_OK"     [fontname=Helvetica, fontsize=10,label="  No, unpair\ncontroller."];
///
///     edge[dir=forward];
///     "IS_DISC_OK" -> b2    [fontname=Helvetica, fontsize=10,label="  No, user exited."];
///     "IS_USER_DONE" -> b3  [fontname=Helvetica, fontsize=10,label="  No, user exited."];
///     a3 -> "IS_USER_DONE";
/// }
/// \enddot
///
/// -------------------------
/// Controller Key Press Flow
/// -------------------------
///
/// Controller key press events are generated when the user presses, holds and releases a key on a bound controller.
///
///
/// @dot
/// digraph CTRL_StateMachine {
///     "RCU"   [shape="ellipse", fontname=Helvetica, fontsize=10, label="Remote Control"];
///     "CTRLD" [shape="ellipse", fontname=Helvetica, fontsize=10, label="Control Driver"];
///     "CTRLM" [shape="ellipse", fontname=Helvetica, fontsize=10, label="Control Manager"];
///     "IRM"   [shape="ellipse", fontname=Helvetica, fontsize=10, label="IR Manager"];
///     "SVCM"  [shape="ellipse", fontname=Helvetica, fontsize=10, label="Service Manager"];
///
///     "RCU"   -> "CTRLD" [fontname=Helvetica, fontsize=10,label="  User presses, holds or\n releases a key"];
///     "CTRLD" -> "CTRLM" [fontname=Helvetica, fontsize=10,label="  Key received in driver"];
///     "CTRLM" -> "IRM"   [fontname=Helvetica, fontsize=10,label="  Key Broadcast  "];
///     "CTRLM" -> "SVCM"  [fontname=Helvetica, fontsize=10,label=""];
/// }
/// \enddot
/// @}
/// @}
#endif
