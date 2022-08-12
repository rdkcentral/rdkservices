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

#include "RemoteActionMapping.h"
#include "libIBusDaemon.h"

#include "UtilsJsonRpc.h"
#include "UtilsIarm.h"
#include "UtilsUnused.h"
#include <exception>

const int supported_ked_keynames[] =
{
    KED_DISCRETE_POWER_ON,
    KED_DISCRETE_POWER_STANDBY,
    KED_POWER,
    KED_VOLUMEUP,
    KED_VOLUMEDOWN,
    KED_MUTE,
    KED_INPUTKEY
};
const int supported_ked_keynames_size = sizeof(supported_ked_keynames) / sizeof(int);

// In ascending RFKey code order
const int supported_irrfdb_slots[] =
{
    MSO_RFKEY_INPUT_SELECT,     // Non-power original/legacy slots
    MSO_RFKEY_VOL_PLUS,
    MSO_RFKEY_VOL_MINUS,
    MSO_RFKEY_MUTE,

    XRC_RFKEY_TV_PWR_TOGGLE,    // 3 new TV power keycodes (XRC Profile update)
    XRC_RFKEY_TV_PWR_ON,
    XRC_RFKEY_TV_PWR_OFF,

    XRC_RFKEY_AVR_PWR_TOGGLE,   // 3 new AVR power keycodes (XRC Profile update)
    XRC_RFKEY_AVR_PWR_OFF,
    XRC_RFKEY_AVR_PWR_ON,

    MSO_RFKEY_PWR_TOGGLE,       // Original/legacy power slots
    MSO_RFKEY_PWR_OFF,
    MSO_RFKEY_PWR_ON
};
const int supported_irrfdb_slots_size = sizeof(supported_irrfdb_slots) / sizeof(int);

// Encodings for "loadStatus" member in the onIRCodeLoad event
#define IRCODE_LOAD_STATUS_OK                   0
#define IRCODE_LOAD_STATUS_TIMEOUT_COMPLETE     1
#define IRCODE_LOAD_STATUS_TIMEOUT_INCOMPLETE   2
#define IRCODE_LOAD_STATUS_REFUSED              3

// "param" had better be a JsonArray!
#define getArrayParameter(paramName, param) {\
    if (Core::JSON::Variant::type::ARRAY == parameters[paramName].Content()) \
        param = parameters[paramName].Array();\
}
#define getArrayParameterObject(parameters, paramName, param) {\
    if (Core::JSON::Variant::type::ARRAY == parameters[paramName].Content()) \
        param = parameters[paramName].Array();\
}

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

using namespace std;

namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::RemoteActionMapping> metadata(
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

        SERVICE_REGISTRATION(RemoteActionMapping, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        RemoteActionMapping* RemoteActionMapping::_instance = nullptr;

        static Core::TimerType<RibLoadTimeoutImpl> ribLoadTimer(RAM_TIMER_THREAD_STACK_SIZE, RAM_TIMER_THREAD_NAME);

        IRRFDBCtrlrLoadProgress RemoteActionMapping::m_readProgress;
        IRDBLoadState RemoteActionMapping::m_irdbLoadState = IRDB_LOAD_STATE_NONE;
        int RemoteActionMapping::m_lastSetRemoteID = -1;
        int RemoteActionMapping::m_ramsOperatingMode = RAMS_OP_MODE_NONE;
        int RemoteActionMapping::m_fiveDigitCodeMode = FIVE_DIGIT_CODE_MODE_NONE;
        bool RemoteActionMapping::m_lastSetHas5DCPresent = false;
        bool RemoteActionMapping::m_lastSetSupports5DC = false;


        RemoteActionMapping::RemoteActionMapping()
            : PluginHost::JSONRPC()
            , m_apiVersionNumber((uint32_t)-1)   /* default max uint32_t so everything gets enabled */    //TODO(MROLLINS) Can't we access this from jsonrpc interface?
            , m_ribLoadTimeoutImpl(this)
        {
            LOGINFO("ctor");
            RemoteActionMapping::_instance = this;

            Register("getApiVersionNumber",       &RemoteActionMapping::getApiVersionNumber, this);

            Register("getLastUsedDeviceID",       &RemoteActionMapping::getLastUsedDeviceIDWrapper, this);
            Register("getKeymap",                 &RemoteActionMapping::getKeymapWrapper, this);
            Register("setKeyActionMapping",       &RemoteActionMapping::setKeyActionMappingWrapper, this);
            Register("clearKeyActionMapping",     &RemoteActionMapping::clearKeyActionMappingWrapper, this);
            Register("getFullKeyActionMapping",   &RemoteActionMapping::getFullKeyActionMappingWrapper, this);
            Register("getSingleKeyActionMapping", &RemoteActionMapping::getSingleKeyActionMappingWrapper, this);
            Register("cancelCodeDownload",        &RemoteActionMapping::cancelCodeDownloadWrapper, this);
            Register("setFiveDigitCode",          &RemoteActionMapping::setFiveDigitCodeWrapper, this);

            setApiVersionNumber(3);
        }

        RemoteActionMapping::~RemoteActionMapping()
        {
            //LOGINFO("dtor");
        }

        const string RemoteActionMapping::Initialize(PluginHost::IShell* /* service */)
        {
            InitializeIARM();
            // On success return empty, to indicate there is no error text.
            return (string());
        }

        void RemoteActionMapping::Deinitialize(PluginHost::IShell* /* service */)
        {
            DeinitializeIARM();
            RemoteActionMapping::_instance = nullptr;
        }

        void RemoteActionMapping::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, ramEventHandler) );
            }
        }

        void RemoteActionMapping::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(CTRLM_MAIN_IARM_BUS_NAME, CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER, ramEventHandler) );
            }
        }

        // Begin event handlers
        void RemoteActionMapping::ramEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (RemoteActionMapping::_instance)
                RemoteActionMapping::_instance->iarmEventHandler(owner, eventId, data, len);
            else
                LOGWARN("WARNING - cannot handle IARM events without a RemoteActionMapping plugin instance!");
        }

        void RemoteActionMapping::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (!strcmp(owner, CTRLM_MAIN_IARM_BUS_NAME))
            {
                if (eventId == CTRLM_RCU_IARM_EVENT_RIB_ACCESS_CONTROLLER)
                {
                    if (data != NULL)
                    {
                        ctrlm_rcu_iarm_event_rib_entry_access_t *ribEntry = (ctrlm_rcu_iarm_event_rib_entry_access_t*)data;
                        if (ribEntry->api_revision == CTRLM_RCU_IARM_BUS_API_REVISION)
                        {
                            int remoteId   = (unsigned char)ribEntry->controller_id;
                            int networkId  = (unsigned char)ribEntry->network_id;
                            int attrId     = (unsigned char)ribEntry->identifier;
                            int index      = (unsigned char)ribEntry->index;
                            int accessType = (unsigned char)ribEntry->access_type;
                            LOGINFO("RIB Access Event: network_id: %u, controller_id: %d, identifier: 0x%02X, index: 0x%02X, access_type: %s.",
                                    networkId, remoteId, attrId, index, ((accessType > 1) ? "INVALID" : ((accessType == 0) ? "READ" : "WRITE")));

                            std::lock_guard<std::mutex> guard(m_stateMutex);

                            if (m_ramsOperatingMode == RAMS_OP_MODE_IRRF_DATABASE)
                            {
                                // Actions to take, for IRRF_DATABASE operational mode
                                if (accessType == CTRLM_ACCESS_TYPE_READ)
                                {
                                    if ((attrId == CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS) &&
                                        (m_irdbLoadState == IRDB_LOAD_STATE_WRITTEN))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("target remote has read the IRRF Status Flags!");
                                            // Change state to FLAG_READ.  Start the download progress timeout,
                                            // and let the IRDB Download flag remain set until we see either
                                            // some actual load progress, or a timeout.
                                            m_irdbLoadState = IRDB_LOAD_STATE_FLAG_READ;
                                            startRIBLoadTimer(TIMEOUT_REMOTE_IRRFDB_READ_IRCODE_START);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected IRRF Status Flags read by remoteId: %d!", remoteId);
                                        }
                                    }
                                    else if ((attrId == CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE) &&
                                             (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("First IRRF Database read for target remote!");
                                            // This is the very first time we see the actual IRDB data being read by the remote.
                                            m_irdbLoadState = IRDB_LOAD_STATE_PROGRESS;
                                            // Clear the IRDB Download flag (back to default), to restore normal OK key operation.
                                            m_helper.setIRDBDownloadFlag(remoteId, false);

                                            handleIRRFDBKeyRead(remoteId, index);
                                            if (m_irdbLoadState == IRDB_LOAD_STATE_PROGRESS)
                                            {
                                                startRIBLoadTimer(TIMEOUT_REMOTE_IRRFDB_READ_IRCODE_NEXT);
                                            }
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected initial IRRF Database read by remoteId: %d!", remoteId);
                                        }
                                    }
                                    else if ((attrId == CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE) &&
                                             (m_irdbLoadState == IRDB_LOAD_STATE_PROGRESS))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            handleIRRFDBKeyRead(remoteId, index);
                                            if (m_irdbLoadState == IRDB_LOAD_STATE_PROGRESS)
                                            {
                                                startRIBLoadTimer(TIMEOUT_REMOTE_IRRFDB_READ_IRCODE_NEXT);
                                            }
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected IRRF Database read by remoteId: %d!", remoteId);
                                        }
                                    }
                                }
                            }
                            else if ((m_ramsOperatingMode == RAMS_OP_MODE_FIVE_DIGIT_CODE) &&
                                     ((m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_TV_SET) ||
                                      (m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_AVR_SET) ||
                                      (m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_TVAVR_SET)))
                            {
                                // Actions to take, for FIVE_DIGIT_CODE SET mode
                                if (accessType == CTRLM_ACCESS_TYPE_READ)
                                {
                                    if ((attrId == CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS) &&
                                        (m_irdbLoadState == IRDB_LOAD_STATE_WRITTEN))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("target remote has read the IRRF Status Flags!");
                                            // Change state to FLAG_READ.  Start the Target IRDB Status read timeout,
                                            // and let whatever 5-Digit Code flag(s) remain set until we see either
                                            // the Target IRDB Status read, or a timeout.
                                            m_irdbLoadState = IRDB_LOAD_STATE_FLAG_READ;
                                            startRIBLoadTimer(TIMEOUT_REMOTE_FIVEDIGITCODE_READ_TARGET);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected IRRF Status Flags read by remoteId: %d!", remoteId);
                                        }
                                    }
                                    else if ((attrId == CTRLM_RCU_RIB_ATTR_ID_TARGET_IRDB_STATUS) &&
                                             (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("5-Digit Codes read from Target IRDB Status!");
                                            // This is the very first time we see the actual 5-Digit Codes being read by the remote.
                                            m_irdbLoadState = IRDB_LOAD_STATE_TARGET_READ;
                                            // Clear the 5-Digit Code flag(s) (back to default), to restore normal OK key operation.
                                            m_helper.setFiveDigitCodeFlags(remoteId, FIVE_DIGIT_CODE_MODE_NONE);

                                            startRIBLoadTimer(TIMEOUT_REMOTE_FIVEDIGITCODE_WRITE_CONTROLLER);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected Target IRDB Status read by remoteId: %d!", remoteId);
                                        }
                                    }
                                }
                                else
                                {
                                    // accessType == CTRLM_ACCESS_TYPE_WRITE
                                    if ((attrId == CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS) &&
                                        (m_irdbLoadState == IRDB_LOAD_STATE_TARGET_READ))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("Controller IRDB Status updated!");
                                            m_irdbLoadState = IRDB_LOAD_STATE_CTRLR_WRITTEN;
                                            handleFiveDigitCodeAccess(remoteId);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected Controller IRDB Status write by remoteId: %d!", remoteId);
                                        }
                                    }
                                }
                            }
                            else if ((m_ramsOperatingMode == RAMS_OP_MODE_FIVE_DIGIT_CODE) &&
                                       (m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_CLEAR))
                            {
                                // Actions to take, for FIVE_DIGIT_CODE CLEAR mode
                                if (accessType == CTRLM_ACCESS_TYPE_READ)
                                {
                                    if ((attrId == CTRLM_RCU_RIB_ATTR_ID_IR_RF_DATABASE_STATUS) &&
                                        (m_irdbLoadState == IRDB_LOAD_STATE_WRITTEN))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("target remote has read the IRRF Status Flags!");
                                            // Change state to FLAG_READ.  Start the Controller IRDB Status write timeout,
                                            // and let the 5-Digit Code Clear All flag remain set until we see either
                                            // the Controller IRDB Status write, or a timeout.
                                            m_irdbLoadState = IRDB_LOAD_STATE_FLAG_READ;
                                            startRIBLoadTimer(TIMEOUT_REMOTE_FIVEDIGITCODE_WRITE_CONTROLLER);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected IRRF Status Flags read by remoteId: %d!", remoteId);
                                        }
                                    }
                                }
                                else
                                {
                                    // accessType == CTRLM_ACCESS_TYPE_WRITE
                                    if ((attrId == CTRLM_RCU_RIB_ATTR_ID_CONTROLLER_IRDB_STATUS) &&
                                        (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ))
                                    {
                                        if (remoteId == m_lastSetRemoteID)
                                        {
                                            LOGWARN("Controller IRDB Status updated!");
                                            m_irdbLoadState = IRDB_LOAD_STATE_CTRLR_WRITTEN;
                                            // Clear the 5-Digit Code flag(s) (back to default), to restore normal OK key operation.
                                            m_helper.setFiveDigitCodeFlags(remoteId, FIVE_DIGIT_CODE_MODE_NONE);

                                            handleFiveDigitCodeAccess(remoteId);
                                        }
                                        else
                                        {
                                            LOGWARN("unexpected Controller IRDB Status write by remoteId: %d!", remoteId);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // _ramsOperatingMode == RAMS_OP_MODE_NONE
                                // Just ignore these events, but log them (already done far above).
                                LOGWARN("Event ignored - m_ramsOperatingMode is %d.", m_ramsOperatingMode);
                            }
                        }
                        else
                        {
                            LOGERR("ERROR - controlMgr event API version is %d, expected %d!!",
                                   ribEntry->api_revision, CTRLM_RCU_IARM_BUS_API_REVISION);
                        }
                    }
                    else
                    {
                        LOGERR("ERROR - event data is NULL!");
                    }
                }
                else
                {
                    LOGERR("UNKNOWN controlMgr Event: eventId: %d", (int)eventId);
                }
            }
            else
            {
                LOGERR("UNKNOWN EVENT - owner: %s, eventId: %d", owner, (int)eventId);
            }
        }   // end of iarmEventHandler()
        // End event handlers


        // Begin methods
        uint32_t RemoteActionMapping::getApiVersionNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            response["version"] = m_apiVersionNumber;

            returnResponse(true);
        }

        uint32_t RemoteActionMapping::getLastUsedDeviceIDWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            string remoteType;
            int deviceID;
            bool bFiveDigitCodeSet = false;
            bool bFiveDigitCodeSupported = false;

            deviceID = getLastUsedDeviceID(remoteType, bFiveDigitCodeSet, bFiveDigitCodeSupported);
            if (deviceID > 0)
            {
                response["deviceID"] = JsonValue(deviceID);
                response["remoteType"] = remoteType;
                response["fiveDigitCodePresent"] = JsonValue(bFiveDigitCodeSet);
                response["setFiveDigitCodeSupported"] = JsonValue(bFiveDigitCodeSupported);
            } else {
                LOGERR("ERROR: getLastUsedDeviceID returned bad deviceID: %d!\n", deviceID);
                status_code = STATUS_FAILURE;
            }

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t RemoteActionMapping::getKeymapWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int keymapType = -1;

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") || !parameters.HasLabel("keymapType"))
            {
                // There are either NO parameters, or no deviceID, or no keymapType.  We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID' and a 'keymapType' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }
            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
            }
            // Get the keymapType from the parameters
            paramKey = "keymapType";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 1))
                {
                    // The keymapType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keymapType' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keymapType = value;
                LOGINFO("keymapType passed in is %d.", keymapType);
            }

            response["keyNames"] = getKeymap(deviceID, keymapType);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t RemoteActionMapping::setKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int keymapType = -1;
            int numMaps = 0;
            KeyPresenceFlags keysPresent;
            KeyGroupSrcInfo srcInfo;
            JObjectArray keyActionMapList;
            std::map<int, keyActionMap> localMaps;

            // Check for attempts to interrupt sequences that are already in-progress
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);

                if ((m_ramsOperatingMode != RAMS_OP_MODE_NONE) && (m_irdbLoadState > IRDB_LOAD_STATE_WRITTEN))
                {
                    LOGERR("ERROR - Operation in progress - OperatingMode: %d, LoadState: %d!",
                           m_ramsOperatingMode, m_irdbLoadState);
                    response["status_code"] = (int)STATUS_INVALID_STATE;
                    returnResponse(false);
                }
            }

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") ||
                !parameters.HasLabel("keymapType") || !parameters.HasLabel("keyActionMapping"))
            {
                // There are either NO parameters, or no 'deviceID', no 'keymapType'. or no 'keyActionMapping'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID', a 'keymapType', and a 'keyActionMapping' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
                // Check to see if this remote will accept this operation.
                {
                    bool bHas5DCPresent = false;
                    bool bSupports5DC = false;
                    string remoteType;
                    // Get the controller's RAMS-related status
                    if (!m_helper.getControllerByID(deviceID, remoteType, bHas5DCPresent, bSupports5DC))
                    {
                        LOGERR("ERROR - Can't get controller(%d) info.", deviceID);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    // If the controller involved has a 5-Digit Code already set,
                    // meaning this method can't succeed.  Fail now.
                    if (bHas5DCPresent)
                    {
#ifdef RAMS_USE_FORCE_DOWNLOAD
                        // Don't fail here, because we will force the download to the controller.
                        LOGWARN("callMethod(%s): WARNING - controller(%d) has a 5-Digit Code set, download will be forced!", __FUNCTION__, deviceID);
#else
                        // If the controller involved has a 5-Digit Code already set,
                        // clearKeyActionMapping() won't work.  Fail now.
                        LOGERR("ERROR - controller(%d) has a 5-Digit Code set, which blocks this method!!", deviceID);
                        response["status_code"] = (int)STATUS_INVALID_STATE;
                        returnResponse(false);
#endif
                    }
                    else
                    {
                        // Save the 5-Digit Code booleans as state information.
                        std::lock_guard<std::mutex> guard(m_stateMutex);
                        m_lastSetHas5DCPresent = bHas5DCPresent;
                        m_lastSetSupports5DC = bSupports5DC;
                    }
                }
            }
            // Get the keymapType from the parameters
            paramKey = "keymapType";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 1))
                {
                    // The keymapType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keymapType' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keymapType = value;
                LOGINFO("keymapType passed in is %d.", keymapType);
            }
            // Get the keyActionMapping array from the parameters
            paramKey = "keyActionMapping";
            if (parameters.HasLabel(paramKey))
            {
                JsonArray value;
                getArrayParameter(paramKey, value);
                if ((value.Length() < 1) || (value.Length() > 7))
                {
                    // The keyActionMapping array size is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keyActionMapping' array parameter value - size is %d!", value.Length());
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keyActionMapList = value;
                numMaps = (int)keyActionMapList.Length();
                LOGINFO("size of keyActionMapping array passed in is %d.", numMaps);
            }
            // Extract and convert the JsonObjects in the array to the keyActionMap class
            if (numMaps > 0)
            {
                for (int i = 0; i < numMaps; i++)
                {
                    keyActionMap actionMap;
                    JsonObject jsonActionMap;
                    int irCodeCount = 0;

                    try
                    {
                        // Get a single JSON keyActionMap from the array of them
                        jsonActionMap = keyActionMapList[i];
                    }
                    catch (...)
                    {
                        LOGERR("ERROR - exception converting keyActionMapList[%d] to object!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    // Get the keyName
                    paramKey = "keyName";
                    if (!jsonActionMap.HasLabel(paramKey))
                    {
                        LOGERR("ERROR - keyActionMap[%d]: missing keyName parameter!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    else
                    {
                        int value = 0;
                        getNumberParameterObject(jsonActionMap, paramKey, value);
                        if ((value < 0) || (value > 255))
                        {
                            // Value is out of range. We will treat this as a fatal error. Exit now.
                            LOGERR("ERROR - Bad jsonActionMap 'keyName' value: %d!", value);
                            response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                            returnResponse(false);
                        }
                        actionMap.keyName = value;
                        LOGWARN("keyActionMap[%d]: keyName value: 0x%02X.", i, actionMap.keyName);
                    }
                    // Get the rfKeyCode
                    int inRFKeyCode = -1;
                    paramKey = "rfKeyCode";
                    if (jsonActionMap.HasLabel(paramKey))
                    {
                        getNumberParameterObject(jsonActionMap, paramKey, inRFKeyCode);
                    }
                    // Lookup the rfKeyCode ourselves, using the keyName value
                    actionMap.rfKeyCode = m_helper.lookupRFKey(actionMap.keyName);
                    if ((actionMap.rfKeyCode < 0) || (actionMap.rfKeyCode > 255))
                    {
                        // rfKeyCode is out of range. We will treat this as a fatal error. Exit now.
                        LOGERR("ERROR - Bad lookup 'rfKeyCode' value: %d!", actionMap.rfKeyCode);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    LOGWARN("keyActionMap[%d]: rfKeyCode value: 0x%02X, input rfKeyCode: 0x%02X.", i, actionMap.rfKeyCode, inRFKeyCode);

                    // Get the tvIRKeyCode IR waveform data
                    paramKey = "tvIRKeyCode";
                    if (!jsonActionMap.HasLabel(paramKey))
                    {
                        LOGERR("ERROR - keyActionMap[%d]: missing tvIRKeyCode parameter!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    else
                    {
                        JsonArray irData;
                        getArrayParameterObject(jsonActionMap, paramKey, irData);
                        if (irData.Length() > CONTROLMGR_MAX_IR_DATA_SIZE)
                        {
                            // The tvIRKeyCode array size is not in range.  We will treat this as a fatal error. Exit now.
                            LOGERR("ERROR - Bad 'tvIRKeyCode' array parameter value - size is %d!", irData.Length());
                            response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                            returnResponse(false);
                        }

                        LOGWARN("keyActionMap[%d]: tvIRKeyCode size: %d.", i, (int)irData.Length());

                        actionMap.tvIRData.clear();
                        if (irData.Length() > 0)
                        {
                            LOGWARN("keyActionMap[%d]: tvIRKeyCode element type: %d.", i, (int)irData[0].Content());
                            // Load the byte vector from the number JsonArray
                            for (int j = 0; j < irData.Length(); j++)
                            {
                                int byteval;
                                try
                                {
                                    byteval = irData[j].Number();
                                }
                                catch (...)
                                {
                                    LOGERR("ERROR - exception converting tvIRKeyCode irData[%d] to number!", j);
                                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                    returnResponse(false);
                                }
                                if ((byteval < 0) || (byteval > 255))
                                {
                                    LOGERR("ERROR - tvIRKeyCode data byteval %d is out of range!", byteval);
                                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                    returnResponse(false);
                                }
                                actionMap.tvIRData.push_back((unsigned char)(byteval & 0xFF));
                            }
                            keysPresent.tv.setKey(actionMap.rfKeyCode);
                            LOGWARN("keyActionMap[%d]: tvIRKeyCode size: %d.", i, actionMap.tvIRData.size());
                            if (actionMap.tvIRData.size() > 12)
                            {
                                LOGWARN("tvIRKeyCode bytes - 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                                        (unsigned char)actionMap.tvIRData.at(0), (unsigned char)actionMap.tvIRData.at(1), (unsigned char)actionMap.tvIRData.at(2),
                                        (unsigned char)actionMap.tvIRData.at(3), (unsigned char)actionMap.tvIRData.at(4), (unsigned char)actionMap.tvIRData.at(5),
                                        (unsigned char)actionMap.tvIRData.at(6), (unsigned char)actionMap.tvIRData.at(7), (unsigned char)actionMap.tvIRData.at(8),
                                        (unsigned char)actionMap.tvIRData.at(9), (unsigned char)actionMap.tvIRData.at(10), (unsigned char)actionMap.tvIRData.at(11));
                            }
                            irCodeCount++;
                        }
                    }

                    // Get the avrIRKeyCode IR waveform data
                    paramKey = "avrIRKeyCode";
                    if (!jsonActionMap.HasLabel(paramKey))
                    {
                        LOGERR("ERROR - keyActionMap[%d]: missing avrIRKeyCode parameter!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    else
                    {
                        JsonArray irData;
                        getArrayParameterObject(jsonActionMap, paramKey, irData);
                        if (irData.Length() > CONTROLMGR_MAX_IR_DATA_SIZE)
                        {
                            // The avrIRKeyCode array size is not in range.  We will treat this as a fatal error. Exit now.
                            LOGERR("ERROR - Bad 'avrIRKeyCode' array parameter value - size is %d!", irData.Length());
                            response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                            returnResponse(false);
                        }

                        LOGWARN("keyActionMap[%d]: avrIRKeyCode size: %d.", i, (int)irData.Length());

                        actionMap.avrIRData.clear();
                        if (irData.Length() > 0)
                        {
                            LOGWARN("keyActionMap[%d]: avrIRKeyCode element type: %d.", i, (int)irData[0].Content());
                            // Load the byte vector from the number JsonArray
                            for (int j = 0; j < irData.Length(); j++)
                            {
                                int byteval;
                                try
                                {
                                    byteval = irData[j].Number();
                                }
                                catch (...)
                                {
                                    LOGERR("ERROR - exception converting avrIRKeyCode irData[%d] to number!", j);
                                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                    returnResponse(false);
                                }
                                if ((byteval < 0) || (byteval > 255))
                                {
                                    LOGERR("ERROR - avrIRKeyCode data byteval %d is out of range!", byteval);
                                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                    returnResponse(false);
                                }
                                actionMap.avrIRData.push_back((unsigned char)(byteval & 0xFF));
                            }
                            keysPresent.avr.setKey(actionMap.rfKeyCode);
                            LOGWARN("keyActionMap[%d]: avrIRKeyCode size: %d.", i, actionMap.avrIRData.size());
                            if (actionMap.avrIRData.size() > 12)
                            {
                                LOGWARN("avrIRKeyCode bytes - 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                                        (unsigned char)actionMap.avrIRData.at(0), (unsigned char)actionMap.avrIRData.at(1), (unsigned char)actionMap.avrIRData.at(2),
                                        (unsigned char)actionMap.avrIRData.at(3), (unsigned char)actionMap.avrIRData.at(4), (unsigned char)actionMap.avrIRData.at(5),
                                        (unsigned char)actionMap.avrIRData.at(6), (unsigned char)actionMap.avrIRData.at(7), (unsigned char)actionMap.avrIRData.at(8),
                                        (unsigned char)actionMap.avrIRData.at(9), (unsigned char)actionMap.avrIRData.at(10), (unsigned char)actionMap.avrIRData.at(11));
                            }
                            irCodeCount++;
                        }
                    }

                    if (irCodeCount < 1)
                    {
                        LOGERR("ERROR - keyActionMap[%d]: empty tvIRKeyCode AND empty avrIRKeyCode!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    // Store the keyActionMap in the local map. Use automatic allocation in the map, so there are no leaks.
                    localMaps[actionMap.rfKeyCode] = actionMap;
                }
                LOGWARN("localMaps is finished, size: %d", (int)localMaps.size());
                if (numMaps != (int)localMaps.size())
                {
                    LOGERR("ERROR - maps conversion failure - input numMaps: %d, output localMaps size: %d!",
                           numMaps, localMaps.size());
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                else
                {
                    // Also check the localMaps for coherence, before we try to use them.
                    int count = 0;
                    for (int i = 0; i < supported_ked_keynames_size; i++)
                    {
                        int rfKeyCode = m_helper.lookupRFKey(supported_ked_keynames[i]);
                        bool bHasMap = true;
                        keyActionMap actionMap;

                        try
                        {
                            actionMap = localMaps.at(rfKeyCode);
                        }
                        catch (...)
                        {
                            LOGWARN("WARNING - No localMaps entry for rfKeyCode: %d", rfKeyCode);
                            bHasMap = false;
                        }
                        if (bHasMap)
                        {
                            if (actionMap.rfKeyCode != rfKeyCode)
                            {
                                LOGERR("ERROR - For rfKeyCode 0x%02X, found actionMap with rfKeyCode 0x%02X!",
                                       rfKeyCode, actionMap.rfKeyCode);
                                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                returnResponse(false);
                            }
                            if ((actionMap.tvIRData.size() <= 1) && (actionMap.avrIRData.size() <= 1))
                            {
                                LOGERR("ERROR - For rfKeyCode 0x%02X, found actionMap with no IR data!", rfKeyCode);
                                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                                returnResponse(false);
                            }
                            count++;
                        }
                    }
                    if (numMaps != count)
                    {
                        LOGERR("ERROR - localMaps count wrong - input numMaps: %d, output localMaps count: %d!",
                               numMaps, count);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                }
            }

            // Decide on the TV/AVR key group assignments, based on the 7 original IR codesets present
            if (!setKeyGroups(srcInfo, const_cast<KeyPresenceFlags&>(keysPresent)))
            {
                LOGERR("ERROR - incoherent IR codesets!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }


            // Modify the IRRF database entries
            status_code = (setKeyActionMapping(deviceID, keymapType, localMaps, const_cast<KeyGroupSrcInfo&>(srcInfo)) ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }  // end of setKeyActionMappingWrapper()


        uint32_t RemoteActionMapping::clearKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int keymapType = -1;
            int keyNames[supported_ked_keynames_size];
            int numNames = 0;
            JsonArray keyNamesList;
            RFKeyFlags rfKeyFlags;

            // Check for attempts to interrupt sequences that are already in-progress
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);

                if ((m_ramsOperatingMode != RAMS_OP_MODE_NONE) && (m_irdbLoadState > IRDB_LOAD_STATE_WRITTEN))
                {
                    LOGERR("ERROR - Operation in progress - OperatingMode: %d, LoadState: %d!",
                           m_ramsOperatingMode, m_irdbLoadState);
                    response["status_code"] = (int)STATUS_INVALID_STATE;
                    returnResponse(false);
                }
            }

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") ||
                !parameters.HasLabel("keymapType") || !parameters.HasLabel("keyNames"))
            {
                // There are either NO parameters, or no 'deviceID', no 'keymapType'. or no 'keyNames'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID', a 'keymapType', and a 'keyNames' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
                // Check to see if this remote will accept this operation.
                {
                    bool bHas5DCPresent = false;
                    bool bSupports5DC = false;
                    string remoteType;
                    // Get the controller's RAMS-related status
                    if (!m_helper.getControllerByID(deviceID, remoteType, bHas5DCPresent, bSupports5DC))
                    {
                        LOGERR("ERROR - Can't get controller(%d) info.", deviceID);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    // If the controller involved has a 5-Digit Code already set,
                    // meaning this method can't succeed.  Fail now.
                    if (bHas5DCPresent)
                    {
#ifdef RAMS_USE_FORCE_DOWNLOAD
                        // Don't fail here, because we will force the download to the controller.
                        LOGWARN("callMethod(%s): WARNING - controller(%d) has a 5-Digit Code set, download will be forced!", __FUNCTION__, deviceID);
#else
                        // If the controller involved has a 5-Digit Code already set,
                        // setKeyActionMapping() won't work.  Fail now.
                        LOGERR("ERROR - controller(%d) has a 5-Digit Code set, which blocks this method!!", deviceID);
                        response["status_code"] = (int)STATUS_INVALID_STATE;
                        returnResponse(false);
#endif
                    }
                    else
                    {
                        // Save the 5-Digit Code booleans as state information.
                        std::lock_guard<std::mutex> guard(m_stateMutex);
                        m_lastSetHas5DCPresent = bHas5DCPresent;
                        m_lastSetSupports5DC = bSupports5DC;
                    }
                }
            }
            // Get the keymapType from the parameters
            paramKey = "keymapType";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 1))
                {
                    // The keymapType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keymapType' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keymapType = value;
                LOGINFO("keymapType passed in is %d.", keymapType);
            }
            // Get the keyNames array from the parameters
            paramKey = "keyNames";
            if (parameters.HasLabel(paramKey))
            {
                JsonArray value;
                getArrayParameter(paramKey, value);
                if ((value.Length() < 1) || (value.Length() > supported_ked_keynames_size))
                {
                    // The keyNames array size is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keyNames' array parameter value - size is %d!", value.Length());
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keyNamesList = value;
                numNames = (int)keyNamesList.Length();
                LOGINFO("size of keyNames array passed in is %d.", numNames);
                LOGWARN("keyNamesList element type: %d.", (int)keyNamesList[0].Content());
            }
            if (numNames > 0)
            {
                int rfKeyCode = -1;
                memset((void*)keyNames, 0, sizeof(keyNames));
                for (int i = 0; i < numNames; i++)
                {
                    try
                    {
                       keyNames[i] = keyNamesList[i].Number();
                    }
                    catch (...)
                    {
                        LOGERR("ERROR - exception converting keyNamesList[%d] to number!", i);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    // Check keyName validity, and build keyflags
                    rfKeyCode = m_helper.lookupRFKey(keyNames[i]);
                    if (rfKeyCode < 0)
                    {
                        LOGERR("ERROR - invalid keyName value: 0x%02X!", keyNames[i]);
                        response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                        returnResponse(false);
                    }
                    else
                    {
                        if (rfKeyFlags.isSet(rfKeyCode))
                        {
                            LOGERR("ERROR - duplicate keyName: 0x%02X!", keyNames[i]);
                            response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                            returnResponse(false);
                        }
                        else
                        {
                            rfKeyFlags.setKey(rfKeyCode);
                            LOGWARN("keyNames value at %d is: 0x%08X.", i, keyNames[i]);
                        }
                    }
                }
            }

            // Ensure that keynames groups are coherent.
            if (!checkClearList(rfKeyFlags, keyNames, &numNames))
            {
                LOGERR("ERROR - incoherent keyNames!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            status_code = (clearKeyActionMapping(deviceID, keymapType, keyNames, numNames) ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }  // end of clearKeyActionMappingWrapper

        uint32_t RemoteActionMapping::getFullKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int keymapType = -1;
            JsonArray mappings;

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") || !parameters.HasLabel("keymapType"))
            {
                // There are either NO parameters, or no 'deviceID', or 'keymapType'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID' and 'keymapType' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
            }
            // Get the keymapType from the parameters
            paramKey = "keymapType";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 1))
                {
                    // The keymapType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keymapType' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keymapType = value;
                LOGINFO("keymapType passed in is %d.", keymapType);
            }

            mappings = getFullKeyActionMapping(deviceID, keymapType);
            response["keyMappings"] = mappings;

            status_code = ((mappings.Length() > 0) ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t RemoteActionMapping::getSingleKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int keymapType = -1;
            int keyName = -1;
            JsonObject mapping;

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") ||
                !parameters.HasLabel("keymapType") || !parameters.HasLabel("keyName"))
            {
                // There are either NO parameters, or no 'deviceID', 'keymapType', or 'keyName'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID', 'keymapType', and 'keyName' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
            }
            // Get the keymapType from the parameters
            paramKey = "keymapType";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 1))
                {
                    // The keymapType value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keymapType' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keymapType = value;
                LOGINFO("keymapType passed in is %d.", keymapType);
            }
            // Get the keyName from the parameters
            paramKey = "keyName";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if (value <= 0)
                {
                    // The keyName value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'keyName' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                keyName = value;
                LOGINFO("keyName passed in is %d.", keyName);
            }

            mapping = getSingleKeyActionMapping(deviceID, keymapType, keyName);
            response["keyMapping"] = JsonValue(mapping);

            status_code = (mapping.IsSet() ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t RemoteActionMapping::cancelCodeDownloadWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;

            // Check for attempts to cancel sequences that are already in-progress (i.e. too late, can't be canceled)
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);

                if ((m_ramsOperatingMode != RAMS_OP_MODE_NONE) && (m_irdbLoadState > IRDB_LOAD_STATE_WRITTEN))
                {
                    LOGERR("ERROR - Too late to cancel operation - OperatingMode: %d, LoadState: %d!",
                           m_ramsOperatingMode, m_irdbLoadState);
                    response["status_code"] = (int)STATUS_INVALID_STATE;
                    returnResponse(false);
                }
            }

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID"))
            {
                // There are either NO parameters, or no 'deviceID'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
            }

            status_code = (cancelCodeDownload(deviceID) ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }

        uint32_t RemoteActionMapping::setFiveDigitCodeWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            StatusCode status_code = STATUS_OK;
            const char* paramKey = NULL;
            int deviceID = -1;
            int tvFiveDigitCode = -1;
            int avrFiveDigitCode = -1;

            // Check for attempts to interrupt sequences that are already in-progress
            {
                std::lock_guard<std::mutex> guard(m_stateMutex);

                if ((m_ramsOperatingMode != RAMS_OP_MODE_NONE) && (m_irdbLoadState > IRDB_LOAD_STATE_WRITTEN))
                {
                    LOGERR("ERROR - Operation in progress - OperatingMode: %d, LoadState: %d!",
                           m_ramsOperatingMode, m_irdbLoadState);
                    response["status_code"] = (int)STATUS_INVALID_STATE;
                    returnResponse(false);
                }
            }

            if (!parameters.IsSet() || !parameters.HasLabel("deviceID") ||
                !parameters.HasLabel("tvFiveDigitCode") || !parameters.HasLabel("avrFiveDigitCode"))
            {
                // There are either NO parameters, or no 'deviceID', no 'tvFiveDigitCode'. or no 'avrFiveDigitCode'.
                // We will treat this as a fatal error. Exit now.
                LOGERR("ERROR - this method requires a 'deviceID', a 'tvFiveDigitCode', and an avrFiveDigitCode' parameter!");
                response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                returnResponse(false);
            }

            // Get the deviceID from the parameters
            paramKey = "deviceID";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value <= 0) || (value > CTRLM_MAIN_MAX_BOUND_CONTROLLERS))
                {
                    // The deviceID value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'deviceID' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                deviceID = value;
                LOGINFO("deviceID passed in is %d.", deviceID);
            }
            // Get the tvFiveDigitCode from the parameters
            paramKey = "tvFiveDigitCode";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 99999))
                {
                    // The tvFiveDigitCode value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'tvFiveDigitCode' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                tvFiveDigitCode = value;
                LOGINFO("tvFiveDigitCode passed in is %d.", tvFiveDigitCode);
            }
            // Get the avrFiveDigitCode from the parameters
            paramKey = "avrFiveDigitCode";
            if (parameters.HasLabel(paramKey))
            {
                int value = 0;
                getNumberParameter(paramKey, value);
                if ((value < 0) || (value > 99999))
                {
                    // The avrFiveDigitCode value is not in range.  We will treat this as a fatal error. Exit now.
                    LOGERR("ERROR - Bad 'avrFiveDigitCode' parameter value: %d!", value);
                    response["status_code"] = (int)STATUS_INVALID_ARGUMENT;
                    returnResponse(false);
                }
                avrFiveDigitCode = value;
                LOGINFO("avrFiveDigitCode passed in is %d.", avrFiveDigitCode);
            }

            status_code = (setFiveDigitCode(deviceID, tvFiveDigitCode, avrFiveDigitCode) ? STATUS_OK : STATUS_FAILURE);

            response["status_code"] = (int)status_code;
            returnResponse(status_code == STATUS_OK);
        }
        // End methods


        // Begin events
        void RemoteActionMapping::onIRCodeLoad(int deviceID, int_vector_t& rfKeyCodes, int status)
        {
            JsonObject params;
            JsonArray keyNames;
            JsonArray keyCodes;

            // Build the KED keyNames list from the rfKeyCodes list
            for (std::size_t i = 0; i < rfKeyCodes.size(); i++)
            {
                int name = m_helper.lookupKeyname(rfKeyCodes[i]);

                keyCodes.Add(JsonValue(rfKeyCodes[i]));
                keyNames.Add(JsonValue(name));

                LOGINFO("Sending rfKeyCode: 0x%02X, keyname: 0x%02X.",
                        (unsigned)rfKeyCodes[i], (unsigned)name);
            }

            params["deviceID"] = JsonValue(deviceID);
            params["keyNames"] = keyNames;
            params["rfKeyCodes"] = keyCodes;
            params["loadStatus"] = JsonValue(status);

            sendNotify("onIRCodeLoad", params);
        }

        void RemoteActionMapping::onFiveDigitCodeLoad(int deviceID, int tvStatus, int avrStatus)
        {
            JsonObject params;

            LOGINFO("Sending deviceID: %d, tvStatus: 0x%02X, avrStatus: 0x%02X.",
                    deviceID, tvStatus, avrStatus);

            params["deviceID"] = JsonValue(deviceID);
            params["tvLoadStatus"] = JsonValue(tvStatus);
            params["avrLoadStatus"] = JsonValue(avrStatus);

            sendNotify("onFiveDigitCodeLoad", params);
        }
        // End events


        // Begin private method implementations
        int RemoteActionMapping::getLastUsedDeviceID(std::string& remoteType, bool& bFiveDigitCodeSet, bool& bFiveDigitCodeSupported)
        {
            int deviceID = 1;

            deviceID = m_helper.getLastUsedDeviceID(remoteType, bFiveDigitCodeSet, bFiveDigitCodeSupported);

            return deviceID;
        }

        JsonArray RemoteActionMapping::getKeymap(int deviceID, int keymapType)
        {
            UNUSED(deviceID);
            UNUSED(keymapType);

            JsonArray keyNames;

            // Fill in the array of integer "KED_" values (hard-coded list)
            for (int i = 0; i < supported_ked_keynames_size; i++)
            {
                keyNames.Add(JsonValue(supported_ked_keynames[i]));
            }

            return keyNames;
        }

        bool RemoteActionMapping::setKeyActionMapping(int deviceID, int keymapType, std::map<int, keyActionMap>& localActionMaps, const KeyGroupSrcInfo& srcInfo)
        {
            keyActionMap actionMap;
            keyActionMap altActionMap;
            int rfKeyCode = -1;
            bool status = false;
            bool success = true;

            // Create an "alternate" actionMap for the discrete power entries, if needed.
            if ((srcInfo.groupDiscretePower == KEY_GROUP_SRC_TV_PWR_CROSS) ||
                (srcInfo.groupDiscretePower == KEY_GROUP_SRC_AVR_PWR_CROSS))
            {
                // Use the toggle power IR data as the alternate.
                bool bHasMap = true;
                try
                {
                    actionMap = localActionMaps.at(MSO_RFKEY_PWR_TOGGLE);
                }
                catch (...)
                {
                    LOGERR("LOGIC ERROR - power crossover needed, but no toggle power actionMap available!!");
                    bHasMap = false;
                }
                if (bHasMap)
                {
                    altActionMap.keyName = KED_UNDEFINEDKEY;    // Flag that the map is an alternate.
                    altActionMap.rfKeyCode = 0;                 // Set this to the on/off discrete code later
                    altActionMap.tvIRData = actionMap.tvIRData;
                    altActionMap.avrIRData = actionMap.avrIRData;
                    LOGWARN("Performing a power crossover.");
                }
            }

            // For every one of the original 7 keys, either set the RIB entry from the ActionMap, or clear the RIB entry.
            // Use any power-related actionMaps that we got to independently set or clear the separate power RIB entries.
            for (int i = 0; i < supported_ked_keynames_size; i++)
            {
                rfKeyCode = m_helper.lookupRFKey(supported_ked_keynames[i]);
                if (rfKeyCode > 0)
                {
                    bool bHasMap = true;
                    try
                    {
                        actionMap = localActionMaps.at(rfKeyCode);
                    }
                    catch (...)
                    {
                        bHasMap = false;
                    }
                    // Handle the separate device power IRRFDB entries
                    if ((rfKeyCode == MSO_RFKEY_PWR_ON) || (rfKeyCode == MSO_RFKEY_PWR_OFF) || (rfKeyCode == MSO_RFKEY_PWR_TOGGLE))
                    {
                        if (bHasMap)
                        {
                            status = m_helper.setDevicePower(deviceID, keymapType, actionMap);
                            if (status)
                            {
                                LOGWARN("setDevicePower success for rfKeyCode: 0x%02X", rfKeyCode);
                            }
                            else
                            {
                                LOGERR("ERROR - separate power set failure for rfKeyCode: 0x%02X.", rfKeyCode);
                                success = false;
                            }
                        }
                        else
                        {
                            status = m_helper.clearDevicePower(deviceID, keymapType, rfKeyCode);
                            if (status)
                            {
                                LOGWARN("clearDevicePower success for rfKeyCode: 0x%02X", rfKeyCode);
                            }
                            else
                            {
                                LOGERR("ERROR - separate power clear failure for rfKeyCode: 0x%02X.", rfKeyCode);
                                success = false;
                            }
                        }
                    }
                    // Now take care of the original 7 IRRFDB entries.
                    // If we have decided to use an alternate source for the discrete power slots,
                    // disregard any actual discrete power actionMap that may exist, and use the alternate.
                    if ((altActionMap.keyName == KED_UNDEFINEDKEY) &&
                        ((rfKeyCode == MSO_RFKEY_PWR_ON) || (rfKeyCode == MSO_RFKEY_PWR_OFF)))
                    {
                        // We MUST set alternate IR codes into the original discrete power slots, if we have them.
                        altActionMap.rfKeyCode = rfKeyCode;
                        status = m_helper.setKeyActionMap(deviceID, keymapType, altActionMap, srcInfo);
                        if (status)
                        {
                            LOGWARN("setKeyActionMap success with alternate for rfKeyCode: 0x%02X", rfKeyCode);
                        }
                        else
                        {
                            LOGERR("ERROR - alternate set failure for rfKeyCode: 0x%02X.", rfKeyCode);
                            success = false;
                        }
                    }
                    else
                    {
                        // Use the actionMaps to set or clear one of the the original 7 IRRFDB slots.
                        if (bHasMap)
                        {
                            // We have an ActionMap for this rfKeyCode, so we should be able to set the RIB from it.
                            status = m_helper.setKeyActionMap(deviceID, keymapType, actionMap, srcInfo);
                            if (status)
                            {
                                LOGWARN("setKeyActionMap success for rfKeyCode: 0x%02X", rfKeyCode);
                            }
                            else
                            {
                                LOGERR("ERROR - set failure for rfKeyCode: 0x%02X.", rfKeyCode);
                                success = false;
                            }
                        }
                        else
                        {
                            // We don't have an ActionMap for this rfKeyCode. Clear the IRRFDB RIB entry back to the "not programmed" state.
                            status = m_helper.clearKeyActionMap(deviceID, keymapType, supported_ked_keynames[i]);
                            if (status)
                            {
                                LOGWARN("Auto-cleared rfKeyCode 0x%02X", rfKeyCode);
                            }
                            else
                            {
                                LOGERR("ERROR - auto-clear failure at rfKeyCode: 0x%02X!", rfKeyCode);
                                success = false;
                            }
                        }
                    }
                }
                else
                {
                    LOGERR("LOGIC ERROR - keyName 0x%02X has no matching rfKeyCode?!", supported_ked_keynames[i]);
                }
            }

            if (success)
            {
                {
                    std::lock_guard<std::mutex> guard(m_stateMutex);
                    // Remember the ID of the remote that last changed the RIB key codes
                    m_lastSetRemoteID = deviceID;
                    if (m_irdbLoadState != IRDB_LOAD_STATE_NONE)
                    {
                        LOGWARN("WARNING - LoadState: %d, should be %d!", m_irdbLoadState, IRDB_LOAD_STATE_NONE);
                    }
                    if (m_ramsOperatingMode != RAMS_OP_MODE_NONE)
                    {
                        LOGWARN("WARNING - OperatingMode: %d, should be %d!", m_ramsOperatingMode, RAMS_OP_MODE_NONE);
                    }
                    m_irdbLoadState = IRDB_LOAD_STATE_WRITTEN;
                    m_ramsOperatingMode = RAMS_OP_MODE_IRRF_DATABASE;
                }
                // Now that the individual key codes are set in the RIB,
                // enable Download from IRRFDB on OK Keypress mode, so the remote reads the RIB IRRF Database
                m_helper.setIRDBDownloadFlag(deviceID, true);
                // Initialize the controller load progress state.
                m_readProgress.clear();
            }

            return success;
        }  // end of setKeyActionMapping()

        bool RemoteActionMapping::clearKeyActionMapping(int deviceID, int keymapType, int* keyNames, int numNames)
        {
            bool result = false;
            int rfKeyCode = -1;

            if ((deviceID < 1) || (keyNames == NULL) || (numNames < 1))
            {
                LOGERR("ERROR - Bad argument.");
                return result;
            }

            for (int i = 0; i < numNames; i++)
            {
                rfKeyCode = m_helper.lookupRFKey(keyNames[i]);
                if (rfKeyCode < 0)
                {
                    LOGERR("LOGIC ERROR - bad key lookup at keyName %d, index %d!", keyNames[i], i);
                    break;
                }
                result = m_helper.clearKeyActionMap(deviceID, keymapType, keyNames[i]);
                if (!result)
                {
                    LOGERR("ERROR - failure at keyName %d, index %d!", keyNames[i], i);
                    break;
                }
            }

            if (result)
            {
                // Set up state machine
                {
                    std::lock_guard<std::mutex> guard(m_stateMutex);
                    // Remember the ID of the remote that last changed the RIB key codes
                    m_lastSetRemoteID = deviceID;
                    if (m_irdbLoadState != IRDB_LOAD_STATE_NONE)
                    {
                        LOGWARN("WARNING - LoadState: %d, should be %d!", m_irdbLoadState, IRDB_LOAD_STATE_NONE);
                    }
                    if (m_ramsOperatingMode != RAMS_OP_MODE_NONE)
                    {
                        LOGWARN("WARNING - OperatingMode: %d, should be %d!", m_ramsOperatingMode, RAMS_OP_MODE_NONE);
                    }
                    m_irdbLoadState = IRDB_LOAD_STATE_WRITTEN;
                    m_ramsOperatingMode = RAMS_OP_MODE_IRRF_DATABASE;
                }
                // Now that the individual key codes are set in the RIB,
                // enable Download from IRRDB on OK Keypress mode, so the remote reads the RIB IRRF Database
                m_helper.setIRDBDownloadFlag(deviceID, true);
                // Initialize the controller load progress state.
                m_readProgress.clear();
            }

            return result;
        }

        RemoteActionMapping::JObjectArray RemoteActionMapping::getFullKeyActionMapping(int deviceID, int keymapType)
        {
            UNUSED(keymapType);
            JObjectArray keyMappings;

            if (deviceID < 1)
            {
                LOGERR("ERROR - Bad deviceID argument!");
                return keyMappings;
            }

            for (int i = 0; i < supported_ked_keynames_size; i++)
            {
                JsonObject keyMapping;
                keyActionMap actionMap;
                actionMap.keyName = supported_ked_keynames[i];
                if (!m_helper.getKeyActionMap(deviceID, keymapType, actionMap))
                {
                    LOGERR("ERROR - bad keyName: 0x%02X.", actionMap.keyName);
                    break;
                }
                else
                {
                    keyMapping["keyName"] = JsonValue(actionMap.keyName);
                    keyMapping["rfKeyCode"] = JsonValue(actionMap.rfKeyCode);
                    // Convert the IR data blobs from byte vectors to JsonArrays
                    JsonArray tvData;
                    for (std::size_t j = 0; j < actionMap.tvIRData.size(); j++)
                    {
                        tvData.Add(JsonValue((int)actionMap.tvIRData[j]));
                    }
                    keyMapping["tvIRKeyCode"] = JsonValue(tvData);

                    JsonArray avrData;
                    for (std::size_t j = 0; j < actionMap.avrIRData.size(); j++)
                    {
                        avrData.Add(JsonValue((int)actionMap.avrIRData[j]));
                    }
                    keyMapping["avrIRKeyCode"] = JsonValue(avrData);
                }
                keyMappings.Add(keyMapping);
            }

            return keyMappings;
        }

        JsonObject RemoteActionMapping::getSingleKeyActionMapping(int deviceID, int keymapType, int keyName)
        {
            UNUSED(keymapType);
            JsonObject      keyMapping;
            keyActionMap    actionMap;

            if ((deviceID < 1) || (keyName < 0) || (keyName > 255))
            {
                LOGERR("ERROR - Bad argument!");
                return keyMapping;
            }

            actionMap.keyName = keyName;
            if (!m_helper.getKeyActionMap(deviceID, keymapType, actionMap))
            {
                LOGERR("ERROR - bad keyName: 0x%02X.", keyName);
            }
            else
            {
                // Return a single keyActionMap (as a JsonObject)
                keyMapping["keyName"] = JsonValue(actionMap.keyName);
                keyMapping["rfKeyCode"] = JsonValue(actionMap.rfKeyCode);
                // Convert the IR data blobs from byte vectors to JsonArrays
                JsonArray tvData;
                for (std::size_t j = 0; j < actionMap.tvIRData.size(); j++)
                {
                    tvData.Add(JsonValue((int)actionMap.tvIRData[j]));
                }
                keyMapping["tvIRKeyCode"] = JsonValue(tvData);

                JsonArray avrData;
                for (std::size_t j = 0; j < actionMap.avrIRData.size(); j++)
                {
                    avrData.Add(JsonValue((int)actionMap.avrIRData[j]));
                }
                keyMapping["avrIRKeyCode"] = JsonValue(avrData);
            }

            return keyMapping;
        }

        bool RemoteActionMapping::cancelCodeDownload(int deviceID)
        {
            if ((m_lastSetRemoteID > 0) && (m_lastSetRemoteID != deviceID))
            {
                // FAILURE: Trying to cancel with a deviceID that doesn't match the deviceID we were primed with!?!
                LOGERR("ERROR - deviceID: %d, doesn't match 'set' deviceID: %d!!", deviceID, m_lastSetRemoteID);
                return false;
            }
            // Cancel whatever request is set in the IRRF Status Flags first, before the OK key gets pressed!
            if (!m_helper.cancelCodeDownload(deviceID))
            {
                LOGERR("ERROR - helper failure - bad deviceID: %d?", deviceID);
                return false;
            }
            else
            {
                int opmode, loadstate;
                {
                    std::lock_guard<std::mutex> guard(m_stateMutex);

                    // Then clean up our event-driven state machine, as well.
                    m_lastSetRemoteID = -1;
                    opmode = m_ramsOperatingMode;
                    loadstate = m_irdbLoadState;
                    m_irdbLoadState = IRDB_LOAD_STATE_NONE;
                    m_ramsOperatingMode = RAMS_OP_MODE_NONE;
                    m_fiveDigitCodeMode = FIVE_DIGIT_CODE_MODE_NONE;
                }

                if (opmode == RAMS_OP_MODE_NONE)
                {
                    LOGWARN("%s: WARNING - cancelCodeDownload called with NO operation to cancel! LoadState: %d.",
                                __FUNCTION__, loadstate);
                }
                else
                {
                    LOGWARN("%s Download Cancelled: LoadState was %d!",
                            ((opmode == RAMS_OP_MODE_IRRF_DATABASE) ? "IRRF Database" : "5-Digit Code"), loadstate);
                }
            }

            return true;
        }

        bool RemoteActionMapping::setFiveDigitCode(int deviceID, int tvFiveDigitCode, int avrFiveDigitCode)
        {
            int mode = 0;
            bool success = m_helper.setFiveDigitCode(deviceID, tvFiveDigitCode, avrFiveDigitCode);

            if (success)
            {
                {
                    std::lock_guard<std::mutex> guard(m_stateMutex);

                    // Choose the enumerated FiveDigitCodeMode by testing the TV/AVR codes.
                    if ((tvFiveDigitCode == 0) && (avrFiveDigitCode == 0))
                    {
                        mode = FIVE_DIGIT_CODE_MODE_CLEAR;
                    }
                    else if ((tvFiveDigitCode > 0) && (avrFiveDigitCode == 0))
                    {
                        mode = FIVE_DIGIT_CODE_MODE_TV_SET;
                    }
                    else if ((tvFiveDigitCode == 0) && (avrFiveDigitCode > 0))
                    {
                        mode = FIVE_DIGIT_CODE_MODE_AVR_SET;
                    }
                    else if ((tvFiveDigitCode > 0) && (avrFiveDigitCode > 0))
                    {
                        mode = FIVE_DIGIT_CODE_MODE_TVAVR_SET;
                    }

                    if (mode == 0)
                    {
                        LOGERR("LOGIC ERROR: 5-digit code mode set to NONE!?!");
                        return false;
                    }
                    else
                    {
                        LOGWARN("5-digit code mode set to %d!", mode);
                    }

                    // Remember the ID of the remote that last changed the 5-digit codes, and setup the state machine.
                    m_lastSetRemoteID = deviceID;
                    if (m_irdbLoadState != IRDB_LOAD_STATE_NONE)
                    {
                        LOGWARN("WARNING - LoadState: %d, should be %d!", m_irdbLoadState, IRDB_LOAD_STATE_NONE);
                    }
                    if (m_ramsOperatingMode != RAMS_OP_MODE_NONE)
                    {
                        LOGWARN("WARNING - OperatingMode: %d, should be %d!", m_ramsOperatingMode, RAMS_OP_MODE_NONE);
                    }
                    if (m_fiveDigitCodeMode != FIVE_DIGIT_CODE_MODE_NONE)
                    {
                        LOGWARN("WARNING - FiveDigitCodeMode: %d, should be %d!", m_ramsOperatingMode, FIVE_DIGIT_CODE_MODE_NONE);
                    }
                    m_irdbLoadState = IRDB_LOAD_STATE_WRITTEN;
                    m_ramsOperatingMode = RAMS_OP_MODE_FIVE_DIGIT_CODE;
                    m_fiveDigitCodeMode = mode;
                }
                // Now that the 5-digit codes are set in the RIB, and the state machine is set up,
                // request Download from Target IRDB Status, or Clear All, on OK Keypress, so the remote sees the request
                m_helper.setFiveDigitCodeFlags(deviceID, mode);
            }

            return true;
        }
        // End private method implementations


        // Begin local private utility methods
        void RemoteActionMapping::setApiVersionNumber(unsigned int apiVersionNumber)
        {
            LOGINFO("setting version: %d", (int)apiVersionNumber);
            m_apiVersionNumber = apiVersionNumber;
        }

        bool RemoteActionMapping::setKeyGroups(KeyGroupSrcInfo& srcInfo, const KeyPresenceFlags& keyPresence)
        {
            bool bOK = true;

            // Input Select Group (straight-forward, TV has precedence)
            if (keyPresence.tv.input)
            {
                srcInfo.groupInputSelect = KEY_GROUP_SRC_TV;
            }
            else if (keyPresence.avr.input)
            {
                srcInfo.groupInputSelect = KEY_GROUP_SRC_AVR;
            }
            else
            {
                LOGWARN("NO input control possible!");
            }

            // Volume/Mute Group  (resolve VOL UP/DN and MUTE together - AVR has precedence)
            if (keyPresence.avr.vol_up && keyPresence.avr.vol_dn)
            {
                srcInfo.groupVolume = KEY_GROUP_SRC_AVR;
                if (keyPresence.avr.mute)
                {
                    srcInfo.groupMute = KEY_GROUP_SRC_AVR;
                }
                else
                {
                    LOGWARN("Choosing AVR volume control, but no Mute possible!");
                }
            }
            else if (keyPresence.tv.vol_up && keyPresence.tv.vol_dn)
            {
                srcInfo.groupVolume = KEY_GROUP_SRC_TV;
                if (keyPresence.tv.mute)
                {
                    srcInfo.groupMute = KEY_GROUP_SRC_TV;
                }
                else
                {
                    LOGWARN("Choosing TV volume control, but no Mute possible!");
                }
            }
            else
            {
                // NO volume control available, via TV or AVR!  Check for mute-only case.
                LOGWARN("NO volume control possible!");
                if (keyPresence.avr.mute)
                {
                    srcInfo.groupMute = KEY_GROUP_SRC_AVR;
                }
                else if (keyPresence.tv.mute)
                {
                    srcInfo.groupMute = KEY_GROUP_SRC_TV;
                }
                else
                {
                    // NO mute control possible!
                    LOGWARN("NO mute control possible!");
                }
            }

            // Check volume group coherence
            if ((srcInfo.groupVolume == KEY_GROUP_SRC_CLEAR) &&
                (keyPresence.tv.vol_up || keyPresence.tv.vol_dn || keyPresence.avr.vol_up || keyPresence.avr.vol_dn))
            {
                LOGERR("Incoherent volume group!!");
                bOK = false;
            }

            // Power Groups (resolve toggle and discrete together - TV has toggle precedence, AVR has discrete precedence)
            bool tvdiscrete = keyPresence.tv.pwr_on && keyPresence.tv.pwr_off;
            bool avrdiscrete = keyPresence.avr.pwr_on && keyPresence.avr.pwr_off;
            bool notvpwr = !keyPresence.tv.pwr_toggle && !tvdiscrete;
            bool noavrpwr = !keyPresence.avr.pwr_toggle && !avrdiscrete;
            // Worry about TV+AVR cases first. For the XR11's sake, put a priority on the TV using the toggle slot.
            if (keyPresence.tv.pwr_toggle && keyPresence.avr.pwr_toggle)
            {
                // Give the TV the toggle power slot, and populate both discrete slots with AVR toggle power codes.
                srcInfo.groupTogglePower = KEY_GROUP_SRC_TV;
                srcInfo.groupDiscretePower = KEY_GROUP_SRC_AVR_PWR_CROSS;
            }
            else if (keyPresence.tv.pwr_toggle && avrdiscrete)
            {
                // Again, give the TV the toggle power slot, but if there is no AVR toggle power, use AVR discrete.
                srcInfo.groupTogglePower = KEY_GROUP_SRC_TV;
                srcInfo.groupDiscretePower = KEY_GROUP_SRC_AVR;
            }
            else if (keyPresence.avr.pwr_toggle && tvdiscrete)
            {
                // At this point, in TV+AVR cases, we can't give the TV the toggle slot.
                // Our only choice, so we have power control for both, is to give the toggle slot to the AVR.
                // This will make the XR11 "TV Power" button affect the AVR, but we can't avoid that.
                // We also can't avoid using TV discrete codes, which have proved unreliable in some cases.
                srcInfo.groupTogglePower = KEY_GROUP_SRC_AVR;
                srcInfo.groupDiscretePower = KEY_GROUP_SRC_TV;
            }
            else if (tvdiscrete && avrdiscrete)
            {
                // At this point, and inside this case, implies that there are NO power toggle codes, for TV or AVR.
                // Sadly, we MUST abandon either TV or AVR power control, at least for 7-slot-IRRFDB controllers.
                // We choose to lose AVR power control, in order to keep TV power control.
                LOGWARN("Unresolvable conflict with discrete power - choosing power control over TV ONLY!");
                srcInfo.groupDiscretePower = KEY_GROUP_SRC_TV;
            }
            else if (noavrpwr)
            {
                // Handle TV with no AVR (at least for power control purposes)
                if (keyPresence.tv.pwr_toggle)
                {
                    srcInfo.groupTogglePower = KEY_GROUP_SRC_TV;
                }
                if (tvdiscrete)
                {
                    srcInfo.groupDiscretePower = KEY_GROUP_SRC_TV;
                }
                else if (keyPresence.tv.pwr_toggle)
                {
                    // Need to copy toggle power IR data into the discrete slots (for XR11 - two power buttons)
                    srcInfo.groupDiscretePower = KEY_GROUP_SRC_TV_PWR_CROSS;
                }
            }
            else if (notvpwr)
            {
                // Handle AVR with no TV (at least for power control purposes)
                if (keyPresence.avr.pwr_toggle)
                {
                    srcInfo.groupTogglePower = KEY_GROUP_SRC_AVR;
                }
                if (avrdiscrete)
                {
                    srcInfo.groupDiscretePower = KEY_GROUP_SRC_AVR;
                }
                else if (keyPresence.avr.pwr_toggle)
                {
                    // Need to copy toggle power IR data into the discrete slots (for XR11 - two power buttons)
                    srcInfo.groupDiscretePower = KEY_GROUP_SRC_AVR_PWR_CROSS;
                }
            }
            else
            {
                // Logically, if we get here, it MUST BE that there are NO TV or AVR power controls available.
                if (notvpwr && noavrpwr)
                {
                    LOGWARN("NO TV or AVR power control IR codes have been supplied!");
                }
                else
                {
                    LOGERR("LOGIC ERROR - power group decision failure - TV power: %s, AVR power: %s!!!",
                           (notvpwr ? "FALSE" : "TRUE"), (noavrpwr ? "FALSE" : "TRUE"));
                    bOK = false;
                }
            }

            // Check discrete power coherence
            if (!tvdiscrete && (keyPresence.tv.pwr_on || keyPresence.tv.pwr_off))
            {
                LOGERR("Incoherent TV discrete power!!");
                bOK = false;
            }
            if (!avrdiscrete && (keyPresence.avr.pwr_on || keyPresence.avr.pwr_off))
            {
                LOGERR("Incoherent AVR discrete power!!");
                bOK = false;
            }

            // Log the choices that were made.
            LOGWARN(" groupDiscretePower: %d", srcInfo.groupDiscretePower);
            LOGWARN("   groupTogglePower: %d", srcInfo.groupTogglePower);
            LOGWARN("        groupVolume: %d", srcInfo.groupVolume);
            LOGWARN("          groupMute: %d", srcInfo.groupMute);
            LOGWARN("   groupInputSelect: %d", srcInfo.groupInputSelect);

            return bOK;
        }

        bool RemoteActionMapping::checkClearList(RFKeyFlags& rfKeyFlags, int* keyNames, int* numNames)
        {
            bool bOK = true;

            // Check the volume group, but don't enforce mute as part of the group, for clearing.
            if (rfKeyFlags.vol_up || rfKeyFlags.vol_dn)
            {
                if (!rfKeyFlags.vol_up)
                {
                    LOGWARN("Attempt to clear volume down ONLY! - fixing...");
                    if (*numNames < supported_ked_keynames_size)
                    {
                        keyNames[*numNames] = KED_VOLUMEUP;
                        (*numNames)++;
                    }
                    else
                    {
                        LOGERR("LOGIC ERROR - no room for KED_VOLUMEUP in list!!");
                        bOK = false;
                    }
                }
                if (!rfKeyFlags.vol_dn)
                {
                    LOGWARN("Attempt to clear volume up ONLY! - fixing...");
                    if (*numNames < supported_ked_keynames_size)
                    {
                        keyNames[*numNames] = KED_VOLUMEDOWN;
                        (*numNames)++;
                    }
                    else
                    {
                        LOGERR("LOGIC ERROR - no room for KED_VOLUMEDOWN in list!!");
                        bOK = false;
                    }
                }
            }

            // Check the discrete power group
            if (rfKeyFlags.pwr_on || rfKeyFlags.pwr_off)
            {
                if (!rfKeyFlags.pwr_on)
                {
                    LOGWARN("Attempt to clear discrete power off ONLY! - fixing...");
                    if (*numNames < supported_ked_keynames_size)
                    {
                        keyNames[*numNames] = KED_DISCRETE_POWER_ON;
                        (*numNames)++;
                    }
                    else
                    {
                        LOGERR("LOGIC ERROR - no room for KED_DISCRETE_POWER_ON in list!!");
                        bOK = false;
                    }
                }
                if (!rfKeyFlags.pwr_off)
                {
                    LOGWARN("Attempt to clear discrete power on ONLY! - fixing...");
                    if (*numNames < supported_ked_keynames_size)
                    {
                        keyNames[*numNames] = KED_DISCRETE_POWER_STANDBY;
                        (*numNames)++;
                    }
                    else
                    {
                        LOGERR("LOGIC ERROR - no room for KED_DISCRETE_POWER_STANDBY in list!!");
                        bOK = false;
                    }
                }
            }

            return bOK;
        }

        bool RemoteActionMapping::checkIRRFDBReadProgress(int_vector_t& rfKeyCodes)
        {
            bool tvOK = false;
            bool avrOK = false;

            rfKeyCodes.clear();

            // Generate the rfKeyCodes read list.
            for (int i = 0; i < supported_irrfdb_slots_size; i++)
            {
                int rfKey = supported_irrfdb_slots[i];
                if (m_readProgress.getSlotRead(rfKey))
                {
                    rfKeyCodes.push_back(rfKey);
                }
            }

            // Decide whether or not all the appropriate slots have been read.
            // Start with the four non-power legacy slots.  We require that
            // the controller reads all four of those.
            if ((!m_readProgress.slot_INPUT_SELECT) ||
                (!m_readProgress.slot_VOL_PLUS) ||
                (!m_readProgress.slot_VOL_MINUS) ||
                (!m_readProgress.slot_MUTE))
            {
                return false;
            }
            // Next, check the 3 legacy power slots.  Some controllers will
            // read all three, but some controllers won't read these at all.
            if ((m_readProgress.slot_PWR_TOGGLE) &&
                (m_readProgress.slot_PWR_OFF) &&
                (m_readProgress.slot_PWR_ON))
            {
                return true;
            }
            // Finally, check the 6 separate, dedicated TV and AVR power slots.
            // We will require controllers that DON'T read the legacy power slots,
            // above, to read both the 3 TV and the 3 AVR dedicated power slots.
            if ((m_readProgress.slot_TV_PWR_TOGGLE) &&
                (m_readProgress.slot_TV_PWR_OFF) &&
                (m_readProgress.slot_TV_PWR_ON))
            {
                tvOK = true;
            }
            if ((m_readProgress.slot_AVR_PWR_TOGGLE) &&
                (m_readProgress.slot_AVR_PWR_OFF) &&
                (m_readProgress.slot_AVR_PWR_ON))
            {
                avrOK = true;
            }

            return (tvOK && avrOK);
        }

        void RemoteActionMapping::handleIRRFDBKeyRead(int deviceID, int rfKey)
        {
            int_vector_t rfKeyCodes;
            bool done = false;

            LOGWARN("target remoteId(%d) has read IRRFDB data for rfKey: 0x%02X.", deviceID, rfKey);
            // Mark this rfkey as read, in the controller read/load progress state.
            m_readProgress.setSlotRead(rfKey, true);

            // Check to see if all the appropriate slots have been read yet.
            done = checkIRRFDBReadProgress(rfKeyCodes);

            // Send the load event if the remote has read all the slots we expected
            if (done)
            {
                stopRIBLoadTimer();
                LOGWARN("Sending onIRCodeLoad event - numKeys is %d.", rfKeyCodes.size());
                m_irdbLoadState = IRDB_LOAD_STATE_NONE;
                onIRCodeLoad(deviceID, rfKeyCodes, IRCODE_LOAD_STATUS_OK);
                m_lastSetRemoteID = -1;
                m_lastSetHas5DCPresent = false;
                m_lastSetSupports5DC = false;
                m_ramsOperatingMode = RAMS_OP_MODE_NONE;
                m_readProgress.clear();
            }
        }

        void RemoteActionMapping::handleFiveDigitCodeAccess(int deviceID)
        {
            unsigned tvLoadStatus = 0;
            unsigned avrLoadStatus = 0;
            int tvStatus = 0;
            int avrStatus = 0;

            // Send the load event if the remote has updated the Controller IRDB Status
            if (m_irdbLoadState == IRDB_LOAD_STATE_CTRLR_WRITTEN)
            {
                stopRIBLoadTimer();
                // Get the Controller IRDB Status TV and AVR Load Status values.
                if (!m_helper.getControllerLoadStatus(deviceID, tvLoadStatus, avrLoadStatus))
                {
                    LOGERR("helper getControllerLoadStatus() FAILED!");
                }
                else
                {
                    // Convert the RIB TV LoadStatus to the XRE TV LoadStatus
                    tvStatus = (int)(tvLoadStatus >> 4);
                    if (tvStatus > 6)
                    {
                        LOGERR("ERROR - Got invalid status %d from tvLoadStatus!?!", tvStatus);
                    }
                    // Convert the RIB AVR LoadStatus to the XRE AVR LoadStatus
                    avrStatus = (int)(avrLoadStatus >> 4);
                    if (avrStatus > 6)
                    {
                        LOGERR("ERROR - Got invalid status %d from avrLoadStatus!?!", avrStatus);
                    }
                }

                LOGWARN("Sending onFiveDigitCodeLoad event - tvResult: 0x%02X, avrResult: 0x%02X.",
                        tvLoadStatus, avrLoadStatus);
                m_irdbLoadState = IRDB_LOAD_STATE_NONE;
                onFiveDigitCodeLoad(deviceID, tvStatus, avrStatus);
                m_lastSetRemoteID = -1;
                m_lastSetHas5DCPresent = false;
                m_lastSetSupports5DC = false;
                m_fiveDigitCodeMode = FIVE_DIGIT_CODE_MODE_NONE;
                m_ramsOperatingMode = RAMS_OP_MODE_NONE;
            }
            else
            {
                LOGWARN("unexpected call with remoteId: %d, _irdbLoadState: %d!", deviceID, m_irdbLoadState);
            }
        }

        void RemoteActionMapping::handleRIBLoadTimeout()
        {
            std::lock_guard<std::mutex> guard(m_stateMutex);

            if (m_ramsOperatingMode == RAMS_OP_MODE_IRRF_DATABASE)
            {
                // Handle this IRRF Database loading timeout
                if ((m_irdbLoadState == IRDB_LOAD_STATE_PROGRESS) ||
                    (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ))
                {
                    int_vector_t rfKeyCodes;
                    int status = IRCODE_LOAD_STATUS_TIMEOUT_INCOMPLETE;

                    if (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ)
                    {
                        // Somehow, NONE of the IRDB data was read by the target controller.
                        // Clear the IRDB Download flag (back to default), to restore normal OK key operation.
                        m_helper.setIRDBDownloadFlag(m_lastSetRemoteID, false);
                        status = IRCODE_LOAD_STATUS_TIMEOUT_INCOMPLETE;
                    }
                    else
                    {
                        // Check to see if all the appropriate slots have been read.
                        if (checkIRRFDBReadProgress(rfKeyCodes))
                        {
                            status = IRCODE_LOAD_STATUS_TIMEOUT_COMPLETE;
                        }
                        else
                        {
                            status = IRCODE_LOAD_STATUS_TIMEOUT_INCOMPLETE;
                        }
                    }
                    if (m_lastSetHas5DCPresent && (status == IRCODE_LOAD_STATUS_TIMEOUT_INCOMPLETE))
                    {
                        // The controller didn't load the IRRF Database entries because
                        // it already had a 5-Digit Code set internally.
                        // Note that _lastSetHas5DCPresent can only be true for XRE API 3 or greater only!
                        status = IRCODE_LOAD_STATUS_REFUSED;
                    }

                    LOGWARN("TIMEOUT: IRRF Database LoadState(%d) - sending onIRCodeLoad event - numKeys: %d, status: %d.",
                            m_irdbLoadState, rfKeyCodes.size(), status);

                    m_irdbLoadState = IRDB_LOAD_STATE_NONE;
                    onIRCodeLoad(m_lastSetRemoteID, rfKeyCodes, status);
                    m_lastSetRemoteID = -1;
                    m_lastSetHas5DCPresent = false;
                    m_lastSetSupports5DC = false;
                    m_ramsOperatingMode = RAMS_OP_MODE_NONE;
                    m_readProgress.clear();
                }
                else
                {
                    LOGWARN("IRRF Database TIMEOUT without load-in-progress!");
                }
            }
            else if (m_ramsOperatingMode == RAMS_OP_MODE_FIVE_DIGIT_CODE)
            {
                // Handle this 5-Digit Code loading timeout
                if (m_irdbLoadState >= IRDB_LOAD_STATE_FLAG_READ)
                {
                    int tvStatus = 16;  // Timeout error codes
                    int avrStatus = 16;

                    // Only set the timeout error indication into the proper result(s) for what was being attempted.
                    if (m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_TV_SET)
                    {
                        avrStatus = 0;
                    }
                    else if (m_fiveDigitCodeMode == FIVE_DIGIT_CODE_MODE_AVR_SET)
                    {
                        tvStatus = 0;
                    }

                    if (m_irdbLoadState == IRDB_LOAD_STATE_FLAG_READ)
                    {
                        // Somehow, the target remote never did anything, despite having read the IRRF Status Flags.
                        // Clear the 5-Digit Code flag(s) (back to default), to restore normal OK key operation.
                        m_helper.setFiveDigitCodeFlags(m_lastSetRemoteID, FIVE_DIGIT_CODE_MODE_NONE);
                    }

                    LOGWARN("5-Digit Code TIMEOUT: _fiveDigitCodeMode: %d, _irdbLoadState: %d, sending onFiveDigitCodeLoad event - tvStatus: %d, avrStatus: %d.",
                            m_fiveDigitCodeMode, m_irdbLoadState, tvStatus, avrStatus);

                    m_irdbLoadState = IRDB_LOAD_STATE_NONE;
                    onFiveDigitCodeLoad(m_lastSetRemoteID, tvStatus, avrStatus);
                    m_lastSetRemoteID = -1;
                    m_lastSetHas5DCPresent = false;
                    m_lastSetSupports5DC = false;
                    m_fiveDigitCodeMode = FIVE_DIGIT_CODE_MODE_NONE;
                    m_ramsOperatingMode = RAMS_OP_MODE_NONE;
                }
                else
                {
                    LOGWARN("5-Digit Code TIMEOUT without load-in-progress!");
                }
            } else {
                LOGWARN("Bogus m_ramsOperatingMode: %d!", m_ramsOperatingMode);
            }
        }

        void RemoteActionMapping::startRIBLoadTimer(int msec)
        {
            stopRIBLoadTimer();
            ribLoadTimer.Schedule(Core::Time::Now().Add(msec), m_ribLoadTimeoutImpl);
            LOGINFO("RIB Load Timer started - time: %dms.", msec);
        }

        void RemoteActionMapping::stopRIBLoadTimer()
        {
            ribLoadTimer.Revoke(m_ribLoadTimeoutImpl);
            LOGINFO("RIB Load Timer stopped.");
        }
        //End local private utility methods

        // Friend class RibLoadTimeoutImpl public Timed method implementation.
        // Core::TimerType callback name is fixed, so this method MUST be called "Timed"!
        uint64_t RibLoadTimeoutImpl::Timed(const uint64_t scheduledTime)
        {
            uint64_t result = 0;
            m_ram->handleRIBLoadTimeout();
            return(result);
        }

    } // namespace Plugin

} // namespace WPEFramework
