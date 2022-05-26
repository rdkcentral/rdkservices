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

#include "RamHelper.h"
#include "UtilsLogging.h"
#include "UtilsUnused.h"

// IR-RF Database RF descriptors, needed for all original configurable keys
// Discrete Power ON/OFF use actual RF keycodes (0x6D, 0x6C), the rest are all XRC ghost codes
unsigned char const rfDescriptor_DiscretePwrOn[]    = { 0x01, 0x4C, 0x02, 0x01, 0x6D };
unsigned char const rfDescriptor_DiscretePwrOff[]   = { 0x01, 0x4C, 0x02, 0x01, 0x6C };

unsigned char const rfDescriptor_IRPowerToggle[]    = { 0x21, 0x4C, 0x02, 0x31, 0x03 };
unsigned char const rfDescriptor_VolumeUp[]         = { 0x01, 0x4C, 0x02, 0x31, 0x06 };
unsigned char const rfDescriptor_VolumeDown[]       = { 0x01, 0x4C, 0x02, 0x31, 0x07 };
unsigned char const rfDescriptor_Mute[]             = { 0x01, 0x4C, 0x02, 0x31, 0x08 };
unsigned char const rfDescriptor_Input[]            = { 0x01, 0x4C, 0x02, 0x31, 0x09 };

// The following symbol enables code that swaps the power-related Device Type field values,
// in the unusual case where an AVR has the toggle power slot, and a TV has the discrete slots.
// This is done in order to get the XR11 All Power button to work properly, in this special case.
#define SPECIAL_XR11_POWER_DEVICETYPE_SWAP_ENABLE       1

#if CONTROLMGR_MAX_IR_DATA_SIZE > (CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE - 8)    // Eight is flag(1)+RFghost(5)+IRheader(2)
#error "ControlMgr RIB request data size is too small for max IR codes!!"
#endif

using namespace std;

namespace WPEFramework {

    namespace Plugin {

        //
        // IARM-level RemoteActionMappingHelper Methods
        //

        ctrlm_network_id_t RemoteActionMappingHelper::getRf4ceNetworkID()
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
                    // Apparent success, search for the RF4CE network ID.
                    for (int i = 0; i < status.network_qty; i++)
                    {
                        if (status.networks[i].type == CTRLM_NETWORK_TYPE_RF4CE)
                        {
                            rf4ceId = status.networks[i].id;
                            break;
                        }
                    }
                }
            }

            return rf4ceId;
        }

        bool RemoteActionMappingHelper::getRf4ceBindRemotes(rf4ceBindRemotes_t* bindRemotes)
        {
            ctrlm_main_iarm_call_network_status_t   netStatus;
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatus;
            ctrlm_network_id_t                      rf4ceId;
            IARM_Result_t                           res;
            bool                                    retVal = false;
            bool                                    found = false;
            int                                     ctrlCount = 0;

            if (bindRemotes == NULL)
            {
                LOGERR("LOGIC ERROR - no bindRemotes output array passed in!!");
                return false;
            }
            memset((void*)bindRemotes, 0, sizeof(rf4ceBindRemotes_t));

            // Get the status of all the paired remotes on the rf4ce network.
            // Start by finding the network_id of the rf4ce network on this STB.
            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId != CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGWARN("found rf4ce network_id: %d.", (int)rf4ceId);
            }
            else
            {
                LOGWARN("WARNING - No RF4CE network_id found!");
                return false;
            }

            // Next, get the array of controller IDs for the rf4ce network.
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
            LOGWARN("status_rf4ce - version_hal: %s, controller_qty: %d, pan_id: 0x%04X, rf_channel number: 0x%02X, rf_channel quality: 0x%02X.\n",
                    netStatus.status.rf4ce.version_hal, netStatus.status.rf4ce.controller_qty, netStatus.status.rf4ce.pan_id,
                    netStatus.status.rf4ce.rf_channel_active.number, netStatus.status.rf4ce.rf_channel_active.quality);

            if (netStatus.status.rf4ce.controller_qty == 0)
            {
                LOGWARN("WARNING - No RF4CE controllers found!");
                return false;
            }

            // There are one or more controllers paired on the rf4ce network.
            // Get the status for each one, and put them in the bindRemotes array, in last-used order.
            retVal = false;
            for (int i = 0; i < netStatus.status.rf4ce.controller_qty; i++)
            {
                memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
                ctrlStatus.api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
                ctrlStatus.network_id = rf4ceId;
                ctrlStatus.controller_id = netStatus.status.rf4ce.controllers[i];
                res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS, (void*)&ctrlStatus, sizeof(ctrlStatus));
                if (res != IARM_RESULT_SUCCESS)
                {
                    LOGERR("ERROR - CONTROLLER_STATUS IARM_Bus_Call FAILED, res: %d, controller_id: %d, index: %d",
                           (int)res, (int)netStatus.status.rf4ce.controllers[i], i);
                }
                else
                {
                    if (ctrlStatus.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                    {
                        LOGERR("ERROR - CONTROLLER_STATUS FAILED, call_result: %d, controller_id: %d, index: %d",
                               (int)ctrlStatus.result, (int)netStatus.status.rf4ce.controllers[i], i);
                    }
                    else
                    {
                        int insIdx = 0;

                        LOGWARN("controller_status[%d] - controller_id: %d, type: %s, SW version: %s, HW version: %s, time_last_key: %lu.",
                                i, netStatus.status.rf4ce.controllers[i], ctrlStatus.status.type,
                                ctrlStatus.status.version_software, ctrlStatus.status.version_hardware, ctrlStatus.status.time_last_key);

                        if (ctrlCount > CTRLM_MAIN_MAX_BOUND_CONTROLLERS)
                        {
                            LOGERR("LOGIC ERROR - Limit of MAX_BOUND_CONTROLLERS exceeded, controller_id: %d, index: %d!!!",
                                   (int)netStatus.status.rf4ce.controllers[i], i);
                            break;
                        }
                        found = false;
                        // Insert this controller_status in the output, in most-recently-used order.
                        for (insIdx = 0; insIdx < CTRLM_MAIN_MAX_BOUND_CONTROLLERS; insIdx++)
                        {
                            // Search for the insertion point
                            if (bindRemotes->remotes[insIdx].status.time_last_key < ctrlStatus.status.time_last_key)
                            {
                                // Make room for the insertion, if needed
                                // ctrlCount can also be considered the "first-empty-slot-index".
                                if (bindRemotes->remotes[insIdx].status.time_last_key != 0)
                                {
                                    int slots2move = ctrlCount - insIdx;
                                    controller_info* src = &(bindRemotes->remotes[insIdx]);
                                    controller_info* dest = src + 1;
                                    memmove((void*)dest, (void*)src, (slots2move * sizeof(controller_info)));
                                }
                                // Insert the controller_status
                                bindRemotes->remotes[insIdx].network_id = rf4ceId;
                                bindRemotes->remotes[insIdx].controller_id = netStatus.status.rf4ce.controllers[i];
                                bindRemotes->remotes[insIdx].status = ctrlStatus.status;
                                found = true;
                                break;
                            }
                        }
                        if (found)
                        {
                            ctrlCount++;
                            bindRemotes->numBindRemotes = ctrlCount;
                            retVal = true;
                            LOGWARN("controller_status[%d] - controller_id: %d, has been inserted at index %d.",
                                    i, netStatus.status.rf4ce.controllers[i], insIdx);
                        }
                        else
                        {
                            LOGERR("LOGIC ERROR - No slot found to insert controller_status[%d] - controller_id: %d!!!",
                                   i, netStatus.status.rf4ce.controllers[i]);
                        }
                    }
                }
            }

            return retVal;
        }

        int RemoteActionMappingHelper::getLastUsedDeviceID(std::string& remoteType, bool& bFiveDigitCodeSet, bool& bFiveDigitCodeSupported)
        {
            rf4ceBindRemotes_t bindRemotes;
            int deviceID = -1;

            if (getRf4ceBindRemotes(&bindRemotes))
            {
                LOGWARN("controlMgr: %d RF4CE remotes found.", bindRemotes.numBindRemotes);

                for (int i = 0; i < bindRemotes.numBindRemotes; i++)
                {
                    LOGWARN("controller_id: %d, type: %s, firmware: %s, hardware: %s, battery: %.2f volts, time_last_key: %lu.",
                            bindRemotes.remotes[i].controller_id, bindRemotes.remotes[i].status.type,
                            bindRemotes.remotes[i].status.version_software, bindRemotes.remotes[i].status.version_hardware,
                            bindRemotes.remotes[i].status.battery_voltage_loaded, bindRemotes.remotes[i].status.time_last_key);
                }

                deviceID = bindRemotes.remotes[0].controller_id;
                remoteType = std::string(bindRemotes.remotes[0].status.type);

                // Set the booleans concerning 5-digit codes.
                bFiveDigitCodeSet = (bindRemotes.remotes[0].status.ir_db_state == CTRLM_RCU_IR_DB_STATE_TV_CODE) ||
                                    (bindRemotes.remotes[0].status.ir_db_state == CTRLM_RCU_IR_DB_STATE_AVR_CODE) ||
                                    (bindRemotes.remotes[0].status.ir_db_state == CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES);

        #if (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)
                bFiveDigitCodeSupported = (bindRemotes.remotes[0].status.ir_db_code_download_supported > 0);
        #else
                bFiveDigitCodeSupported = false;
        #endif //   (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)
            }
            else
            {
                LOGWARN("NO valid RF4CE remotes found!");
            }

            return deviceID;
        }

        bool RemoteActionMappingHelper::getControllerByID(int deviceID, std::string& remoteType, bool& bFiveDigitCodeSet, bool& bFiveDigitCodeSupported)
        {
            ctrlm_rcu_iarm_call_controller_status_t ctrlStatus;
            ctrlm_network_id_t                      rf4ceId;
            IARM_Result_t                           res;

            // Find the network_id of the rf4ce network on this STB.
            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId != CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGWARN("found rf4ce network_id: %d.", (int)rf4ceId);
            }
            else
            {
                LOGWARN("WARNING - No RF4CE network_id found!");
                return false;
            }

            // Get the controller status
            memset((void*)&ctrlStatus, 0, sizeof(ctrlStatus));
            ctrlStatus.api_revision = CTRLM_RCU_IARM_BUS_API_REVISION;
            ctrlStatus.network_id = rf4ceId;
            ctrlStatus.controller_id = deviceID;
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_CONTROLLER_STATUS, (void*)&ctrlStatus, sizeof(ctrlStatus));
            if (res != IARM_RESULT_SUCCESS)
            {
                LOGERR("CONTROLLER_STATUS IARM_Bus_Call FAILED, res: %d, controller_id: %d, network_id: %d.",
                       (int)res, deviceID, rf4ceId);
                return false;
            }
            else
            {
                if (ctrlStatus.result != CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGERR("CONTROLLER_STATUS FAILED, call_result: %d, controller_id: %d, network_id: %d.",
                           (int)ctrlStatus.result, deviceID, rf4ceId);
                    return false;
                }
                else
                {
                    LOGWARN("controller_id: %d, type: %s, firmware: %s, hardware: %s, ir_db_state: %d, ir_db_code_download_supported: %s.",
                            ctrlStatus.controller_id, ctrlStatus.status.type,
                            ctrlStatus.status.version_software, ctrlStatus.status.version_hardware,
        #if (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)
                                ctrlStatus.status.ir_db_state, (ctrlStatus.status.ir_db_code_download_supported ? "TRUE" : "FALSE"));
        #else
                                ctrlStatus.status.ir_db_state, "FALSE"));
        #endif //   (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)

                    remoteType = std::string(ctrlStatus.status.type);

                    // Set the booleans concerning 5-digit codes.
                    bFiveDigitCodeSet = (ctrlStatus.status.ir_db_state == CTRLM_RCU_IR_DB_STATE_TV_CODE) ||
                                        (ctrlStatus.status.ir_db_state == CTRLM_RCU_IR_DB_STATE_AVR_CODE) ||
                                        (ctrlStatus.status.ir_db_state == CTRLM_RCU_IR_DB_STATE_TV_AVR_CODES);

        #if (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)
                    bFiveDigitCodeSupported = (ctrlStatus.status.ir_db_code_download_supported > 0);
        #else
                    bFiveDigitCodeSupported = false;
        #endif //   (CTRLM_RCU_IARM_BUS_API_REVISION >= 4)
                }
            }

            return true;
        }

        // All actionMap members must be set properly
        bool RemoteActionMappingHelper::setKeyActionMap(int deviceID, int keymapType,
                                                        keyActionMap& actionMap,
                                                        const KeyGroupSrcInfo& srcInfo)
        {
            UNUSED(keymapType);
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            int             deviceType = -1;    // 0 is TV, 1 is AVR, -1 is none.
            int             dataSize = 0;
            unsigned char*  data = NULL;
            unsigned char   flags = MSO_RIB_IRRFDB_PERMANENT_BIT | MSO_RIB_IRRFDB_IRSPECIFIED_BIT;
            unsigned char   irConfig = 0;
            unsigned char*  bytePtr = (unsigned char*)&(ribRequest.data[0]);
            unsigned char*  rfDesc = NULL;
            size_t          rfDescLength = 0;
            size_t          total = 0;

            if ((deviceID < 1) || (actionMap.keyName < 0x30) ||
                (actionMap.rfKeyCode < 0) || (actionMap.rfKeyCode > 255) ||
                ((actionMap.tvIRData.size() <= 1) && (actionMap.avrIRData.size() <= 1)))
            {
                LOGERR("ERROR - Bad arguments! Cannot set map!");
                return false;
            }

            if (actionMap.keyName == KED_UNDEFINEDKEY)
            {
                LOGWARN("WARNING - actionMap for rfKeyCode 0x%02X is an alternate!", actionMap.rfKeyCode);
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found!  Cannot set map!");
                return false;
            }

            // Use the srcInfo to select the correct IR data source, for the current RF key.
            switch (actionMap.rfKeyCode)
            {
                // INPUT is a group all by itself
                case MSO_RFKEY_INPUT_SELECT:
                    if (srcInfo.groupInputSelect == KEY_GROUP_SRC_TV)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV IR code for INPUT_SELECT.");
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupInputSelect == KEY_GROUP_SRC_AVR)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR IR code for INPUT_SELECT.");
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else
                    {
                        LOGWARN("WARNING - No INPUT_SELECT, cannot set IRRFDB slot, src: %d!", srcInfo.groupInputSelect);
                    }
                    break;
                // POWER_TOGGLE is a group all by itself
                case MSO_RFKEY_PWR_TOGGLE:
                    if (srcInfo.groupTogglePower == KEY_GROUP_SRC_TV)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV IR code for POWER_TOGGLE.");
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupTogglePower == KEY_GROUP_SRC_AVR)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR IR code for POWER_TOGGLE.");
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else
                    {
                        LOGWARN("WARNING - No POWER_TOGGLE, cannot set IRRFDB slot, src: %d!", srcInfo.groupTogglePower);
                    }
                    break;
                case MSO_RFKEY_PWR_OFF:
                case MSO_RFKEY_PWR_ON:
                {
                    const char *name = (actionMap.rfKeyCode == MSO_RFKEY_PWR_ON) ? "DISCRETE_PWR_ON" : "DISCRETE_PWR_OFF";
                    if (srcInfo.groupDiscretePower == KEY_GROUP_SRC_TV)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV IR code for %s.", name);
                        irConfig = 0x4F;    // Tweak database, min transmissions = 0xF
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupDiscretePower == KEY_GROUP_SRC_AVR)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR IR code for %s.", name);
                        irConfig = 0x4F;    // Tweak database, min transmissions = 0xF
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else if (srcInfo.groupDiscretePower == KEY_GROUP_SRC_TV_PWR_CROSS)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV toggle IR code into  %s.", name);
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupDiscretePower == KEY_GROUP_SRC_AVR_PWR_CROSS)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR toggle IR code into %s.", name);
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else
                    {
                        LOGWARN("WARNING - No %s, cannot set IRRFDB slot, src: %d!", name, srcInfo.groupDiscretePower);
                    }
                    break;
                }
                case MSO_RFKEY_VOL_PLUS:
                case MSO_RFKEY_VOL_MINUS:
                {
                    const char *name = (actionMap.rfKeyCode == MSO_RFKEY_VOL_PLUS) ? "VOLUME_UP" : "VOLUME_DOWN";
                    if (srcInfo.groupVolume == KEY_GROUP_SRC_TV)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV IR code for %s.", name);
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupVolume == KEY_GROUP_SRC_AVR)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR IR code for %s.", name);
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else
                    {
                        LOGWARN("WARNING - No %s, cannot set IRRFDB slot, src: %d!", name, srcInfo.groupVolume);
                    }
                    break;
                }
                case MSO_RFKEY_MUTE:
                {
                    if (srcInfo.groupMute == KEY_GROUP_SRC_TV)
                    {
                        // Use the TV blob
                        LOGWARN("Setting TV IR code for MUTE.");
                        data = actionMap.tvIRData.data();
                        dataSize = actionMap.tvIRData.size();
                        deviceType = 0;
                    }
                    else if (srcInfo.groupMute == KEY_GROUP_SRC_AVR)
                    {
                        // Use the AVR blob
                        LOGWARN("Setting AVR IR code for MUTE.");
                        data = actionMap.avrIRData.data();
                        dataSize = actionMap.avrIRData.size();
                        deviceType = 1;
                    }
                    else
                    {
                        LOGWARN("WARNING - No MUTE, cannot set IRRFDB slot, src: %d!", srcInfo.groupMute);
                    }
                    break;
                }
                default:
                    LOGERR("LOGIC ERROR - RF Key 0x%02X is outside the map!", (unsigned)actionMap.rfKeyCode);
                    return false;
            }

            if (dataSize > CONTROLMGR_MAX_IR_DATA_SIZE)
            {
                LOGERR("LOGIC ERROR - IRCode dataSize %d, for RF Key 0x%02X, exceeds size limits!",
                       dataSize, (unsigned)actionMap.rfKeyCode);
                return false;
            }

            if ((deviceType < 0) || (dataSize == 0) || (data == NULL))
            {
                // Neither TV nor AVR IR code is available for this RF key.  This indicates missing actionMap data, a fatal error!
                LOGERR("LOGIC ERROR - No TV or AVR IR code available for RF Key: 0x%02X!!\n", (unsigned)actionMap.rfKeyCode);
                return false;
            }

        #ifdef SPECIAL_XR11_POWER_DEVICETYPE_SWAP_ENABLE
            if ((srcInfo.groupTogglePower == KEY_GROUP_SRC_AVR) &&
                (srcInfo.groupDiscretePower == KEY_GROUP_SRC_TV) &&
                ((actionMap.rfKeyCode == MSO_RFKEY_PWR_TOGGLE) ||
                 (actionMap.rfKeyCode == MSO_RFKEY_PWR_OFF) ||
                 (actionMap.rfKeyCode == MSO_RFKEY_PWR_ON)))
            {
                // Switch from TV to AVR, or vice-versa.
                if (deviceType == 1)
                {
                    deviceType = 0;
                }
                else if (deviceType == 0)
                {
                    deviceType = 1;
                }
            }
        #endif  // SPECIAL_XR11_POWER_DEVICETYPE_SWAP_ENABLE

            if (deviceType == 1)
            {
                flags |= MSO_RIB_IRRFDB_DEVICETYPE_AVR;
            }

            // Construct the direct RIB entry for this RF key
            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
            ribRequest.attribute_index  = (unsigned char)actionMap.rfKeyCode;

            // Decide what RF Descriptor we need to add for this RF key.
            switch (actionMap.rfKeyCode)
            {
                case MSO_RFKEY_PWR_TOGGLE:
                    rfDesc = (unsigned char*)rfDescriptor_IRPowerToggle;
                    rfDescLength = sizeof(rfDescriptor_IRPowerToggle);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("IR Power Toggle RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_PWR_OFF:
                    rfDesc = (unsigned char*)rfDescriptor_DiscretePwrOff;
                    rfDescLength = sizeof(rfDescriptor_DiscretePwrOff);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Discrete Power Off RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_PWR_ON:
                    rfDesc = (unsigned char*)rfDescriptor_DiscretePwrOn;
                    rfDescLength = sizeof(rfDescriptor_DiscretePwrOn);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Discrete Power On RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_VOL_PLUS:
                    rfDesc = (unsigned char*)rfDescriptor_VolumeUp;
                    rfDescLength = sizeof(rfDescriptor_VolumeUp);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Volume Up RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_VOL_MINUS:
                    rfDesc = (unsigned char*)rfDescriptor_VolumeDown;
                    rfDescLength = sizeof(rfDescriptor_VolumeDown);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Volume Down RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_MUTE:
                    rfDesc = (unsigned char*)rfDescriptor_Mute;
                    rfDescLength = sizeof(rfDescriptor_Mute);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Mute RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case MSO_RFKEY_INPUT_SELECT:
                    rfDesc = (unsigned char*)rfDescriptor_Input;
                    rfDescLength = sizeof(rfDescriptor_Input);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Input RF Descriptor included, size: %d.", rfDescLength);
                    break;
                default:
                    LOGWARN("No RF Descriptor included for RF Key 0x%02X.", (unsigned)actionMap.rfKeyCode);
                    break;
            }

            // We now know what the write size will be.
            total = 1 + rfDescLength + 2 + dataSize;
            if (total > CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE)
            {
                LOGERR("LOGIC ERROR - IR-RF DB entry length is %u, for RF Key 0x%02X, exceeds size limits!!",
                       total, (unsigned)actionMap.rfKeyCode);
                return false;
            }
            ribRequest.length = (unsigned char)total;

            // Copy all the data into the ribRequest structure
            *bytePtr = flags;
            bytePtr++;
            if (rfDesc != NULL)
            {
                memcpy(bytePtr, rfDesc, rfDescLength);
                bytePtr += rfDescLength;
            }
            *bytePtr = irConfig;
            bytePtr++;
            *bytePtr = (unsigned char)dataSize;
            bytePtr++;
            memcpy(bytePtr, data, dataSize);

            LOGWARN("SET ribRequest data - total: %d, dataSize: %d, data: 0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X - "
                    "0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X.\n",
                    total, dataSize,
                    (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1], (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3],
                    (unsigned char)ribRequest.data[4], (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                    (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10], (unsigned char)ribRequest.data[11],
                    (unsigned char)ribRequest.data[12], (unsigned char)ribRequest.data[13], (unsigned char)ribRequest.data[14], (unsigned char)ribRequest.data[15],
                    (unsigned char)ribRequest.data[16], (unsigned char)ribRequest.data[17], (unsigned char)ribRequest.data[18], (unsigned char)ribRequest.data[19]);

            // Do the direct write to the IR-RF DB RIB entry.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Set RIB IR-RF DB Request: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGWARN("%s: map set for keyName: 0x%02X, rfKeyCode: 0x%02X, %s IrCode size: %d.\n", __FUNCTION__,
                               actionMap.keyName, actionMap.rfKeyCode, ((deviceType == 1) ? "AVR" : "TV"), dataSize);
                }
                else
                {
                    LOGERR("FAILURE result in SET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of setKeyActionMap()

        // actionMap.keyName is a required in-parameter, all the rest are out-parameters
        bool RemoteActionMappingHelper::getKeyActionMap(int deviceID, int keymapType, keyActionMap& actionMap)
        {
            UNUSED(keymapType);
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            unsigned char   flags = 0;
            unsigned char*  pData = NULL;
            unsigned char*  pMax = NULL;
            int             dataSize = 0;
            int             rfKey = -1;
            int             deviceType = -1;    // 0 == TV, 1 == AVR, -1 == no mapping

            if ((deviceID < 1) || (actionMap.keyName < 0x30))
            {
                LOGERR("Bad arguments! Cannot get map!");
                return false;
            }

            rfKey = lookupRFKey(actionMap.keyName);
            if (rfKey < 0)
            {
                LOGERR("keyName 0x%02X does not map to a RF key! Cannot get map!", (unsigned)actionMap.keyName);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Cannot get map!");
                return false;
            }

            // All we can do for now is get info from the RIB - getting info from the actual remote is not possible
            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
            ribRequest.attribute_index  = (unsigned char)rfKey;
            ribRequest.length           = CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE;

            // Read the RIB IRRFDB entry for the specified rfKey
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Get RIB IR-RF DB Request: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if ((ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS) && (ribRequest.length > 0))
                {
                    flags = (unsigned char)ribRequest.data[0];
                    LOGWARN("RIB data: 0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X - "
                            "0x%02X, 0x%02X, 0x%02X, 0x%02X - 0x%02X, 0x%02X, 0x%02X, 0x%02X.\n",
                            (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1], (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3],
                            (unsigned char)ribRequest.data[4], (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                            (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10], (unsigned char)ribRequest.data[11],
                            (unsigned char)ribRequest.data[12], (unsigned char)ribRequest.data[13], (unsigned char)ribRequest.data[14], (unsigned char)ribRequest.data[15],
                            (unsigned char)ribRequest.data[16], (unsigned char)ribRequest.data[17], (unsigned char)ribRequest.data[18], (unsigned char)ribRequest.data[19]);
                    // Parse the flags byte, and find the IR code data, if present
                    if (flags & MSO_RIB_IRRFDB_DEFAULT_BIT)
                    {
                        // No IRCode mapping, regardless of other flag bits
                        deviceType = -1;
                    }
                    else if (flags & MSO_RIB_IRRFDB_IRSPECIFIED_BIT)
                    {
                        // Get the deviceType first.
                        deviceType = (flags & MSO_RIB_IRRFDB_DEVICETYPE_AVR) ? 1 : 0;
                        // There is an IR Descriptor, and we need to find it
                        pData = (unsigned char*)&(ribRequest.data[1]);
                        pMax = pData + ribRequest.length - 2;  // pMax points at the last valid byte, according to ribRequest.length
                        if (flags & MSO_RIB_IRRFDB_RFPRESSED_BIT)
                        {
                            // Advance past the pressed descriptor
                            pData += pData[2] + 3;
                            if (pData > pMax) flags = 0;    // Indicate failure by zeroing the flags
                        }
                        if (flags & MSO_RIB_IRRFDB_RFREPEATED_BIT)
                        {
                            // Advance past the repeated descriptor
                            pData += pData[2] + 3;
                            if (pData > pMax) flags = 0;
                        }
                        if (flags & MSO_RIB_IRRFDB_RFRELEASED_BIT)
                        {
                            // Advance past the released descriptor
                            pData += pData[2] + 3;
                            if (pData > pMax) flags = 0;
                        }
                        if (flags != 0)
                        {
                            // pData should be pointing to the IR descriptor now
                            pData++;
                            if (pData < pMax)
                            {
                                // Get the IRCode length
                                dataSize = (int)*pData;
                                if ((pData + dataSize) > pMax)
                                {
                                    // ERROR - parsed IRCode length exceeds ribRequest.length
                                    LOGERR("FAILURE in IR Descriptor parse - IR Code Length: %d.\n", dataSize);
                                    return false;
                                }
                                else
                                {
                                    // Leave pData pointing past the length, to the first IR data byte.
                                    pData++;
                                }
                            }
                            else
                            {
                                LOGERR("FAILURE in IR Descriptor!\n");
                                return false;
                            }
                        }
                        else
                        {
                            LOGERR("FAILURE in RF Descriptors!\n");
                            return false;
                        }
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d, length: %d.\n",
                           ribRequest.result, ribRequest.length);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            LOGWARN("IR-RF DB RIB entry - deviceType: %d, dataSize: %d.\n", deviceType, dataSize);

            actionMap.rfKeyCode = rfKey;
            actionMap.tvIRData.clear();
            actionMap.avrIRData.clear();
            if (deviceType == 0)
            {
                actionMap.tvIRData.assign(pData, pData + dataSize);
            }
            else if (deviceType == 1)
            {
                actionMap.avrIRData.assign(pData, pData + dataSize);
            }

            return true;
        }

        bool RemoteActionMappingHelper::clearKeyActionMap(int deviceID, int keymapType, int keyName)
        {
            UNUSED(keymapType);
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            int             rfKey = lookupRFKey(keyName);
            unsigned char   flags = MSO_RIB_IRRFDB_PERMANENT_BIT | MSO_RIB_IRRFDB_DEFAULT_BIT;

            if ((deviceID < 1) || (rfKey <= 0))
            {
                LOGERR("ERROR: Bad arguments - deviceID: %d, keyName: 0x%02X!  Failed to clear map!", deviceID, keyName);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Failed to clear map!");
                return false;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
            ribRequest.attribute_index  = (unsigned char)rfKey;
            ribRequest.length           = 1 + 2 + CONTROLMGR_MAX_IR_DATA_SIZE;
            ribRequest.data[0]          = flags;

            // Direct write to the RIB IRRFDB entry for this RF key.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Wrote RIB IR-RF Database: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGWARN("Successfully cleared RIB IRRFDB entry for RF key 0x%02X.\n", (unsigned)rfKey);
                }
                else
                {
                    LOGERR("FAILURE result in SET ribRequest! status: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                return false;
            }

            // If we are clearing a power entry, clear the corresponding separate "device" power entry, too.
            if ((rfKey == MSO_RFKEY_PWR_TOGGLE) ||
                (rfKey == MSO_RFKEY_PWR_OFF) ||
                (rfKey == MSO_RFKEY_PWR_ON))
            {
                clearDevicePower(deviceID, keymapType, rfKey);
            }

            return true;
        }

        bool RemoteActionMappingHelper::setDevicePower(int deviceID, int keymapType, keyActionMap& actionMap)
        {
            byte_vector_t irData;
            int tvRFKey = 0;
            int avrRFKey = 0;
            bool tvOK = true;
            bool avrOK = true;

            if (deviceID < 1)
            {
                LOGERR("ERROR: Bad argument - deviceID: %d!", deviceID);
                return false;
            }

            if ((actionMap.rfKeyCode != MSO_RFKEY_PWR_TOGGLE) &&
                (actionMap.rfKeyCode != MSO_RFKEY_PWR_OFF) &&
                (actionMap.rfKeyCode != MSO_RFKEY_PWR_ON))
            {
                LOGERR("ERROR: Bad rfKeyCode in actionMap: 0x%02X!", actionMap.rfKeyCode);
                return false;
            }

            // Translate from the original 3 rfKeyCodes to the appropriate separate TV and AVR RF key codes
            switch (actionMap.rfKeyCode)
            {
                case MSO_RFKEY_PWR_TOGGLE:
                    tvRFKey = XRC_RFKEY_TV_PWR_TOGGLE;
                    avrRFKey = XRC_RFKEY_AVR_PWR_TOGGLE;
                    LOGWARN("Power Toggle - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
                case MSO_RFKEY_PWR_OFF:
                    tvRFKey = XRC_RFKEY_TV_PWR_OFF;
                    avrRFKey = XRC_RFKEY_AVR_PWR_OFF;
                    LOGWARN("Discrete Power Off - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
                case MSO_RFKEY_PWR_ON:
                    tvRFKey = XRC_RFKEY_TV_PWR_ON;
                    avrRFKey = XRC_RFKEY_AVR_PWR_ON;
                    LOGWARN("Discrete Power On - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
            }

            // We ignore the "srcInfo" decisions, and either set or clear, according to what is available in the actionMap,
            irData.clear();
            if (actionMap.tvIRData.size() > 0)
            {
                // We have TV IR data, so we should set the separate TV slot for the key
                irData = actionMap.tvIRData;
                LOGWARN("Setting separate TV key slot 0x%02X.", tvRFKey);
                tvOK = setRIBDevicePower(deviceID, keymapType, tvRFKey, irData);
            }
            else
            {
                // We should clear the separate TV slot for the key
                LOGWARN("Clearing separate TV key slot 0x%02X.", tvRFKey);
                tvOK = clearRIBDevicePower(deviceID, keymapType, tvRFKey);
            }
            irData.clear();
            if (actionMap.avrIRData.size() > 0)
            {
                // We have AVR IR data, so we should set the separate AVR slot for the key
                irData = actionMap.avrIRData;
                LOGWARN("Setting separate AVR key slot 0x%02X.", avrRFKey);
                avrOK = setRIBDevicePower(deviceID, keymapType, avrRFKey, irData);
            }
            else
            {
                // We should clear the separate AVR slot for the key
                LOGWARN("Clearing separate AVR key slot 0x%02X.", avrRFKey);
                avrOK = clearRIBDevicePower(deviceID, keymapType, avrRFKey);
            }

            return (tvOK && avrOK);
        }

        bool RemoteActionMappingHelper::clearDevicePower(int deviceID, int keymapType, int rfKeyCode)
        {
            int tvRFKey = 0;
            int avrRFKey = 0;
            bool tvOK = true;
            bool avrOK = true;

            if (deviceID < 1)
            {
                LOGERR("ERROR: Bad deviceID: %d!", deviceID);
                return false;
            }

            if ((rfKeyCode != MSO_RFKEY_PWR_TOGGLE) &&
                (rfKeyCode != MSO_RFKEY_PWR_OFF) &&
                (rfKeyCode != MSO_RFKEY_PWR_ON))
            {
                LOGERR("ERROR: Bad rfKeyCode: 0x%02X!", rfKeyCode);
                return false;
            }

            // Translate from the original 3 rfKeyCodes to the appropriate separate TV and AVR RF key codes
            switch (rfKeyCode)
            {
                case MSO_RFKEY_PWR_TOGGLE:
                    tvRFKey = XRC_RFKEY_TV_PWR_TOGGLE;
                    avrRFKey = XRC_RFKEY_AVR_PWR_TOGGLE;
                    LOGWARN("Power Toggle - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
                case MSO_RFKEY_PWR_OFF:
                    tvRFKey = XRC_RFKEY_TV_PWR_OFF;
                    avrRFKey = XRC_RFKEY_AVR_PWR_OFF;
                    LOGWARN("Discrete Power Off - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
                case MSO_RFKEY_PWR_ON:
                    tvRFKey = XRC_RFKEY_TV_PWR_ON;
                    avrRFKey = XRC_RFKEY_AVR_PWR_ON;
                    LOGWARN("Discrete Power On - TV key slot 0x%02X, AVR key slot 0x%02X.", tvRFKey, avrRFKey);
                    break;
            }
            LOGWARN("Clearing separate TV key slot 0x%02X.", tvRFKey);
            tvOK = clearRIBDevicePower(deviceID, keymapType, tvRFKey);
            LOGWARN("Clearing separate AVR key slot 0x%02X.", avrRFKey);
            avrOK = clearRIBDevicePower(deviceID, keymapType, avrRFKey);

            return (tvOK && avrOK);
        }

        bool RemoteActionMappingHelper::setRIBDevicePower(int deviceID, int keymapType, int rfKeyCode, byte_vector_t& irData)
        {
            UNUSED(keymapType);
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            int             dataSize = irData.size();
            unsigned char*  data = irData.data();
            unsigned char   flags = MSO_RIB_IRRFDB_PERMANENT_BIT | MSO_RIB_IRRFDB_IRSPECIFIED_BIT;
            unsigned char   irConfig = 0;
            unsigned char*  bytePtr = (unsigned char*)&(ribRequest.data[0]);
            unsigned char*  rfDesc = NULL;
            size_t          rfDescLength = 0;
            size_t          total = 0;

            if ((deviceID < 1) || (irData.size() <= 1))
            {
                LOGERR("ERROR - Bad arguments - deviceID: %d, data size: %d!! Cannot set device power IRRFDB entry!",
                       deviceID, irData.size());
                return false;
            }

            if (!((rfKeyCode == XRC_RFKEY_TV_PWR_TOGGLE) ||
                  (rfKeyCode == XRC_RFKEY_TV_PWR_OFF) ||
                  (rfKeyCode == XRC_RFKEY_TV_PWR_ON) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_TOGGLE) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_OFF) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_ON) ))
            {
                LOGERR("ERROR - Wrong rfKeyCode(0x%02X) - NOT a device power rfKeyCode code!!", rfKeyCode);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found!  Cannot set map!");
                return false;
            }

            if (dataSize > CONTROLMGR_MAX_IR_DATA_SIZE)
            {
                LOGERR("LOGIC ERROR - IRCode dataSize %d, for RF Key 0x%02X, exceeds size limits!",
                       dataSize, (unsigned)rfKeyCode);
                return false;
            }

            // Construct the direct RIB entry for this RF key
            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
            ribRequest.attribute_index  = (unsigned char)rfKeyCode;

            // Decide what RF Descriptor we need to add for this RF key.
            switch (rfKeyCode)
            {
                case XRC_RFKEY_TV_PWR_TOGGLE:
                case XRC_RFKEY_AVR_PWR_TOGGLE:
                    rfDesc = (unsigned char*)rfDescriptor_IRPowerToggle;
                    rfDescLength = sizeof(rfDescriptor_IRPowerToggle);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("IR Power Toggle RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case XRC_RFKEY_TV_PWR_OFF:
                case XRC_RFKEY_AVR_PWR_OFF:
                    rfDesc = (unsigned char*)rfDescriptor_DiscretePwrOff;
                    rfDescLength = sizeof(rfDescriptor_DiscretePwrOff);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Discrete Power Off RF Descriptor included, size: %d.", rfDescLength);
                    break;
                case XRC_RFKEY_TV_PWR_ON:
                case XRC_RFKEY_AVR_PWR_ON:
                    rfDesc = (unsigned char*)rfDescriptor_DiscretePwrOn;
                    rfDescLength = sizeof(rfDescriptor_DiscretePwrOn);
                    flags |= MSO_RIB_IRRFDB_RFRELEASED_BIT;
                    LOGWARN("Discrete Power On RF Descriptor included, size: %d.", rfDescLength);
                    break;
                default:
                    LOGERR("LOGIC ERROR - Invalid separate power RF Key 0x%02X.", (unsigned)rfKeyCode);
                    break;
            }

            // Set the DeviceType flags field properly
            if ((rfKeyCode == XRC_RFKEY_AVR_PWR_TOGGLE) ||
                (rfKeyCode == XRC_RFKEY_AVR_PWR_OFF) ||
                (rfKeyCode == XRC_RFKEY_AVR_PWR_ON))
            {
                flags |= MSO_RIB_IRRFDB_DEVICETYPE_AVR;
            }

            // Set the IR Config byte properly
            if ((rfKeyCode == XRC_RFKEY_TV_PWR_OFF) ||
                (rfKeyCode == XRC_RFKEY_TV_PWR_ON) ||
                (rfKeyCode == XRC_RFKEY_AVR_PWR_OFF) ||
                (rfKeyCode == XRC_RFKEY_AVR_PWR_ON))
            {
                irConfig = 0x4F;    // Tweak database, min transmissions = 0xF, only for discrete power
            }

            // We now know what the write size will be.
            total = 1 + rfDescLength + 2 + dataSize;
            if (total > CTRLM_RCU_MAX_RIB_ATTRIBUTE_SIZE)
            {
                LOGERR("LOGIC ERROR - IR-RF DB entry length is %u, for RF Key 0x%02X, exceeds size limits!!",
                       total, (unsigned)rfKeyCode);
                return false;
            }
            ribRequest.length = (unsigned char)total;

            // Copy all the data into the ribRequest structure
            *bytePtr = flags;
            bytePtr++;
            if (rfDesc != NULL)
            {
                memcpy(bytePtr, rfDesc, rfDescLength);
                bytePtr += rfDescLength;
            }
            *bytePtr = irConfig;
            bytePtr++;
            *bytePtr = (unsigned char)dataSize;
            bytePtr++;
            memcpy(bytePtr, data, dataSize);

            // Do the direct write to the IR-RF DB RIB entry.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Set RIB IR-RF DB Request: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGWARN("separate map set for rfKeyCode: 0x%02X, IrCode size: %d.\n",
                            (unsigned)rfKeyCode, dataSize);
                }
                else
                {
                    LOGERR("FAILURE result in SET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of setRIBDevicePower()

        bool RemoteActionMappingHelper::clearRIBDevicePower(int deviceID, int keymapType, int rfKeyCode)
        {
            UNUSED(keymapType);
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            unsigned char   flags = MSO_RIB_IRRFDB_PERMANENT_BIT | MSO_RIB_IRRFDB_DEFAULT_BIT;

            if ((deviceID < 1) || (rfKeyCode <= 0))
            {
                LOGERR("ERROR: Bad arguments - deviceID: %d, rfKeyCode: 0x%02X!  Failed to clear map!", deviceID, rfKeyCode);
                return false;
            }

            if (!((rfKeyCode == XRC_RFKEY_TV_PWR_TOGGLE) ||
                  (rfKeyCode == XRC_RFKEY_TV_PWR_OFF) ||
                  (rfKeyCode == XRC_RFKEY_TV_PWR_ON) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_TOGGLE) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_OFF) ||
                  (rfKeyCode == XRC_RFKEY_AVR_PWR_ON) ))
            {
                LOGERR("ERROR - Wrong rfKeyCode(0x%02X) - NOT a device power RFKey code!!", rfKeyCode);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Failed to clear map!");
                return false;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE;
            ribRequest.attribute_index  = (unsigned char)rfKeyCode;
            ribRequest.length           = 1 + 2 + CONTROLMGR_MAX_IR_DATA_SIZE;
            ribRequest.data[0]          = flags;

            // Direct write to the RIB IRRFDB entry for this RF key.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Wrote RIB IR-RF Database: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    LOGWARN("Successfully cleared separate power slot for rfKeyCode 0x%02X.\n", (unsigned)rfKeyCode);
                }
                else
                {
                    LOGERR("FAILURE result in SET ribRequest! status: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of clearRIBDevicePower

        // Note that, regardless of how we set the IRRF Database-related IRRF Status Flags, we will clear all 5-Digit Code-related flags here.
        bool RemoteActionMappingHelper::setIRDBDownloadFlag(int deviceID, bool bDownload)
        {
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            unsigned char flags = (unsigned char)(bDownload ? XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT : XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT);

#ifdef RAMS_USE_FORCE_DOWNLOAD
            if (bDownload) flags |= XRC_RIB_IRRFSTATUS_FORCE_IRDB_BIT;  // Set force download
#endif

            if (deviceID < 1)
            {
                LOGERR("Bad deviceID: %d! Unable to change flags!", deviceID);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Unable to change flags!");
                return false;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS;
            ribRequest.attribute_index  = 0;    // TODO: XRC says this is supposed to be the "key" - what does that mean in this context?
            ribRequest.length           = 1;

            // Read the RIB IRRF Status to get the current Flags setting
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Current RIB IR-RF Status: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    if (((unsigned char)ribRequest.data[0]) != flags)
                    {
                        // Write the IRRF Status Flags byte in the RIB
                        ribRequest.data[0] = flags;

                        // If the length was changed by controlMgr, set it back to 1
                        if(ribRequest.length != 1)
                        {
                            LOGWARN("Length changed by controlMgr in GET ribRequest from 1 to %d!  Setting back to 1.\n", ribRequest.length);
                            ribRequest.length = 1;
                        }

                        res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
                        if (res == IARM_RESULT_SUCCESS)
                        {
                            if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                            {
                                LOGWARN("Set RIB IR-RF Status success: controller_id: %u, network_id: 0x%02X, "
                                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                            }
                            else
                            {
                                LOGERR("FAILURE result in SET ribRequest! result: %d.\n", ribRequest.result);
                                return false;
                            }
                        }
                        else
                        {
                            LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                            return false;
                        }
                    }
                    else
                    {
                        LOGWARN("Not writing IRRF Status Flags - flags already set to 0x%02X.\n", (unsigned char)ribRequest.data[0]);
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            return true;
        }

        bool RemoteActionMappingHelper::setFiveDigitCode(int deviceID, int tvFiveDigitCode, int avrFiveDigitCode)
        {
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            char                                codestr[6];
            unsigned char                       flags = 0;

            if (deviceID < 1)
            {
                LOGERR("Bad deviceID: %d! Unable to change flags!", deviceID);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Unable to change flags!");
                return false;
            }

            if ((tvFiveDigitCode == 0) && (avrFiveDigitCode == 0))
            {
                // Nothing to set into the Target IRDB Status - what is wanted is a Clear All operation - not done here.
                return true;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_TARGET_IRDB_STATUS;
            ribRequest.attribute_index  = 0;    // TODO: XRC says this is supposed to be the "key" - what does that mean in this context?
            ribRequest.length           = CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS;

            // Read the entire RIB Target IRDB Status attribute, to get the current Flags and  TV and AVR strings.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Current RIB Target IRDB Status: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    if (ribRequest.length == 13)
                    {
                        LOGWARN("Target IRDB Status Data: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, "
                                "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X.",
                                (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1],
                                (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3], (unsigned char)ribRequest.data[4],
                                (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                                (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10],
                                (unsigned char)ribRequest.data[11], (unsigned char)ribRequest.data[12]);
                    }
                    flags = (unsigned char)ribRequest.data[0];
                    // Write the TV and/or AVR 5-digit codes, as strings, to the data area.
                    if (tvFiveDigitCode > 0)
                    {
                        memset((void*)codestr, 0, sizeof(codestr));
                        snprintf(codestr, 6, "%05u", tvFiveDigitCode);
                        memcpy((void*)&ribRequest.data[1], (const void*)codestr, 6);
                        flags |= XRC_RIB_TARGET_STATUS_FLAGS_TV_CODE_PRESENT_BIT;
                        // Clear the Not Programmed and/or IRRF Database bits, if they were set.
                        if ((flags & (XRC_RIB_TARGET_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT | XRC_RIB_TARGET_STATUS_FLAGS_IRRF_DATABASE_BIT)) != 0)
                        {
                            flags &= ~(XRC_RIB_TARGET_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT | XRC_RIB_TARGET_STATUS_FLAGS_IRRF_DATABASE_BIT);
                        }
                    }
                    if (avrFiveDigitCode > 0)
                    {
                        memset((void*)codestr, 0, sizeof(codestr));
                        snprintf(codestr, 6, "%05u", avrFiveDigitCode);
                        memcpy((void*)&ribRequest.data[7], (const void*)codestr, 6);
                        flags |= XRC_RIB_TARGET_STATUS_FLAGS_AVR_CODE_PRESENT_BIT;
                        // Clear the Not Programmed and/or IRRF Database bits, if they were set.
                        if ((flags & (XRC_RIB_TARGET_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT | XRC_RIB_TARGET_STATUS_FLAGS_IRRF_DATABASE_BIT)) != 0)
                        {
                            flags &= ~(XRC_RIB_TARGET_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT | XRC_RIB_TARGET_STATUS_FLAGS_IRRF_DATABASE_BIT);
                        }
                    }
                    // Update the flags byte in the request.
                    ribRequest.data[0] = flags;
                    // Write the Target IRDB Status attribute back to the RIB.
                    ribRequest.length = CTRLM_RCU_RIB_ATTR_LEN_TARGET_IRDB_STATUS;
                    res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
                    if (res == IARM_RESULT_SUCCESS)
                    {
                        if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                        {
                            LOGWARN("Set RIB Target IRDB Status success: controller_id: %u, network_id: 0x%02X, "
                                    "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u.",
                                    ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                                    ribRequest.attribute_index, ribRequest.result, ribRequest.length);
                            if (ribRequest.length == 13)
                            {
                                LOGWARN("new Target IRDB Status Data: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, "
                                        "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X.",
                                        (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1],
                                        (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3], (unsigned char)ribRequest.data[4],
                                        (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                                        (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10],
                                        (unsigned char)ribRequest.data[11], (unsigned char)ribRequest.data[12]);
                            }
                        }
                        else
                        {
                            LOGERR("FAILURE result in SET ribRequest! result: %d.\n", ribRequest.result);
                            return false;
                        }
                    }
                    else
                    {
                        LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                        return false;
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of setFiveDigitCode()

        // Note that, regardless of how we set the 5-Digit Code-related IRRF Status Flags, we will clear all IRRF Database-related flags here.
        bool RemoteActionMappingHelper::setFiveDigitCodeFlags(int deviceID, int mode)
        {
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            unsigned char flags = (unsigned char)XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT;

            if (deviceID < 1)
            {
                LOGERR("Bad deviceID: %d! Unable to change flags!", deviceID);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Unable to change flags!");
                return false;
            }

            // Translate the "mode" into the flags we want to set.
            switch (mode)
            {
                case FIVE_DIGIT_CODE_MODE_NONE:         flags |= 0; break;  // Set NO additional flags
                case FIVE_DIGIT_CODE_MODE_TV_SET:       flags |= XRC_RIB_IRRFSTATUS_DOWNLOAD_TV_5DCODE_BIT; break;
                case FIVE_DIGIT_CODE_MODE_AVR_SET:      flags |= XRC_RIB_IRRFSTATUS_DOWNLOAD_AVR_5DCODE_BIT; break;
                case FIVE_DIGIT_CODE_MODE_TVAVR_SET:    flags |= (XRC_RIB_IRRFSTATUS_DOWNLOAD_TV_5DCODE_BIT | XRC_RIB_IRRFSTATUS_DOWNLOAD_AVR_5DCODE_BIT); break;
                case FIVE_DIGIT_CODE_MODE_CLEAR:        flags |= XRC_RIB_IRRFSTATUS_CLEAR_ALL_5DCODES_BIT; break;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS;
            ribRequest.attribute_index  = 0;    // TODO: XRC says this is supposed to be the "key" - what does that mean in this context?
            ribRequest.length           = 1;

            // Read the RIB IRRF Status to get the current Flags setting
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Current RIB IR-RF Status: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    if (((unsigned char)ribRequest.data[0]) != flags)
                    {
                        // Write the IRRF Status Flags byte in the RIB
                        ribRequest.data[0] = flags;

                        // If the length was changed by controlMgr, set it back to 1
                        if(ribRequest.length != 1)
                        {
                            LOGWARN("Length changed by controlMgr in GET ribRequest from 1 to %d!  Setting back to 1.\n", ribRequest.length);
                            ribRequest.length = 1;
                        }

                        res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_SET, (void *)&ribRequest, sizeof(ribRequest));
                        if (res == IARM_RESULT_SUCCESS)
                        {
                            if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                            {
                                LOGWARN("Set RIB IR-RF Status success: controller_id: %u, network_id: 0x%02X, "
                                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                            }
                            else
                            {
                                LOGERR("FAILURE result in SET ribRequest! result: %d.\n", ribRequest.result);
                                return false;
                            }
                        }
                        else
                        {
                            LOGERR("FAILURE in bus call RIB_REQUEST_SET! return value: %d.\n", res);
                            return false;
                        }
                    }
                    else
                    {
                        LOGERR("Not writing IRRF Status Flags - flags already set to 0x%02X.\n", (unsigned char)ribRequest.data[0]);
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of setFiveDigitCodeFlags()

        bool RemoteActionMappingHelper::getControllerLoadStatus(int deviceID, unsigned& tvLoadStatus, unsigned& avrLoadStatus)
        {
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;

            if (deviceID < 1)
            {
                LOGERR("Bad deviceID: %d! Unable to change flags!", deviceID);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Unable to change flags!");
                return false;
            }

            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS;
            ribRequest.attribute_index  = 0;    // TODO: XRC says this is supposed to be the "key" - what does that mean in this context?
            ribRequest.length           = CTRLM_RCU_RIB_ATTR_LEN_CONTROLLER_IRDB_STATUS;

            // Read the entire RIB Controller IRDB Status attribute, to get the current TV and AVR Load Status bytes.
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Current RIB Controller IRDB Status: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    if (ribRequest.length == 13)
                    {
                        LOGWARN("Controller IRDB Status Data: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, "
                                "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X.",
                                (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1],
                                (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3], (unsigned char)ribRequest.data[4],
                                (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                                (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10],
                                (unsigned char)ribRequest.data[11], (unsigned char)ribRequest.data[12]);
                    }
                    else if (ribRequest.length == 15)
                    {
                        LOGWARN("Controller IRDB Status Data: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, "
                                "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X.",
                                (unsigned char)ribRequest.data[0], (unsigned char)ribRequest.data[1],
                                (unsigned char)ribRequest.data[2], (unsigned char)ribRequest.data[3], (unsigned char)ribRequest.data[4],
                                (unsigned char)ribRequest.data[5], (unsigned char)ribRequest.data[6], (unsigned char)ribRequest.data[7],
                                (unsigned char)ribRequest.data[8], (unsigned char)ribRequest.data[9], (unsigned char)ribRequest.data[10],
                                (unsigned char)ribRequest.data[11], (unsigned char)ribRequest.data[12],
                                (unsigned char)ribRequest.data[13], (unsigned char)ribRequest.data[14]);
                    }
                    tvLoadStatus = 0;
                    if (ribRequest.length == 15)
                    {
                        tvLoadStatus = ribRequest.data[13];
                    }
                    avrLoadStatus = 0;
                    if (ribRequest.length == 15)
                    {
                        avrLoadStatus = ribRequest.data[14];
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            return true;
        }   // end of getControllerLoadStatus()

        bool RemoteActionMappingHelper::cancelCodeDownload(int deviceID)
        {
            ctrlm_rcu_iarm_call_rib_request_t   ribRequest;
            ctrlm_network_id_t                  rf4ceId;
            IARM_Result_t                       res;
            unsigned char                       flags = 0;
            bool                                bIRRFDBPending = false;

            if (deviceID < 1)
            {
                LOGERR("Bad deviceID: %d! Unable to change flags!", deviceID);
                return false;
            }

            rf4ceId = getRf4ceNetworkID();
            if (rf4ceId == CTRLM_MAIN_NETWORK_ID_INVALID)
            {
                LOGERR("FAILURE - No RF4CE network_id found! Unable to change flags!");
                return false;
            }

            // Find out if there is a IRRF Database request waiting to go off.
            memset((void*)&ribRequest, 0, sizeof(ctrlm_rcu_iarm_call_rib_request_t));
            ribRequest.api_revision     = CTRLM_RCU_IARM_BUS_API_REVISION;
            ribRequest.network_id       = rf4ceId;
            ribRequest.controller_id    = deviceID;
            ribRequest.attribute_id     = CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS;
            ribRequest.attribute_index  = 0;    // TODO: XRC says this is supposed to be the "key" - what does that mean in this context?
            ribRequest.length           = 1;

            // Read the RIB IRRF Status to get the current Flags setting
            res = IARM_Bus_Call(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_CALL_RIB_REQUEST_GET, (void *)&ribRequest, sizeof(ribRequest));
            if (res == IARM_RESULT_SUCCESS)
            {
                LOGWARN("Current RIB IR-RF Status: controller_id: %u, network_id: 0x%02X, "
                        "attribute_id: 0x%02X, attribute_index: 0x%02X, result: %u, length: %u, data[0]: 0x%02X.",
                        ribRequest.controller_id, ribRequest.network_id, (unsigned char)ribRequest.attribute_id,
                        ribRequest.attribute_index, ribRequest.result, ribRequest.length, (unsigned char)ribRequest.data[0]);
                if (ribRequest.result == CTRLM_IARM_CALL_RESULT_SUCCESS)
                {
                    flags = ((unsigned char)ribRequest.data[0]);
                    if ((flags & (XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT | XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT)) == XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT)
                    {
                        // There was a IRRF Database download pending - clear the IRRF Databass entries.
                        bIRRFDBPending = true;
                    }
                    else
                    {
                        LOGWARN("Not clearing IRRF Database entries - no download pending.\n");
                    }
                }
                else
                {
                    LOGERR("FAILURE result in GET ribRequest! result: %d.\n", ribRequest.result);
                    return false;
                }
            }
            else
            {
                LOGERR("FAILURE in bus call RIB_REQUEST_GET! return value: %d.\n", res);
                return false;
            }

            // This clears all 5-digit code flags, and sets the IRRF Database flags back to normal idle (Do Not Download).
            setFiveDigitCodeFlags(deviceID, FIVE_DIGIT_CODE_MODE_NONE);
            if (bIRRFDBPending)
            {
                // Clear all the IRRF Database entries.
                clearKeyActionMap(deviceID, 0, KED_VOLUMEUP);
                clearKeyActionMap(deviceID, 0, KED_VOLUMEDOWN);
                clearKeyActionMap(deviceID, 0, KED_MUTE);
                clearKeyActionMap(deviceID, 0, KED_INPUTKEY);
                clearKeyActionMap(deviceID, 0, KED_POWER);
                clearKeyActionMap(deviceID, 0, KED_DISCRETE_POWER_ON);
                clearKeyActionMap(deviceID, 0, KED_DISCRETE_POWER_STANDBY);
                LOGWARN("Pending IRRF Database download cancelled - all entries cleared!\n");
            }

            return true;
        }

        int RemoteActionMappingHelper::lookupRFKey(int keyName)
        {
            int rfKey = -1;

            switch(keyName)
            {
                case KED_VOLUMEUP:
                    rfKey = MSO_RFKEY_VOL_PLUS;
                    break;
                case KED_VOLUMEDOWN:
                    rfKey = MSO_RFKEY_VOL_MINUS;
                    break;
                case KED_MUTE:
                    rfKey = MSO_RFKEY_MUTE;
                    break;
                case KED_INPUTKEY:
                    rfKey = MSO_RFKEY_INPUT_SELECT;
                    break;
                case KED_TVPOWER:
                    rfKey = MSO_RFKEY_PWR_TOGGLE;
                    break;
                case KED_POWER:
                    rfKey = MSO_RFKEY_PWR_TOGGLE;
                    break;
                case KED_RF_POWER:
                    rfKey = MSO_RFKEY_PWR_TOGGLE;
                    break;
                case KED_DISCRETE_POWER_ON:
                    rfKey = MSO_RFKEY_PWR_ON;
                    break;
                case KED_DISCRETE_POWER_STANDBY:
                    rfKey = MSO_RFKEY_PWR_OFF;
                    break;
            }

            return rfKey;
        }


        int RemoteActionMappingHelper::lookupKeyname(int rfKey)
        {
            int keyName = -1;

            switch(rfKey)
            {
                case MSO_RFKEY_VOL_PLUS:
                    keyName = KED_VOLUMEUP;
                    break;
                case MSO_RFKEY_VOL_MINUS:
                    keyName = KED_VOLUMEDOWN;
                    break;
                case MSO_RFKEY_MUTE:
                    keyName = KED_MUTE;
                    break;
                case MSO_RFKEY_INPUT_SELECT:
                    keyName = KED_INPUTKEY;
                    break;
                case MSO_RFKEY_PWR_TOGGLE:
                    keyName = KED_POWER;
                    break;
                case MSO_RFKEY_PWR_ON:
                    keyName = KED_DISCRETE_POWER_ON;
                    break;
                case MSO_RFKEY_PWR_OFF:
                    keyName = KED_DISCRETE_POWER_STANDBY;
                    break;
                // Map the 6 separate power RF code back to the appropriate/related keynames, too
                case XRC_RFKEY_TV_PWR_TOGGLE:
                    keyName = KED_POWER;
                    break;
                case XRC_RFKEY_TV_PWR_ON:
                    keyName = KED_DISCRETE_POWER_ON;
                    break;
                case XRC_RFKEY_TV_PWR_OFF:
                    keyName = KED_DISCRETE_POWER_STANDBY;
                    break;
                case XRC_RFKEY_AVR_PWR_TOGGLE:
                    keyName = KED_POWER;
                    break;
                case XRC_RFKEY_AVR_PWR_ON:
                    keyName = KED_DISCRETE_POWER_ON;
                    break;
                case XRC_RFKEY_AVR_PWR_OFF:
                    keyName = KED_DISCRETE_POWER_STANDBY;
                    break;
            }

            return keyName;
        }

    } // namespace Plugin

} // namespace WPEFramework
