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

#ifndef SYSTEMSERVICES_H
#define SYSTEMSERVICES_H

#include <stdint.h>
#include <thread>
#include <regex.h>
#include <cctype>
#include <fstream>
#include <cstring>
using std::ofstream;
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Module.h"
#include "tracing/Logging.h"
#include "UtilsThreadRAII.h"
#include "SystemServicesHelper.h"
#include "platformcaps/platformcaps.h"
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#include "pwrMgr.h"
#include "host.hpp"
#include "sleepMode.hpp"
#endif /* USE_IARMBUS || USE_IARM_BUS */

#include "sysMgr.h"
#include "cSettings.h"
#include "cTimer.h"
#include "rfcapi.h"

/* System Services Triggered Events. */
#define EVT_ONSYSTEMSAMPLEEVENT           "onSampleEvent"
#define EVT_ONSYSTEMPOWERSTATECHANGED     "onSystemPowerStateChanged"
#define EVT_ONSYSTEMMODECHANGED           "onSystemModeChanged"
#define EVT_ONNETWORKSTANDBYMODECHANGED   "onNetworkStandbyModeChanged"
#define EVT_ONFIRMWAREUPDATEINFORECEIVED  "onFirmwareUpdateInfoReceived"
#define EVT_ONFIRMWAREUPDATESTATECHANGED  "onFirmwareUpdateStateChange"
#define EVT_ONTEMPERATURETHRESHOLDCHANGED "onTemperatureThresholdChanged"
#define EVT_ONMACADDRESSRETRIEVED         "onMacAddressesRetreived"
#define EVT_ONREBOOTREQUEST               "onRebootRequest"
#define EVT_ON_SYSTEM_CLOCK_SET           "onSystemClockSet"
#define EVT_ONFWPENDINGREBOOT             "onFirmwarePendingReboot" /* Auto Reboot notifier */
#define EVT_ONREBOOTREQUEST               "onRebootRequest"
#define EVT_ONTERRITORYCHANGED            "onTerritoryChanged"
#define EVT_ONTIMEZONEDSTCHANGED          "onTimeZoneDSTChanged"
#define TERRITORYFILE                     "/opt/secure/persistent/System/Territory.txt"

namespace WPEFramework {
    namespace Plugin {

        // This is a server for a JSONRPC communication channel.
        // For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
        // By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
        // This realization of this interface implements, by default, the following methods on this plugin
        // - exists
        // - register
        // - unregister
        // Any other methood to be handled by this plugin  can be added can be added by using the
        // templated methods Register on the PluginHost::JSONRPC class.
        // As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
        // this class exposes a public method called, Notify(), using this methods, all subscribed clients
        // will receive a JSONRPC message as a notification, in case this method is called.

        /**
         * @struct modeInfo
         * @brief This structure contains information of modes.
         * @ingroup SERVMGR_SYSTEM
         */
        struct modeInfo {
            std::string mode;  //operating mode
            int duration;  // duration in seconds
        };

        class SystemServices : public PluginHost::IPlugin, public PluginHost::JSONRPC {
            private:
                typedef Core::JSON::String JString;
                typedef Core::JSON::ArrayType<JString> JStringArray;
                typedef Core::JSON::Boolean JBool;
                string m_stbVersionString;
                cSettings m_cacheService;
                static cSettings m_temp_settings;
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                static IARM_Bus_SYSMgr_GetSystemStates_Param_t paramGetSysState;
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
                Utils::ThreadRAII thread_getMacAddresses;
                SystemServices* m_systemService;
                /* TODO: Need to decide whether needed or not since setProperty
                   and getProperty functionalities are XRE/RTRemote dependent. */
                static JsonObject _systemParams;
                static const string MODEL_NAME;
                static const string HARDWARE_ID;
		static const string FRIENDLY_ID;

                enum class FWUpdateAvailableEnum { FW_UPDATE_AVAILABLE, FW_MATCH_CURRENT_VER, NO_FW_VERSION, EMPTY_SW_UPDATE_CONF };
                // We do not allow this plugin to be copied !!
                SystemServices(const SystemServices&) = delete;
                SystemServices& operator=(const SystemServices&) = delete;
                static void getMacAddressesAsync(SystemServices *p);
                static std::string m_currentMode;
                std::string m_current_state;
                static cTimer m_operatingModeTimer;
                static int m_remainingDuration;
                Utils::ThreadRAII m_getFirmwareInfoThread;
                PluginHost::IShell* m_shellService { nullptr };
                regex_t m_regexUnallowedChars;

                int m_FwUpdateState_LatestEvent;

                bool m_networkStandbyMode;
                bool m_networkStandbyModeValid;

                std::string m_powerStateBeforeReboot;
                bool m_powerStateBeforeRebootValid;

		std::string m_strTerritory;
                std::string m_strRegion;

                static void startModeTimer(int duration);
                static void stopModeTimer();
                static void updateDuration();
				std::string  m_strStandardTerritoryList;
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
                bool getManufacturerData(const string& parameter, JsonObject& response);
                uint32_t getMfgSerialNumber(const JsonObject& parameters, JsonObject& response);
                bool getModelName(const string& parameter, JsonObject& response);
		std::string m_ManufacturerDataHardwareID;
		std::string m_ManufacturerDataModelName;
                bool m_ManufacturerDataHardwareIdValid;
		bool m_ManufacturerDataModelNameValid;
                std::string m_MfgSerialNumber;
                bool m_MfgSerialNumberValid;
		
#endif
            public:
                SystemServices();
                virtual ~SystemServices();

                static SystemServices* _instance;
                virtual const string Initialize(PluginHost::IShell* service) override;
                virtual void Deinitialize(PluginHost::IShell* service) override;
                virtual string Information() const override { return {}; }

                BEGIN_INTERFACE_MAP(SystemServices)
                INTERFACE_ENTRY(PluginHost::IPlugin)
                INTERFACE_ENTRY(PluginHost::IDispatcher)
                END_INTERFACE_MAP

                static int runScript(const std::string& script,
                        const std::string& args, string *output = NULL,
                        string *error = NULL, int timeout = 30000);
                std::string getStbVersionString();
                std::string getClientVersionString();
                std::string getStbTimestampString();

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                void InitializeIARM();
                void DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

                /* Events : Begin */
                void onFirmwareUpdateInfoRecieved(string CallGUID);
                void onSystemPowerStateChanged(string currentPowerState, string powerState);
                void onPwrMgrReboot(string requestedApp, string rebootReason);
                void onNetorkModeChanged(bool betworkStandbyMode);
                void onSystemModeChanged(string mode);
                void onFirmwareUpdateStateChange(int state);
                void onClockSet();
                void onTemperatureThresholdChanged(string thresholdType,
                        bool exceed, float temperature);
                void onRebootRequest(string reason);
                void onFirmwarePendingReboot(int seconds); /* Event handler for Pending Reboot */
		void onTerritoryChanged(string oldTerritory, string newTerritory, string oldRegion="", string newRegion="");
		void onTimeZoneDSTChanged(string oldTimeZone, string newTimeZone);
                /* Events : End */

                /* Methods : Begin */
#ifdef DEBUG
                uint32_t sampleAPI(const JsonObject& parameters, JsonObject& response);
#endif /* DEBUG */
                uint32_t requestSystemReboot(const JsonObject& parameters, JsonObject& response);
                uint32_t requestSystemUptime(const JsonObject& parameters, JsonObject& response);
                uint32_t requestEnableMoca(const JsonObject& parameters, JsonObject& response);
                uint32_t getDeviceInfo(const JsonObject& parameters, JsonObject& response);
                uint32_t queryMocaStatus(const JsonObject& parameters, JsonObject& response);
                uint32_t getStateInfo(const JsonObject& parameter, JsonObject& resposne);
#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
                uint32_t getDevicePowerState(const JsonObject& parameters,JsonObject& response);
                uint32_t setDevicePowerState(const JsonObject& parameters,JsonObject& response);
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */
                uint32_t isRebootRequested(const JsonObject& parameters,JsonObject& response);
                uint32_t setGZEnabled(const JsonObject& parameters,JsonObject& response);
                uint32_t isGZEnabled(const JsonObject& parameters,JsonObject& response);
                uint32_t getSystemVersions(const JsonObject& parameters, JsonObject& response);

                /* TODO: Stub implementation; Decide whether needed or not since setProperty
                   and getProperty functionalities are XRE/RTRemote dependent. */
                bool setProperties(const JsonObject& propertyNames);
                bool getProperties(const JsonObject& params, JsonObject& returnProperties);
                uint32_t getCachedValue(const JsonObject& parameters, JsonObject& response);
                uint32_t setCachedValue(const JsonObject& parameters, JsonObject& response);
                uint32_t cacheContains(const JsonObject& parameters, JsonObject& response);
                uint32_t removeCacheKey(const JsonObject& parameters, JsonObject& response);
                uint32_t getMode(const JsonObject& parameters, JsonObject& response);
                uint32_t updateFirmware(const JsonObject& parameters, JsonObject& response);
                uint32_t setMode(const JsonObject& parameters, JsonObject& response);
                uint32_t setBootLoaderPattern(const JsonObject& parameters, JsonObject& response);
                static void firmwareUpdateInfoReceived(void);
                uint32_t getFirmwareUpdateInfo(const JsonObject& parameters, JsonObject& response);
                void reportFirmwareUpdateInfoReceived(string firmwareUpdateVersion,
                        int httpStatus, bool success, string firmwareVersion, string responseString);
                uint32_t setDeepSleepTimer(const JsonObject& parameters, JsonObject& response);
                uint32_t setPreferredStandbyMode(const JsonObject& parameters, JsonObject& response);
                uint32_t getPreferredStandbyMode(const JsonObject& parameters, JsonObject& response);
                uint32_t getAvailableStandbyModes(const JsonObject& parameters, JsonObject& response);
#ifdef ENABLE_DEEP_SLEEP
		uint32_t getWakeupReason(const JsonObject& parameters, JsonObject& response);
                uint32_t getLastWakeupKeyCode(const JsonObject& parameters, JsonObject& response);
#endif
		uint32_t setTerritory(const JsonObject& parameters, JsonObject& response);
		uint32_t getTerritory(const JsonObject& parameters, JsonObject& response);
		bool readTerritoryFromFile();
		bool isStrAlphaUpper(string strVal);
		bool isRegionValid(string regionStr);
		uint32_t writeTerritory(string territory, string region);
                uint32_t getXconfParams(const JsonObject& parameters, JsonObject& response);
                uint32_t getSerialNumber(const JsonObject& parameters, JsonObject& response);
                bool getSerialNumberTR069(JsonObject& response);
                bool getSerialNumberSnmp(JsonObject& response);
                uint32_t getFirmwareDownloadPercent(const JsonObject& parameters,JsonObject& response);
                uint32_t getFirmwareUpdateState(const JsonObject& parameters, JsonObject& response);
                uint32_t getDownloadedFirmwareInfo(const JsonObject& parameters, JsonObject& response);
                uint32_t getMacAddresses(const JsonObject& parameters, JsonObject& response);
                uint32_t setTimeZoneDST(const JsonObject& parameters, JsonObject& response);
                uint32_t getTimeZoneDST(const JsonObject& parameters, JsonObject& response);
                bool processTimeZones(std::string dir, JsonObject& out);
                uint32_t getTimeZones(const JsonObject& parameters, JsonObject& response);

                uint32_t getCoreTemperature(const JsonObject& parameters, JsonObject& response);
                uint32_t getPreviousRebootInfo(const JsonObject& parameters, JsonObject& response);
                uint32_t getLastDeepSleepReason(const JsonObject& parameters, JsonObject& response);
                uint32_t clearLastDeepSleepReason(const JsonObject& parameters, JsonObject& response);
#ifdef ENABLE_THERMAL_PROTECTION
                uint32_t getTemperatureThresholds(const JsonObject& parameters, JsonObject& response);
                uint32_t setTemperatureThresholds(const JsonObject& parameters, JsonObject& response);
		uint32_t getOvertempGraceInterval(const JsonObject& parameters, JsonObject& response);
                uint32_t setOvertempGraceInterval(const JsonObject& parameters, JsonObject& response);
#endif /* ENABLE_THERMAL_PROTECTION */
                uint32_t getPreviousRebootInfo2(const JsonObject& parameters, JsonObject& response);
                uint32_t getPreviousRebootReason(const JsonObject& parameters, JsonObject& response);
                uint32_t getRFCConfig(const JsonObject& parameters, JsonObject& response);
                uint32_t getMilestones(const JsonObject& parameters, JsonObject& response);
                uint32_t enableXREConnectionRetention(const JsonObject& parameters, JsonObject& response);
                uint32_t setNetworkStandbyMode (const JsonObject& parameters, JsonObject& response);
                uint32_t getNetworkStandbyMode (const JsonObject& parameters, JsonObject& response);
                uint32_t getPowerStateIsManagedByDevice(const JsonObject& parameters, JsonObject& response);
                uint32_t uploadLogs(const JsonObject& parameters, JsonObject& response);
                uint32_t getPowerStateBeforeReboot (const JsonObject& parameters,JsonObject& response);
                uint32_t getLastFirmwareFailureReason(const JsonObject& parameters, JsonObject& response);
                uint32_t setOptOutTelemetry(const JsonObject& parameters,JsonObject& response);
                uint32_t isOptOutTelemetry(const JsonObject& parameters,JsonObject& response);
                uint32_t fireFirmwarePendingReboot(const JsonObject& parameters, JsonObject& response);
                uint32_t setFirmwareRebootDelay(const JsonObject& parameters, JsonObject& response);
                uint32_t setFirmwareAutoReboot(const JsonObject& parameters, JsonObject& response);
                uint32_t getStoreDemoLink(const JsonObject& parameters, JsonObject& response);
                uint32_t deletePersistentPath(const JsonObject& parameters, JsonObject& response);
#ifdef ENABLE_SET_WAKEUP_SRC_CONFIG
                uint32_t setWakeupSrcConfiguration(const JsonObject& parameters, JsonObject& response);
#endif //ENABLE_SET_WAKEUP_SRC_CONFIG
                uint32_t getPlatformConfiguration(const JsonObject& parameters, PlatformCaps& response);
        }; /* end of system service class */
    } /* end of plugin */
} /* end of wpeframework */

#endif //SYSTEMSERVICES_H
