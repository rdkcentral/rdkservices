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

#define CTRLM_MAIN_IARM_BUS_NAME                                 "Ctrlm"                                ///< Control Manager's IARM Bus Name
#define CTRLM_MAIN_IARM_BUS_API_REVISION                         (16)                                   ///< Revision of the Control Manager Main IARM API

#define CTRLM_MAIN_IARM_CALL_STATUS_GET                          "Main_StatusGet"                       ///< Retrieves Control Manager's Status information
#define CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET                  "Main_NetworkStatusGet"                ///< Retrieves the network's Status information
#define CTRLM_MAIN_IARM_CALL_IR_REMOTE_USAGE_GET                 "Main_IrRemoteUsageGet"                ///< Retrieves the ir remote usage info
#define CTRLM_MAIN_IARM_CALL_LAST_KEY_INFO_GET                   "Main_LastKeyInfoGet"                  ///< Retrieves the last key info
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_SET_VALUES          "Main_ControlService_SetValues"        ///< IARM Call to set control service values
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_GET_VALUES          "Main_ControlService_GetValues"        ///< IARM Call to get control service values
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_CAN_FIND_MY_REMOTE  "Main_ControlService_CanFindMyRemote"  ///< IARM Call to get control service find my remote
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_START_PAIRING_MODE  "Main_ControlService_StartPairingMode" ///< IARM Call to set control service start pairing mode
#define CTRLM_MAIN_IARM_CALL_CONTROL_SERVICE_END_PAIRING_MODE    "Main_ControlService_EndPairingMode"   ///< IARM Call to set control service end pairing mode
#define CTRLM_MAIN_IARM_CALL_PAIRING_METRICS_GET                 "Main_PairingMetricsGet"               ///< Retrieves the stb's pairing metrics
#define CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET                     "Main_ChipStatusGet"                   ///< get Chip status

#define CTRLM_MAIN_NETWORK_ID_INVALID                          (0xFF) ///< An invalid network identifier
#define CTRLM_MAIN_CONTROLLER_ID_INVALID                       (0xFF) ///< An invalid controller identifier

#define CTRLM_MAIN_CONTROLLER_ID_ALL                           (0xFE) ///< Indicates that the command applies to all networks

#define CTRLM_MAIN_CONTROLLER_ID_LAST_USED                     (0xFD) ///< An last used controller identifier

#define CTRLM_MAIN_VERSION_LENGTH                                (20) ///< Maximum length of the version string
#define CTRLM_MAIN_MAX_NETWORKS                                   (4) ///< Maximum number of networks
#define CTRLM_MAIN_MAX_BOUND_CONTROLLERS                          (9) ///< Maximum number of bound controllers
#define CTRLM_MAIN_MAX_CHIPSET_LENGTH                            (16) ///< Maximum length of chipset name string (including null termination)
#define CTRLM_MAIN_COMMIT_ID_MAX_LENGTH                          (48) ///< Maximum length of commit ID string (including null termination)
#define CTRLM_MAIN_RECEIVER_ID_MAX_LENGTH                        (40) ///< Maximum length of receiver ID string (including null termination)
#define CTRLM_MAIN_DEVICE_ID_MAX_LENGTH                          (24) ///< Maximum length of device ID string (including null termination)
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

#define CTRLM_MAX_PARAM_STR_LEN           (64)

typedef enum {
   CTRLM_IARM_CALL_RESULT_SUCCESS                 = 0, ///< The requested operation was completed successfully.
   CTRLM_IARM_CALL_RESULT_ERROR                   = 1, ///< An error occurred during the requested operation.
   CTRLM_IARM_CALL_RESULT_ERROR_READ_ONLY         = 2, ///< An error occurred trying to write to a read-only entity.
   CTRLM_IARM_CALL_RESULT_ERROR_INVALID_PARAMETER = 3, ///< An input parameter is invalid.
   CTRLM_IARM_CALL_RESULT_ERROR_API_REVISION      = 4, ///< The API revision is invalid or no longer supported
   CTRLM_IARM_CALL_RESULT_ERROR_NOT_SUPPORTED     = 5, ///< The requested operation is not supported
   CTRLM_IARM_CALL_RESULT_INVALID                 = 6, ///< Invalid call result value
} ctrlm_iarm_call_result_t;

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

/// @brief Network Status Structure
/// @details The Network Status structure is used in the CTRLM_MAIN_IARM_CALL_NETWORK_STATUS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision; ///< Revision of this API
   ctrlm_iarm_call_result_t result;       ///< OUT - Result of the operation
   ctrlm_network_id_t       network_id;   ///< IN - identifier of network
   union {
      ctrlm_network_status_rf4ce_t rf4ce; ///< OUT - RF4CE network status
   } status;                              ///< OUT - Union of network status types
} ctrlm_main_iarm_call_network_status_t;

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

/// @brief Chip Status Structure
/// @details The Chip Status structure is used in the CTRLM_MAIN_IARM_CALL_CHIP_STATUS_GET call. See the @link CTRLM_IPC_MAIN_CALLS Calls@endlink section for more details on invoking this call.
typedef struct {
   unsigned char            api_revision;   ///< Revision of this API
   ctrlm_iarm_call_result_t result;         ///< OUT - Result of the operation
   ctrlm_network_id_t       network_id;     ///< IN - identifier of network
   unsigned char            chip_connected; ///< OUT - 1 - chip connected, 0 - chip disconnected
} ctrlm_main_iarm_call_chip_status_t;

#define CTRLM_VOICE_IARM_CALL_STATUS               "Voice_Status"             ///< IARM Call to get status
#define CTRLM_VOICE_IARM_CALL_CONFIGURE_VOICE      "Voice_ConfigureVoice"     ///< IARM Call to set up voice with JSON payload
#define CTRLM_VOICE_IARM_CALL_SET_VOICE_INIT       "Voice_SetVoiceInit"       ///< IARM Call to set application data with JSON payload in the voice server init message
#define CTRLM_VOICE_IARM_CALL_SEND_VOICE_MESSAGE   "Voice_SendVoiceMessage"   ///< IARM Call to send JSON payload to voice server
#define CTRLM_VOICE_IARM_CALL_SESSION_TYPES        "Voice_SessionTypes"       ///< IARM Call to get voice session request types
#define CTRLM_VOICE_IARM_CALL_SESSION_REQUEST      "Voice_SessionRequest"     ///< IARM Call to request a voice session
#define CTRLM_VOICE_IARM_CALL_SESSION_TERMINATE    "Voice_SessionTerminate"   ///< IARM Call to terminate a voice session
#define CTRLM_VOICE_IARM_CALL_SESSION_AUDIO_STREAM_START "Voice_SessionAudioStreamStart"   ///< IARM Call to start an audio stream in a voice session

#define CTRLM_VOICE_IARM_CALL_RESULT_LEN_MAX       (2048) ///< IARM Call result length

#define CTRLM_VOICE_IARM_BUS_API_REVISION             (9) ///< Revision of the Voice IARM API
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

#define CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS            "Rcu_ControllerStatus"     ///< IARM Call to get controller information
#define CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET              "Rcu_RibRequestGet"        ///< IARM Call to retrieves an attribute from the controller's RIB
#define CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET              "Rcu_RibRequestSet"        ///< IARM Call to set an attribute in the controller's RIB
#define CTRLM_RCU_IARM_CALL_REVERSE_CMD                  "Rcu_ReverseCmd"           ///< IARM Call to Trigger Remote Controller Action

#define CTRLM_RCU_IARM_BUS_API_REVISION                  (13)    ///< Revision of the RCU IARM API
#define CTRLM_RCU_VALIDATION_KEY_QTY                      (3)    ///< Number of validation keys used in internal validation
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
#define CTRLM_RCU_RIB_ATTR_LEN_CONTROLLER_IRDB_STATUS    (15)    ///< RIB Attribute Length - Controller IRDB Status
#define CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS        (13)    ///< RIB Attribute Length - Target IRDB Status

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

typedef enum {
   CONTROLLER_REBOOT_POWER_ON      = 0,
   CONTROLLER_REBOOT_EXTERNAL      = 1,
   CONTROLLER_REBOOT_WATCHDOG      = 2,
   CONTROLLER_REBOOT_CLOCK_LOSS    = 3,
   CONTROLLER_REBOOT_BROWN_OUT     = 4,
   CONTROLLER_REBOOT_OTHER         = 5,
   CONTROLLER_REBOOT_ASSERT_NUMBER = 6
} controller_reboot_reason_t;

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
#endif
