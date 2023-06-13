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
#include "libIARM.h"

#include "RamHelper.h"

#include <mutex>
#include <map>

#define IARM_REMOTEACTIONMAPPING_PLUGIN_NAME    "Remote_Action_Mapping"

#define RAM_TIMER_THREAD_STACK_SIZE     (64 * 1024)
#define RAM_TIMER_THREAD_NAME           "RAMRIBLoadTimer"

// Substitute for the old AbstractService Status enumeration
typedef enum {
    STATUS_OK           = 0,
    STATUS_FAILURE,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_STATE,
    STATUS_METHOD_NOT_FOUND
} StatusCode;

// To track the different states of the controller-event-driven loading/clearing sequence (for IRRF Database or 5-Digit Codes)
typedef enum {
    IRDB_LOAD_STATE_NONE,           // Idle.
    IRDB_LOAD_STATE_WRITTEN,        // IRRF Database entries (IRRF_DATABASE mode), or Target IRDB Status TV/AVR 5-Digit Codes (FIVE_DIGIT_CODE mode), are set.
    IRDB_LOAD_STATE_FLAG_READ,      // IRRF Status Flags (both modes) have been read by the controller.
    IRDB_LOAD_STATE_PROGRESS,       // IRRF Database entries are being read by the controller. IRRF_DATABASE mode only.
    IRDB_LOAD_STATE_TARGET_READ,    // Target IRDB Status TV/AVR 5-Digit Codes have been read by the controller. FIVE_DIGIT_CODE mode only.
    IRDB_LOAD_STATE_CTRLR_WRITTEN   // Controller IRDB Status has been written (updated) by the controller. FIVE_DIGIT_CODE mode only.
} IRDBLoadState;

typedef enum {
    RAMS_OP_MODE_NONE,
    RAMS_OP_MODE_IRRF_DATABASE,
    RAMS_OP_MODE_FIVE_DIGIT_CODE
} RAMSOperatingMode;

// Timeout values for the remote reading the RIB IRRF Database entries, in milliseconds
// NOTE that there IS NO TIMEOUT for how long it takes the customer to press the OK key!!
#define TIMEOUT_REMOTE_IRRFDB_READ_IRCODE_START             4000    // Max delay between OK keypress read and 1st key IRCode read
#define TIMEOUT_REMOTE_IRRFDB_READ_IRCODE_NEXT              900     // Max delay between key IRCode reads, once they start
// Timeout values for the remote reading the RIB 5-Digit Codes, and subsequent writing controller status
#define TIMEOUT_REMOTE_FIVEDIGITCODE_READ_TARGET            500     // Max delay between OK keypress read and Target IRDB Status read
#define TIMEOUT_REMOTE_FIVEDIGITCODE_WRITE_CONTROLLER       1600    // Max delay between Target IRDB Status read and Controller IRDB Status write



namespace WPEFramework {

    namespace Plugin {

        class RemoteActionMapping;  // Forward declaration

        // Load timer implementation, for timeout on controller loading from the RIB
        class RibLoadTimeoutImpl
        {
        private:
            RibLoadTimeoutImpl() = delete;
            RibLoadTimeoutImpl& operator=(const RibLoadTimeoutImpl& RHS) = delete;

        public:
            RibLoadTimeoutImpl(RemoteActionMapping* ram): m_ram(ram){}
            RibLoadTimeoutImpl(const RibLoadTimeoutImpl& copy): m_ram(copy.m_ram){}
            ~RibLoadTimeoutImpl() {}

            inline bool operator==(const RibLoadTimeoutImpl& RHS) const
            {
                return(m_ram == RHS.m_ram);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);   // The method MUST be called "Timed"!

        private:
            RemoteActionMapping* m_ram;
        };

        class RemoteActionMapping : public PluginHost::IPlugin, public PluginHost::JSONRPC {
        private:
            typedef Core::JSON::String JString;
            typedef Core::JSON::ArrayType<JString> JStringArray;
            typedef Core::JSON::ArrayType<JsonObject> JObjectArray;
            typedef Core::JSON::Boolean JBool;

            // We do not allow this plugin to be copied !!
            RemoteActionMapping(const RemoteActionMapping&) = delete;
            RemoteActionMapping& operator=(const RemoteActionMapping&) = delete;

            //Begin methods
            uint32_t getApiVersionNumber(const JsonObject& parameters, JsonObject& response);

            uint32_t getLastUsedDeviceIDWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getKeymapWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t clearKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getFullKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t getSingleKeyActionMappingWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t cancelCodeDownloadWrapper(const JsonObject& parameters, JsonObject& response);
            uint32_t setFiveDigitCodeWrapper(const JsonObject& parameters, JsonObject& response);
            //End methods

            //Begin events
            void onIRCodeLoad(int deviceID, int_vector_t& rfKeyCodes, int status);
            void onFiveDigitCodeLoad(int deviceID, int tvStatus, int avrStatus);
            //End events

        public:
            RemoteActionMapping();
            virtual ~RemoteActionMapping();
            //IPlugin methods
            virtual const string Initialize(PluginHost::IShell* service) override;
            virtual void Deinitialize(PluginHost::IShell* service) override;
            virtual string Information() const override { return {}; }

            BEGIN_INTERFACE_MAP(RemoteActionMapping)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

        private:
            void InitializeIARM();
            void DeinitializeIARM();
            // Handlers for IARM events
            static void ramEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

            // Underlying private implementations for public wrapper methods
            int getLastUsedDeviceID(std::string& remoteType, bool& pbFiveDigitCodeSet, bool& pbFiveDigitCodeSupported);
            JsonArray getKeymap(int deviceID, int keymapType);
            bool setKeyActionMapping(int deviceID, int keymapType, std::map<int, keyActionMap>& localActionMaps, const KeyGroupSrcInfo& srcInfo );
            bool clearKeyActionMapping(int deviceID, int keymapType, int* keyNames, int numNames);
            JObjectArray getFullKeyActionMapping(int deviceID, int keymapType);
            JsonObject getSingleKeyActionMapping(int deviceID, int keymapType, int keyName);
            bool cancelCodeDownload(int deviceID);
            bool setFiveDigitCode(int deviceID, int tvFiveDigitCode, int avrFiveDigitCode);

            // Local utility methods
            void setApiVersionNumber(uint32_t apiVersionNumber);

            bool setKeyGroups(KeyGroupSrcInfo& srcInfo, const KeyPresenceFlags& keyPresence);
            bool checkClearList(RFKeyFlags& rfKeyFlags, int* keyNames, int* numNames);

            bool checkIRRFDBReadProgress(int_vector_t& rfKeyCodes);
            void handleIRRFDBKeyRead(int deviceID, int rfKey);
            void handleFiveDigitCodeAccess(int deviceID);
            void handleRIBLoadTimeout();

            void startRIBLoadTimer(int msec);
            void stopRIBLoadTimer();

        public:
            static RemoteActionMapping* _instance;
        private:
            uint32_t    m_apiVersionNumber;

            std::mutex  m_stateMutex;

            RemoteActionMappingHelper m_helper;
            friend class RemoteActionMappingHelper;

            RibLoadTimeoutImpl m_ribLoadTimeoutImpl;
            friend class RibLoadTimeoutImpl;

            // State machine
            static IRDBLoadState m_irdbLoadState;
            static IRRFDBCtrlrLoadProgress m_readProgress;
            static int m_lastSetRemoteID;
            static int m_ramsOperatingMode;
            static int m_fiveDigitCodeMode;
            static bool m_lastSetHas5DCPresent;
            static bool m_lastSetSupports5DC;
        };
	} // namespace Plugin
} // namespace WPEFramework
