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

#pragma once

#include "Module.h"

//IARM includes
#include "libIBus.h"

#include "ctrlm_ipc.h"
#include "ctrlm_ipc_rcu.h"

#include <string>
#include <vector>

#include "comcastIrKeyCodes.h"

typedef std::vector<unsigned char>  byte_vector_t;
typedef std::vector<int>            int_vector_t;

#define RAMS_USE_FORCE_DOWNLOAD     1

struct controller_info {
    ctrlm_network_id_t          network_id;
    ctrlm_controller_id_t       controller_id;
    ctrlm_controller_status_t   status;
};

struct rf4ceBindRemotes_t {
    int                 numBindRemotes;
    controller_info     remotes[CTRLM_MAIN_MAX_BOUND_CONTROLLERS];
};

// Mapped MSO RF Keys
#define MSO_RFKEY_INPUT_SELECT      0x034

#define MSO_RFKEY_VOL_PLUS          0x041
#define MSO_RFKEY_VOL_MINUS         0x042
#define MSO_RFKEY_MUTE              0x043

#define XRC_RFKEY_TV_PWR_TOGGLE     0x040   // 3 new TV power keycodes (XRC Profile update)
#define XRC_RFKEY_TV_PWR_ON         0x04D
#define XRC_RFKEY_TV_PWR_OFF        0x04E

#define XRC_RFKEY_AVR_PWR_TOGGLE    0x068   // 3 new AVR power keycodes (XRC Profile update)
#define XRC_RFKEY_AVR_PWR_OFF       0x069
#define XRC_RFKEY_AVR_PWR_ON        0x06A

#define MSO_RFKEY_PWR_TOGGLE        0x06B   // Older power keycodes
#define MSO_RFKEY_PWR_OFF           0x06C
#define MSO_RFKEY_PWR_ON            0x06D

// IR-RF Database Flags byte bit masks
#define MSO_RIB_IRRFDB_PERMANENT_BIT    0x080
#define MSO_RIB_IRRFDB_DEFAULT_BIT      0x040
#define MSO_RIB_IRRFDB_DEVICETYPE_BITS  0x030
#define MSO_RIB_IRRFDB_DEVICETYPE_AVR   0x020
#define MSO_RIB_IRRFDB_IRSPECIFIED_BIT  0x008
#define MSO_RIB_IRRFDB_RFRELEASED_BIT   0x004
#define MSO_RIB_IRRFDB_RFREPEATED_BIT   0x002
#define MSO_RIB_IRRFDB_RFPRESSED_BIT    0x001

// IR-RF Status Flags byte bit masks
#define XRC_RIB_IRRFSTATUS_DOWNLOAD_IRDB_BIT        0x080
#define XRC_RIB_IRRFSTATUS_DONT_DOWNLOAD_IRDB_BIT   0x040
#define XRC_RIB_IRRFSTATUS_CLEAR_ALL_5DCODES_BIT    0x010
#define XRC_RIB_IRRFSTATUS_TRANSMIT_DESCRIPTOR_BIT  0x008
#define XRC_RIB_IRRFSTATUS_DOWNLOAD_AVR_5DCODE_BIT  0x004
#define XRC_RIB_IRRFSTATUS_DOWNLOAD_TV_5DCODE_BIT   0x002
#define XRC_RIB_IRRFSTATUS_FORCE_IRDB_BIT           0x001

// Controller IRDB Status Flags byte bit masks
#define XRC_RIB_CTRLR_STATUS_FLAGS_5DIGIT_SUPPORT_BIT       0x080
#define XRC_RIB_CTRLR_STATUS_FLAGS_AVR_CODE_PRESENT_BIT     0x010
#define XRC_RIB_CTRLR_STATUS_FLAGS_TV_CODE_PRESENT_BIT      0x008
#define XRC_RIB_CTRLR_STATUS_FLAGS_IRRF_DATABASE_BIT        0x004
#define XRC_RIB_CTRLR_STATUS_FLAGS_IRDB_TYPE_BIT            0x002
#define XRC_RIB_CTRLR_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT     0x001

// Controller IRDB Status Load Status byte bit masks
#define XRC_RIB_CTRLR_STATUS_LOAD_RESULT_BITS       0x0F0

// Target IRDB Status Flags byte bit masks
#define XRC_RIB_TARGET_STATUS_FLAGS_AVR_CODE_PRESENT_BIT    0x010
#define XRC_RIB_TARGET_STATUS_FLAGS_TV_CODE_PRESENT_BIT     0x008
#define XRC_RIB_TARGET_STATUS_FLAGS_IRRF_DATABASE_BIT       0x004
#define XRC_RIB_TARGET_STATUS_FLAGS_NO_IR_PROGRAMMED_BIT    0x001

// Maximum size (in bytes) of the IR waveform data
#define CONTROLMGR_MAX_IR_DATA_SIZE     (80)


// Enumeration for combinations of 5-Digit Code flags in the IR-RF Status Flags byte.
typedef enum {
    FIVE_DIGIT_CODE_MODE_NONE,
    FIVE_DIGIT_CODE_MODE_TV_SET,
    FIVE_DIGIT_CODE_MODE_AVR_SET,
    FIVE_DIGIT_CODE_MODE_TVAVR_SET,
    FIVE_DIGIT_CODE_MODE_CLEAR
} FiveDigitCodeMode;

// Definitions used for key group decision making
typedef enum {
    KEY_GROUP_SRC_CLEAR,
    KEY_GROUP_SRC_TV,
    KEY_GROUP_SRC_AVR,
    KEY_GROUP_SRC_TV_PWR_CROSS,     // xxx_PWR_CROSS are only used for the DiscretePower group,
    KEY_GROUP_SRC_AVR_PWR_CROSS     // to implement DiscretePower when only TogglePower is available.
} KeyGroupSrc;

class KeyGroupSrcInfo {
public:
    KeyGroupSrc         groupInputSelect;
    KeyGroupSrc         groupVolume;            // Includes VOL_UP and VOL_DOWN
    KeyGroupSrc         groupMute;
    KeyGroupSrc         groupTogglePower;
    KeyGroupSrc         groupDiscretePower;     // Includes Discrete Power ON and OFF

    KeyGroupSrcInfo() { groupInputSelect = groupVolume = groupMute = groupTogglePower = groupDiscretePower = KEY_GROUP_SRC_CLEAR; }
};

// Classes to aid in key group decisions (for the original 7 IRRFDB slots, especially the legacy power slots)
class RFKeyFlags {
public:
    bool            input;
    bool            vol_up;
    bool            vol_dn;
    bool            mute;
    bool            pwr_toggle;
    bool            pwr_on;
    bool            pwr_off;

    RFKeyFlags() { input = vol_up = vol_dn = mute = pwr_toggle = pwr_on = pwr_off = false; }

    void setKey(int rfKey) {
        switch (rfKey) {
        case MSO_RFKEY_INPUT_SELECT:    input = true;       break;
        case MSO_RFKEY_VOL_PLUS:        vol_up = true;      break;
        case MSO_RFKEY_VOL_MINUS:       vol_dn = true;      break;
        case MSO_RFKEY_MUTE:            mute = true;        break;
        case MSO_RFKEY_PWR_TOGGLE:      pwr_toggle = true;  break;
        case MSO_RFKEY_PWR_OFF:         pwr_off = true;     break;
        case MSO_RFKEY_PWR_ON:          pwr_on = true;      break;
        }
    }

    void clearKey(int rfKey) {
        switch (rfKey) {
        case MSO_RFKEY_INPUT_SELECT:    input = false;      break;
        case MSO_RFKEY_VOL_PLUS:        vol_up = false;     break;
        case MSO_RFKEY_VOL_MINUS:       vol_dn = false;     break;
        case MSO_RFKEY_MUTE:            mute = false;       break;
        case MSO_RFKEY_PWR_TOGGLE:      pwr_toggle = false; break;
        case MSO_RFKEY_PWR_OFF:         pwr_off = false;    break;
        case MSO_RFKEY_PWR_ON:          pwr_on = false;     break;
        }
    }

    bool isSet(int rfKey) {
        bool isset = false;
        switch (rfKey) {
        case MSO_RFKEY_INPUT_SELECT:    isset = input;      break;
        case MSO_RFKEY_VOL_PLUS:        isset = vol_up;     break;
        case MSO_RFKEY_VOL_MINUS:       isset = vol_dn;     break;
        case MSO_RFKEY_MUTE:            isset = mute;       break;
        case MSO_RFKEY_PWR_TOGGLE:      isset = pwr_toggle; break;
        case MSO_RFKEY_PWR_OFF:         isset = pwr_off;    break;
        case MSO_RFKEY_PWR_ON:          isset = pwr_on;     break;
        }
        return isset;
    }

};

class KeyPresenceFlags {
public:
    RFKeyFlags      tv;
    RFKeyFlags      avr;
};

// Class used for evaluating the target controller's IR-RF Database read/load progress
class IRRFDBCtrlrLoadProgress {
public:
    bool slot_INPUT_SELECT;
    bool slot_VOL_PLUS;
    bool slot_VOL_MINUS;
    bool slot_MUTE;
    bool slot_TV_PWR_TOGGLE;
    bool slot_TV_PWR_ON;
    bool slot_TV_PWR_OFF;
    bool slot_AVR_PWR_TOGGLE;
    bool slot_AVR_PWR_OFF;
    bool slot_AVR_PWR_ON;
    bool slot_PWR_TOGGLE;
    bool slot_PWR_OFF;
    bool slot_PWR_ON;

    IRRFDBCtrlrLoadProgress() {
        clear();
    }

    void clear() {
        slot_INPUT_SELECT = slot_VOL_PLUS = slot_VOL_MINUS = slot_MUTE =
        slot_TV_PWR_TOGGLE = slot_TV_PWR_ON = slot_TV_PWR_OFF =
        slot_AVR_PWR_TOGGLE = slot_AVR_PWR_OFF = slot_AVR_PWR_ON =
        slot_PWR_TOGGLE = slot_PWR_OFF = slot_PWR_ON = false;
    }

    void setSlotRead(int rfKey, bool state) {
        switch (rfKey) {
        case MSO_RFKEY_INPUT_SELECT:    slot_INPUT_SELECT = state;      break;
        case MSO_RFKEY_VOL_PLUS:        slot_VOL_PLUS = state;          break;
        case MSO_RFKEY_VOL_MINUS:       slot_VOL_MINUS = state;         break;
        case MSO_RFKEY_MUTE:            slot_MUTE = state;              break;
        case XRC_RFKEY_TV_PWR_TOGGLE:   slot_TV_PWR_TOGGLE = state;     break;
        case XRC_RFKEY_TV_PWR_ON:       slot_TV_PWR_ON = state;         break;
        case XRC_RFKEY_TV_PWR_OFF:      slot_TV_PWR_OFF = state;        break;
        case XRC_RFKEY_AVR_PWR_TOGGLE:  slot_AVR_PWR_TOGGLE = state;    break;
        case XRC_RFKEY_AVR_PWR_OFF:     slot_AVR_PWR_OFF = state;       break;
        case XRC_RFKEY_AVR_PWR_ON:      slot_AVR_PWR_ON = state;        break;
        case MSO_RFKEY_PWR_TOGGLE:      slot_PWR_TOGGLE = state;        break;
        case MSO_RFKEY_PWR_OFF:         slot_PWR_OFF = state;           break;
        case MSO_RFKEY_PWR_ON:          slot_PWR_ON = state;            break;
        }
    }

    bool getSlotRead(int rfKey) {
        bool state = false;
        switch (rfKey) {
        case MSO_RFKEY_INPUT_SELECT:    state = slot_INPUT_SELECT;      break;
        case MSO_RFKEY_VOL_PLUS:        state = slot_VOL_PLUS;          break;
        case MSO_RFKEY_VOL_MINUS:       state = slot_VOL_MINUS;         break;
        case MSO_RFKEY_MUTE:            state = slot_MUTE;              break;
        case XRC_RFKEY_TV_PWR_TOGGLE:   state = slot_TV_PWR_TOGGLE;     break;
        case XRC_RFKEY_TV_PWR_ON:       state = slot_TV_PWR_ON;         break;
        case XRC_RFKEY_TV_PWR_OFF:      state = slot_TV_PWR_OFF;        break;
        case XRC_RFKEY_AVR_PWR_TOGGLE:  state = slot_AVR_PWR_TOGGLE;    break;
        case XRC_RFKEY_AVR_PWR_OFF:     state = slot_AVR_PWR_OFF;       break;
        case XRC_RFKEY_AVR_PWR_ON:      state = slot_AVR_PWR_ON;        break;
        case MSO_RFKEY_PWR_TOGGLE:      state = slot_PWR_TOGGLE;        break;
        case MSO_RFKEY_PWR_OFF:         state = slot_PWR_OFF;           break;
        case MSO_RFKEY_PWR_ON:          state = slot_PWR_ON;            break;
        }
        return state;
    }
};

// Class to represent a single instance of a keyActionMap
class keyActionMap
{
public:
    int             keyName;
    int             rfKeyCode;
    byte_vector_t   tvIRData;
    byte_vector_t   avrIRData;
};


namespace WPEFramework {

    namespace Plugin {

        // Helper class for the RemoteActionMapping plugin - contains details of IARM communication with ControlMgr
        class RemoteActionMappingHelper
        {
        public:
            int getLastUsedDeviceID(std::string& remoteType, bool& bFiveDigitCodeSet, bool& bFiveDigitCodeSupported);
            bool getControllerByID(int deviceID, std::string& remoteType, bool& pbFiveDigitCodeSet, bool& pbFiveDigitCodeSupported);
            bool setKeyActionMap(int deviceID, int keymapType, keyActionMap& actionMap, const KeyGroupSrcInfo& srcInfo);
            bool getKeyActionMap(int deviceID, int keymapType, keyActionMap& actionMap);    // In this case, actionMap is an in/out parameter
            bool clearKeyActionMap(int deviceID, int keymapType, int keyName);
            bool setFiveDigitCode(int deviceID, int tvFiveDigitCode, int avrFiveDigitCode);

            bool setIRDBDownloadFlag(int deviceID, bool bDownload);
            bool setFiveDigitCodeFlags(int deviceID, int mode);
            bool cancelCodeDownload(int deviceID);

            bool getControllerLoadStatus(int deviceID, unsigned& tvLoadStatus, unsigned& avrLoadStatus);

            int lookupRFKey(int keyName);
            int lookupKeyname(int rfKey);

            bool setDevicePower(int deviceID, int keymapType, keyActionMap& actionMap);
            bool clearDevicePower(int deviceID, int keymapType, int rfKeyCode);

        private:
            ctrlm_network_id_t getRf4ceNetworkID(void);
            bool getRf4ceBindRemotes(rf4ceBindRemotes_t* bindRemotes);
            bool setRIBDevicePower(int deviceID, int keymapType, int rfKeyCode, byte_vector_t& irData);
            bool clearRIBDevicePower(int deviceID, int keymapType, int rfKeyCode);
        };

    } // namespace Plugin

} // namespace WPEFramework
