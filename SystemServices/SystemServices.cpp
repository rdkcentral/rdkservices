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

#include <stdlib.h>
#include <errno.h>
#include <cstdio>
#include <regex>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <bits/stdc++.h>
#include <algorithm>
#include <curl/curl.h>

#include "SystemServices.h"
#include "StateObserverHelper.h"
#include "uploadlogs.h"

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIBusDaemon.h"
#include "UtilsIarm.h"
#endif /* USE_IARMBUS || USE_IARM_BUS */

#ifdef ENABLE_THERMAL_PROTECTION
#include "thermonitor.h"
#endif /* ENABLE_THERMAL_PROTECTION */

#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
#include "libIBus.h"
#include "pwrMgr.h"
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */

#include "mfrMgr.h"

#ifdef ENABLE_DEEP_SLEEP
#include "deepSleepMgr.h"
#endif

#include "UtilsCStr.h"
#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"
#include "UtilscRunScript.h"
#include "UtilsfileExists.h"

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 4
#define API_VERSION_NUMBER_PATCH 0

#define MAX_REBOOT_DELAY 86400 /* 24Hr = 86400 sec */
#define TR181_FW_DELAY_REBOOT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"
#define TR181_AUTOREBOOT_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"

#define RFC_PWRMGR2 "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.PwrMgr2.Enable"

#define ZONEINFO_DIR "/usr/share/zoneinfo"
#define LOCALTIME_FILE "/opt/persistent/localtime"

#define DEVICE_PROPERTIES_FILE "/etc/device.properties"

#define DEVICE_INFO_SCRIPT "sh /lib/rdk/getDeviceDetails.sh read"

#define STATUS_CODE_NO_SWUPDATE_CONF 460 

#define OPTOUT_TELEMETRY_STATUS "/opt/tmtryoptout"

#define REGEX_UNALLOWABLE_INPUT "[^[:alnum:]_-]{1}"

#define STORE_DEMO_FILE "/opt/persistent/store-mode-video/videoFile.mp4"
#define STORE_DEMO_LINK "file:///opt/persistent/store-mode-video/videoFile.mp4"

#define RFC_LOG_UPLOAD "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.LogUploadBeforeDeepSleep.Enable"
#define TR181_SYSTEM_FRIENDLY_NAME "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.SystemServices.FriendlyName"

#define LOG_UPLOAD_STATUS_SUCCESS "UPLOAD_SUCCESS"
#define LOG_UPLOAD_STATUS_FAILURE "UPLOAD_FAILURE"
#define LOG_UPLOAD_STATUS_ABORTED "UPLOAD_ABORTED"


/**
 * @struct firmwareUpdate
 * @brief This structure contains information of firmware update.
 * @ingroup SERVMGR_SYSTEM
 */
struct firmwareUpdate {
    string firmwareUpdateVersion; // firmware Version
    int httpStatus;    //http Response code
    bool success;
};

/**
 * @brief This function is used get the moca file is present or not.
 * @return true if the moca file is present else returns false.
 */
bool getMocaStatus(void)
{
    bool status = false;

    if (Utils::fileExists(MOCA_FILE)) {
        status = true;
    } else {
        /* Nothing to do. */
    }

    return status;
}

/**
 * @brief Stores the value of enabled in the file /opt/gzenabled.
 *
 * @param enabled Boolean flag.
 * @return Returns true if it has persisted the flag, false otherwise.
 */
bool setGzEnabled(bool enabled)
{
    bool retVal = false;
    ofstream outfile;

    outfile.open(GZ_STATUS.c_str(), ios::out);
    if (outfile) {
        outfile << (enabled ? "true" :"false");
        outfile.close();
        retVal = true;
    } else {
        printf(" GZ_FILE_ERROR: Can't open file for write mode\n");
    }
    return retVal;
}

/**
 * @brief returns the value of the contents of /opt/gzenabled.
 *
 * @return Returns value that has read from the file.
 */
bool isGzEnabledHelper(bool& enabled)
{
    bool retVal = false;
    string gzEnabled;

    retVal = getFileContent(GZ_STATUS.c_str(), gzEnabled);
    if (retVal && gzEnabled.length()) {
        if (gzEnabled.find("true") != string::npos) {
            enabled = true;
        } else {
            enabled = false;
        }
        retVal = true;
    } else {
        retVal = false;
    }

    return retVal;
}

/**
 * @brief returns the value of the contents of requested mac/ip
 *
 * @return Returns value that has read from the file.
 */
string collectDeviceInfo(string methodType)
{
    FILE *fp = NULL;
    char *pData = NULL;
    string respBuffer;
    string fileName = "/tmp/." + methodType;
    char buffer[128] = {'\0'};

    if ((fp = fopen(fileName.c_str(), "r")) != NULL) {
        pData = fgets(buffer, (sizeof(buffer) - 1), fp);
        fclose(fp);

        /* Process read information from file */
        if ((strlen(buffer) > 0) && (pData)) {
            respBuffer = buffer;
            if ("ecm_mac" == methodType) {
                respBuffer = regex_replace(respBuffer, std::regex(" "), ":");
            }
        } else {
            /* temp fix for XONE-11787. Proper fix will
               be coming from OCAP team */
            if ("estb_mac" == methodType) {
                respBuffer = "Acquiring";
            } else {
                respBuffer = "";
            }
        }
    }
    return respBuffer;
}

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)

std::string iarmModeToString(IARM_Bus_Daemon_SysMode_t& iarmMode)
{
    if (IARM_BUS_SYS_MODE_WAREHOUSE == iarmMode) {
        return MODE_WAREHOUSE;
    } else if (IARM_BUS_SYS_MODE_EAS == iarmMode) {
        return MODE_EAS;
    }

    return MODE_NORMAL;
}

void stringToIarmMode(std::string mode, IARM_Bus_Daemon_SysMode_t& iarmMode)
{
    if (MODE_WAREHOUSE == mode) {
        iarmMode = IARM_BUS_SYS_MODE_WAREHOUSE;
    } else if (MODE_EAS == mode) {
        iarmMode = IARM_BUS_SYS_MODE_EAS;
    } else {
        iarmMode = IARM_BUS_SYS_MODE_NORMAL;
    }
}

bool setPowerState(std::string powerState)
{
    IARM_Bus_PWRMgr_SetPowerState_Param_t param;
    if (powerState == "STANDBY") {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
    } else if (powerState == "ON") {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    } else if (powerState == "DEEP_SLEEP") {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    } else if (powerState == "LIGHT_SLEEP") {
        param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
    } else {
        return false;
    }

    IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_SetPowerState,
        (void*)&param, sizeof(param));

    if (res == IARM_RESULT_SUCCESS)
        return true;
    else
        return false;
}

#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

/**
 * @brief WPEFramework class for SystemServices
 */
namespace WPEFramework {

    namespace {

        static Plugin::Metadata<Plugin::SystemServices> metadata(
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
        //Prototypes
        std::string   SystemServices::m_currentMode = "";
        cTimer    SystemServices::m_operatingModeTimer;
        int       SystemServices::m_remainingDuration = 0;
        JsonObject SystemServices::_systemParams;
        const string SystemServices::MODEL_NAME = "modelName";
        const string SystemServices::HARDWARE_ID = "hardwareID";
	const string SystemServices::FRIENDLY_ID = "friendly_id";

        IARM_Bus_SYSMgr_GetSystemStates_Param_t SystemServices::paramGetSysState = {};

        static void _powerEventHandler(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len);

#ifdef ENABLE_THERMAL_PROTECTION
        static void handleThermalLevelChange(IARM_Bus_PWRMgr_EventData_t *param);
        void _thermMgrEventsHandler(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len);
#endif /* ENABLE_THERMAL_PROTECTION */

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        static IARM_Result_t _SysModeChange(void *arg);
        static void _systemStateChanged(const char *owner,
                IARM_EventId_t eventId, void *data, size_t len);
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

        SERVICE_REGISTRATION(SystemServices, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        SystemServices* SystemServices::_instance = nullptr;
        cSettings SystemServices::m_temp_settings(SYSTEM_SERVICE_TEMP_FILE);

        /**
         * Register SystemService module as wpeframework plugin
         */
        SystemServices::SystemServices()
            : PluginHost::JSONRPC()
              , m_cacheService(SYSTEM_SERVICE_SETTINGS_FILE)
        {
            SystemServices::_instance = this;
	    //Updating the standard territory
            m_strStandardTerritoryList =   "ABW AFG AGO AIA ALA ALB AND ARE ARG ARM ASM ATA ATF ATG AUS AUT AZE BDI BEL BEN BES BFA BGD BGR BHR BHS BIH BLM BLR BLZ BMU BOL                BRA BRB BRN BTN BVT BWA CAF CAN CCK CHE CHL CHN CIV CMR COD COG COK COL COM CPV CRI CUB Cuba CUW CXR CYM CYP CZE DEU DJI DMA DNK DOM DZA ECU EGY ERI ESH ESP                EST ETH FIN FJI FLK FRA FRO FSM GAB GBR GEO GGY GHA GIB GIN GLP GMB GNB GNQ GRC GRD GRL GTM GUF GUM GUY HKG HMD HND HRV HTI HUN IDN IMN IND IOT IRL IRN IRQ                 ISL ISR ITA JAM JEY JOR JPN KAZ KEN KGZ KHM KIR KNA KOR KWT LAO LBN LBR LBY LCA LIE LKA LSO LTU LUX LVA MAC MAF MAR MCO MDA MDG MDV MEX MHL MKD MLI MLT MMR                 MNE MNG MNP MOZ MRT MSR MTQ MUS MWI MYS MYT NAM NCL NER NFK NGA NIC NIU NLD NOR NPL NRU NZL OMN PAK PAN PCN PER PHL PLW PNG POL PRI PRK PRT PRY PSE PYF QAT                 REU ROU RUS RWA SAU SDN SEN SGP SGS SHN SJM SLB SLE SLV SMR SOM SPM SRB SSD STP SUR SVK SVN SWE SWZ SXM SYC SYR TCA TCD TGO THA TJK TKL TKM TLS TON TTO TUN                 TUR TUV TWN TZA UGA UKR UMI URY USA UZB VAT VCT VEN VGB VIR VNM VUT WLF WSM YEM ZAF ZMB ZWE";

            CreateHandler({ 2 });

            SystemServices::m_FwUpdateState_LatestEvent=FirmwareUpdateStateUninitialized;

            m_networkStandbyModeValid = false;
            m_powerStateBeforeRebootValid = false;
            m_isPwrMgr2RFCEnabled = false;
            m_friendlyName = "Living Room";


#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
	    m_ManufacturerDataHardwareIdValid = false;
	    m_ManufacturerDataModelNameValid = false;
            m_MfgSerialNumberValid = false;
#endif
            m_uploadLogsPid = -1;

            regcomp (&m_regexUnallowedChars, REGEX_UNALLOWABLE_INPUT, REG_EXTENDED);

            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            registerMethod("sampleSystemServiceAPI", &SystemServices::sampleAPI, this);
#endif /* DEBUG */
            registerMethod("getDeviceInfo", &SystemServices::getDeviceInfo, this);
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
            registerMethod("getMfgSerialNumber", &SystemServices::getMfgSerialNumber, this);
#endif
            registerMethod("reboot", &SystemServices::requestSystemReboot, this);
            registerMethod("enableMoca", &SystemServices::requestEnableMoca, this);
            registerMethod("queryMocaStatus", &SystemServices::queryMocaStatus,
                    this);
            registerMethod("requestSystemUptime",
                    &SystemServices::requestSystemUptime, this);
            registerMethod("getStateInfo", &SystemServices::getStateInfo, this);
#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
            registerMethod("getPowerState", &SystemServices::getDevicePowerState,
                    this);
            registerMethod("setPowerState", &SystemServices::setDevicePowerState,
                    this);
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */
            registerMethod("setGzEnabled", &SystemServices::setGZEnabled, this);
            registerMethod("isGzEnabled", &SystemServices::isGZEnabled, this);
            registerMethod("hasRebootBeenRequested",
                    &SystemServices::isRebootRequested, this);
            registerMethod("getMode", &SystemServices::getMode, this);
            registerMethod("updateFirmware", &SystemServices::updateFirmware, this);
            registerMethod("setMode", &SystemServices::setMode, this);
            registerMethod("setBootLoaderPattern", &SystemServices::setBootLoaderPattern, this);
            registerMethod("getFirmwareUpdateInfo",
                    &SystemServices::getFirmwareUpdateInfo, this);
            registerMethod("setDeepSleepTimer", &SystemServices::setDeepSleepTimer,
                    this);
            registerMethod("setPreferredStandbyMode",
                    &SystemServices::setPreferredStandbyMode, this);
            registerMethod("getPreferredStandbyMode",
                    &SystemServices::getPreferredStandbyMode, this);
            registerMethod("getAvailableStandbyModes",
                    &SystemServices::getAvailableStandbyModes, this);
            registerMethod("getXconfParams", &SystemServices::getXconfParams, this);
            registerMethod("getSerialNumber", &SystemServices::getSerialNumber,
                    this);
            registerMethod("getDownloadedFirmwareInfo",
                    &SystemServices::getDownloadedFirmwareInfo, this);
            registerMethod("getFirmwareDownloadPercent",
                    &SystemServices::getFirmwareDownloadPercent, this);
            registerMethod("getFirmwareUpdateState",
                    &SystemServices::getFirmwareUpdateState, this);
            registerMethod("getMacAddresses",&SystemServices::getMacAddresses, this);
            registerMethod("setTimeZoneDST", &SystemServices::setTimeZoneDST, this);
            registerMethod("getTimeZoneDST", &SystemServices::getTimeZoneDST, this);
            registerMethod("getCoreTemperature", &SystemServices::getCoreTemperature,
                    this);
            registerMethod("getCachedValue", &SystemServices::getCachedValue, this);
            registerMethod("setCachedValue", &SystemServices::setCachedValue, this);
            registerMethod("cacheContains", &SystemServices::cacheContains, this);
            registerMethod("removeCacheKey", &SystemServices::removeCacheKey, this);
            registerMethod("getPreviousRebootInfo",
                    &SystemServices::getPreviousRebootInfo, this);
            registerMethod("getLastDeepSleepReason",
                    &SystemServices::getLastDeepSleepReason, this);
            registerMethod("clearLastDeepSleepReason",
                    &SystemServices::clearLastDeepSleepReason, this);
#ifdef ENABLE_THERMAL_PROTECTION
            registerMethod("getTemperatureThresholds",
                    &SystemServices::getTemperatureThresholds, this);
            registerMethod("setTemperatureThresholds",
                    &SystemServices::setTemperatureThresholds, this);
	    registerMethod("getOvertempGraceInterval",
                    &SystemServices::getOvertempGraceInterval, this);
            registerMethod("setOvertempGraceInterval",
                    &SystemServices::setOvertempGraceInterval, this);
#endif /* ENABLE_THERMAL_PROTECTION */
            registerMethod("getPreviousRebootInfo2",
                    &SystemServices::getPreviousRebootInfo2, this);
            registerMethod("getPreviousRebootReason",
                    &SystemServices::getPreviousRebootReason, this);
            registerMethod("getRFCConfig", &SystemServices::getRFCConfig, this);
            registerMethod("getMilestones", &SystemServices::getMilestones, this);
            registerMethod("enableXREConnectionRetention",
                    &SystemServices::enableXREConnectionRetention, this);
            registerMethod("getSystemVersions", &SystemServices::getSystemVersions, this);
            registerMethod("setNetworkStandbyMode", &SystemServices::setNetworkStandbyMode, this);
            registerMethod("getNetworkStandbyMode", &SystemServices::getNetworkStandbyMode, this);
            registerMethod("getPowerStateIsManagedByDevice", &SystemServices::getPowerStateIsManagedByDevice, this);
    	    registerMethod("setTerritory", &SystemServices::setTerritory, this);
	    registerMethod("getTerritory", &SystemServices::getTerritory, this);
#ifdef ENABLE_SET_WAKEUP_SRC_CONFIG
            registerMethod("setWakeupSrcConfiguration", &SystemServices::setWakeupSrcConfiguration, this);
#endif //ENABLE_SET_WAKEUP_SRC_CONFIG

            // version 2 APIs
            registerMethod(_T("getTimeZones"), &SystemServices::getTimeZones, this);
#ifdef ENABLE_DEEP_SLEEP
            registerMethod(_T("getWakeupReason"), &SystemServices::getWakeupReason, this);
            registerMethod(_T("getLastWakeupKeyCode"), &SystemServices::getLastWakeupKeyCode, this);
#endif
            registerMethod("uploadLogs", &SystemServices::uploadLogs, this);

            registerMethod("uploadLogsAsync", &SystemServices::uploadLogsAsync, this);
            registerMethod("abortLogUpload", &SystemServices::abortLogUpload, this);

            registerMethod("getPowerStateBeforeReboot", &SystemServices::getPowerStateBeforeReboot,
                    this);
            registerMethod("getLastFirmwareFailureReason", &SystemServices::getLastFirmwareFailureReason, this);
            registerMethod("setOptOutTelemetry", &SystemServices::setOptOutTelemetry, this);
            registerMethod("isOptOutTelemetry", &SystemServices::isOptOutTelemetry, this);
            registerMethod("fireFirmwarePendingReboot", &SystemServices::fireFirmwarePendingReboot, this);
            registerMethod("setFirmwareRebootDelay", &SystemServices::setFirmwareRebootDelay, this);
            registerMethod("setFirmwareAutoReboot", &SystemServices::setFirmwareAutoReboot, this);
#ifdef ENABLE_SYSTEM_GET_STORE_DEMO_LINK
            registerMethod("getStoreDemoLink", &SystemServices::getStoreDemoLink, this);
#endif
            registerMethod("deletePersistentPath", &SystemServices::deletePersistentPath, this);
            Register<JsonObject, PlatformCaps>("getPlatformConfiguration",
                &SystemServices::getPlatformConfiguration, this);
            GetHandler(2)->Register<JsonObject, PlatformCaps>("getPlatformConfiguration",
                &SystemServices::getPlatformConfiguration, this);
	    registerMethod("getFriendlyName", &SystemServices::getFriendlyName, this);
            registerMethod("setFriendlyName", &SystemServices::setFriendlyName, this);

        }

        SystemServices::~SystemServices()
        {
            regfree (&m_regexUnallowedChars);
        }

        const string SystemServices::Initialize(PluginHost::IShell* service)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            m_shellService = service;
            m_shellService->AddRef();

            //Initialise timer with interval and callback function.
            m_operatingModeTimer.setInterval(updateDuration, MODE_TIMER_UPDATE_INTERVAL);

            //first boot? then set to NORMAL mode
            if (!m_temp_settings.contains("mode") && m_currentMode == "") {
                JsonObject mode,param,response;
                param["duration"] = -1;
                param["mode"] = MODE_NORMAL;
                mode["modeInfo"] = param;

                LOGINFO("first boot so setting mode to '%s' ('%s' does not contain(\"mode\"))\n",
                        (param["mode"].String()).c_str(), SYSTEM_SERVICE_TEMP_FILE);

                setMode(mode, response);
            } else if (m_currentMode.empty()) {
                JsonObject mode,param,response;
                param["duration"] = m_temp_settings.getValue("mode_duration");
                param["mode"] = m_temp_settings.getValue("mode");
                mode["modeInfo"] = param;

                LOGINFO("receiver restarted so setting mode:%s duration:%d\n",
                        (param["mode"].String()).c_str(), (int)param["duration"].Number());

                setMode(mode, response);
            }

#ifdef DISABLE_GEOGRAPHY_TIMEZONE
            std::string timeZone = getTimeZoneDSTHelper();

            if (!timeZone.empty()) {
                std::string tzenv = ":";
                tzenv += timeZone;
                Core::SystemInfo::SetEnvironment(_T("TZ"), tzenv.c_str());
            }
#endif
            RFC_ParamData_t param = {0};
            WDMP_STATUS status = getRFCParameter((char*)"thunderapi", TR181_SYSTEM_FRIENDLY_NAME, &param);
            if(WDMP_SUCCESS == status && param.type == WDMP_STRING)
            {
                m_friendlyName = param.value;
                LOGINFO("Success Getting the friendly name value :%s \n",m_friendlyName.c_str());
            }

            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void SystemServices::Deinitialize(PluginHost::IShell*)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            SystemServices::_instance = nullptr;
            m_shellService->Release();
            m_shellService = nullptr;
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void SystemServices::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterCall(IARM_BUS_COMMON_API_SysModeChange, _SysModeChange));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, _systemStateChanged));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _powerEventHandler));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_REBOOTING, _powerEventHandler));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED, _powerEventHandler));
                
                
#ifdef ENABLE_THERMAL_PROTECTION
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, _thermMgrEventsHandler));
#endif //ENABLE_THERMAL_PROTECTION
            }
	    
            RFC_ParamData_t param = {0};
            WDMP_STATUS status = getRFCParameter(NULL, RFC_PWRMGR2, &param);
            if(WDMP_SUCCESS == status && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
            {
                m_isPwrMgr2RFCEnabled = true;
            }
        }

        void SystemServices::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, _systemStateChanged));
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _powerEventHandler));
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_REBOOTING, _powerEventHandler));
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED,_powerEventHandler ));

    #ifdef ENABLE_THERMAL_PROTECTION
                IARM_CHECK( IARM_Bus_RemoveEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, _thermMgrEventsHandler));
    #endif //ENABLE_THERMAL_PROTECTION
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

#ifdef DEBUG
        /**
         * @brief : sampleAPI
         */
        uint32_t SystemServices::sampleAPI(const JsonObject& parameters,
                JsonObject& response)
        {
            response["sampleAPI"] = "Success";
            /* Kept for debug purpose/future reference. */
            sendNotify(EVT_ONSYSTEMSAMPLEEVENT, parameters);
            returnResponse(true);
        }
#endif /* DEBUG */

        uint32_t SystemServices::requestSystemReboot(const JsonObject& parameters,
                JsonObject& response)
        {
            int32_t nfxResult = E_NOK;
            string customReason = "No custom reason provided";
            string otherReason = "No other reason supplied";
            bool result = false;

            nfxResult = system("pgrep nrdPluginApp");
            if (E_OK == nfxResult) {
                LOGINFO("SystemService shutting down Netflix...\n");
                nfxResult = system("pkill nrdPluginApp");
                if (E_OK == nfxResult) {
                    //give Netflix process some time to terminate gracefully.
                    sleep(10);
                } else {
                    LOGINFO("SystemService unable to shutdown Netflix \
                            process. nfxResult = %ld\n", (long int)nfxResult);
                }
            }

            if (parameters.HasLabel("rebootReason")) {
                customReason = parameters["rebootReason"].String();
                otherReason = customReason;
            }

            IARM_Bus_PWRMgr_RebootParam_t rebootParam;
            strncpy(rebootParam.requestor, "SystemServices", sizeof(rebootParam.requestor));
            strncpy(rebootParam.reboot_reason_custom, customReason.c_str(), sizeof(rebootParam.reboot_reason_custom));
            strncpy(rebootParam.reboot_reason_other, otherReason.c_str(), sizeof(rebootParam.reboot_reason_other));
            LOGINFO("requestSystemReboot: custom reason: %s, other reason: %s\n", rebootParam.reboot_reason_custom,
                rebootParam.reboot_reason_other);

            IARM_Result_t iarmcallstatus = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                    IARM_BUS_PWRMGR_API_Reboot, &rebootParam, sizeof(rebootParam));
            if(IARM_RESULT_SUCCESS != iarmcallstatus) {
                LOGWARN("requestSystemReboot: IARM_BUS_PWRMGR_API_Reboot failed with code %d.\n", iarmcallstatus); 
            }
            response["IARM_Bus_Call_STATUS"] = static_cast <int32_t> (iarmcallstatus);
            result = true;
            returnResponse(result);
        }//end of requestSystemReboot

        /*
         * @brief This function delays the reboot in seconds.
         * This will internally sets the tr181 fwDelayReboot parameter.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.System.2.setFirmwareRebootDelay",
         *                  "params":{"delaySeconds": int seconds}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t SystemServices::setFirmwareRebootDelay(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            uint32_t delay_in_sec = 0;

            if ( parameters.HasLabel("delaySeconds") ){
                /* get the value */
                delay_in_sec = static_cast<unsigned int>(parameters["delaySeconds"].Number());

                /* we can delay with max 24 Hrs = 86400 sec */
                if (delay_in_sec > 0 && delay_in_sec <= MAX_REBOOT_DELAY ){

                    const char * set_rfc_val=(parameters["delaySeconds"].String()).c_str();

                    LOGINFO("set_rfc_value %s\n",set_rfc_val);

                    /*set tr181Set command from here*/
                    WDMP_STATUS status = setRFCParameter((char*)"thunderapi",
                            TR181_FW_DELAY_REBOOT, set_rfc_val, WDMP_INT);
                    if ( WDMP_SUCCESS == status ){
                        result=true;
                        LOGINFO("Success Setting setFirmwareRebootDelay value\n");
                    }
                    else {
                        LOGINFO("Failed Setting setFirmwareRebootDelay value %s\n",getRFCErrorString(status));
                    }
                }
                else {
                    /* we didnt get a valid Auto Reboot delay */
                    LOGERR("Invalid setFirmwareRebootDelay Value Max.Value is 86400 sec\n");
                }
            }
            else {
                /* havent got the correct label */
                LOGERR("setFirmwareRebootDelay Missing Key Values\n");
                populateResponseWithError(SysSrv_MissingKeyValues,response);
            }
            returnResponse(result);
        }

        /*
         * @brief This function Enable/Disable the AutReboot Feature.
         * This will internally sets the tr181 AutoReboot.Enable to True/False.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.System.2.setFirmwareAutoReboot",
         *                  "params":{"enable": bool }}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t SystemServices::setFirmwareAutoReboot(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            bool enableFwAutoreboot = false;

           if ( parameters.HasLabel("enable") ){
               /* get the value */
               enableFwAutoreboot = (parameters["enable"].Boolean());
               LOGINFO("setFirmwareAutoReboot : %s\n",(enableFwAutoreboot)? "true":"false");

               const char *set_rfc_val = (parameters["enable"].String().c_str());

               /* set tr181Set command from here */
               WDMP_STATUS status = setRFCParameter((char*)"thunderapi",
                       TR181_AUTOREBOOT_ENABLE,set_rfc_val,WDMP_BOOLEAN);
               if ( WDMP_SUCCESS == status ){
                   result=true;
                   LOGINFO("Success Setting the setFirmwareAutoReboot value\n");
               }
               else {
                   LOGINFO("Failed Setting the setFirmwareAutoReboot value %s\n",getRFCErrorString(status));
               }
           }
           else {
               /* havent got the correct label */
               LOGERR("setFirmwareAutoReboot Missing Key Values\n");
               populateResponseWithError(SysSrv_MissingKeyValues,response);
           }
           returnResponse(result);
        }

        /*
         * @brief This function notifies about pending Reboot.
         * This will internally set 120 sec and trigger event to application.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.System.2.fireFirmwarePendingReboot",
         *                  "params":{}}
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":true}}
         * @return: Core::<StatusCode>
         */

        uint32_t SystemServices::fireFirmwarePendingReboot(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            int seconds = 600; /* 10 Minutes to Reboot */

            /* trigger event saying we are in Maintenance Window */

            /* check if we have valid instance */
            if ( _instance ){
                /* clear any older values, Reset the fwDelayReboot = 0 */
                LOGINFO("Reset Older FwDelayReboot to 0, if any\n");

                WDMP_STATUS status = setRFCParameter((char*)"thunderapi",
                        TR181_FW_DELAY_REBOOT,"0", WDMP_INT);

                /* call the event handler if reset SUCCESS */
                if ( WDMP_SUCCESS == status ){
                    /* trigger event saying we are in Maintenance Window */
                    _instance->onFirmwarePendingReboot(seconds);
                    result=true;
                }
                else {
                    LOGINFO("Failed to reset FwDelayReboot due to %s\n",getRFCErrorString(status));
                }
            }
            else {
                LOGERR("_instance in fireFirmwarePendingReboot is NULL.\n");
            }

            returnResponse(result);
        }

        /*
         * @brief : send event when system is in maintenance window
         * @param1[in]  : int seconds
         */

        void SystemServices::onFirmwarePendingReboot(int seconds)
        {
            JsonObject params;
            params["fireFirmwarePendingReboot"] = seconds;
            LOGINFO("Notifying onFirmwarePendingReboot received \n");
            sendNotify(EVT_ONFWPENDINGREBOOT, params);
        }

        /***
         * @brief : send notification when system power state is changed
         *
         * @param1[in]  : powerState
         * @param2[out] : {"jsonrpc": "2.0",
         *		"method": "org.rdk.SystemServices.events.1.onSystemPowerStateChanged",
         *		"param":{"powerState": <string new power state mode>}}
         */
        void SystemServices::onSystemPowerStateChanged(string currentPowerState, string powerState)
        {

            if ("LIGHT_SLEEP" == powerState || "STANDBY" == powerState) {
                if ("ON" == currentPowerState) {
                    RFC_ParamData_t param = {0};
                    WDMP_STATUS status = getRFCParameter(NULL, RFC_LOG_UPLOAD, &param);
                    if(WDMP_SUCCESS == status && param.type == WDMP_BOOLEAN && (strncasecmp(param.value,"true",4) == 0))
                    {
                        JsonObject p;
                        JsonObject r;
                        uploadLogsAsync(p, r);
                    }
                }
            } else if ("DEEP_SLEEP" == powerState) {

                pid_t uploadLogsPid = -1;

                {
                    lock_guard<mutex> lck(m_uploadLogsMutex);
                    uploadLogsPid = m_uploadLogsPid;
                }

                if (-1 != uploadLogsPid)
                {
                    JsonObject p;
                    JsonObject r;
                    abortLogUpload(p, r);
                }
            }

            JsonObject params;
            params["powerState"] = powerState;
            params["currentPowerState"] = currentPowerState;
            LOGWARN("power state changed from '%s' to '%s'", currentPowerState.c_str(), powerState.c_str());
            sendNotify(EVT_ONSYSTEMPOWERSTATECHANGED, params);
        }

        void SystemServices::onPwrMgrReboot(string requestedApp, string rebootReason)
        {
            JsonObject params;
            params["requestedApp"] = requestedApp;
            params["rebootReason"] = rebootReason;

            sendNotify(EVT_ONREBOOTREQUEST, params);
        }

        void SystemServices::onNetorkModeChanged(bool bNetworkStandbyMode)
        {
            m_networkStandbyMode = bNetworkStandbyMode;
            m_networkStandbyModeValid = true;
            JsonObject params;
            params["nwStandby"] = bNetworkStandbyMode;
            sendNotify(EVT_ONNETWORKSTANDBYMODECHANGED , params);
        }

        /**
         * @breif : to enable Moca Settings
         * @param1[in] : {"params":{"value":true}}
         * @param2[out] :  {"success":<bool>}
         */
        uint32_t SystemServices::requestEnableMoca(const JsonObject& parameters,
                JsonObject& response)
        {
            int32_t eRetval = E_NOK;
            bool enableMoca = false;
            ofstream mocaFile;
             if (parameters.HasLabel("value")) {
                 enableMoca = parameters["value"].Boolean();
                 if (enableMoca) {
                 mocaFile.open(MOCA_FILE, ios::out);
                     if (mocaFile) {
                         mocaFile.close();
                         /* TODO: replace system() */
                         eRetval = system("/etc/init.d/moca_init start");
                     } else {
                         LOGERR("moca file open failed\n");
                         populateResponseWithError(SysSrv_FileAccessFailed, response);
                     }
                 } else {
                     std::remove(MOCA_FILE);
                     if (!Utils::fileExists(MOCA_FILE)) {
                         /* TODO: replace system() */
                         eRetval = system("/etc/init.d/moca_init start");
                     } else {
                         LOGERR("moca file remove failed\n");
                         populateResponseWithError(SysSrv_FileAccessFailed, response);
                     }
                 }
            } else {
                populateResponseWithError(SysSrv_MissingKeyValues, response);
            }
            LOGERR("eRetval = %d\n", eRetval);
            returnResponse((E_OK == eRetval)? true: false);
        } //End of requestEnableMoca

        /**
         * @brief  : To fetch system uptime
         * @param2[out] : {"result":{"systemUptime":"378641.03","success":true}}
         */
        uint32_t SystemServices::requestSystemUptime(const JsonObject& parameters,
                JsonObject& response)
        {
            struct timespec time;
            bool result = false;

            if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
            {
                float uptime = (float)time.tv_sec + (float)time.tv_nsec / 1e9;
                std::string value = std::to_string(uptime);
                value = value.erase(value.find_last_not_of("0") + 1);

                if (value.back() == '.')
                    value += '0';

                response["systemUptime"] = value;
                LOGINFO("uptime is %s seconds", value.c_str());
                result = true;
                }
            else
                LOGERR("unable to evaluate uptime by clock_gettime");

            returnResponse(result);
        }

        /**
         * @brief : API to query DeviceInfo details
         *
         * @param1[in]  : {"params":{"params":["<key>"]}}
         * @param2[out] : "result":{<key>:<Device Info Details>,"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getDeviceInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
            string queryParams;

            if (parameters.HasLabel("params")) {
                queryParams = parameters["params"].String();
                removeCharsFromString(queryParams, "[\"]");

                regmatch_t  m_regmatchUnallowedChars[1];
                if (REG_NOERROR == regexec(&m_regexUnallowedChars, queryParams.c_str(), 1, m_regmatchUnallowedChars, 0))
                {
                    response["message"] = "Input has unallowable characters";
                    LOGERR("Input has unallowable characters: '%s'", queryParams.c_str());

                    returnResponse(false);
                }

           }

            // there is no /tmp/.make from /lib/rdk/getDeviceDetails.sh, but it can be taken from /etc/device.properties
            if (queryParams.empty() || queryParams == "make") {

                if (!Utils::fileExists(DEVICE_PROPERTIES_FILE)) {
                    populateResponseWithError(SysSrv_FileNotPresent, response);
                    returnResponse(retAPIStatus);
                }

                char buf[1024];

                FILE *f = fopen(DEVICE_PROPERTIES_FILE, "r");

                if(!f) {
                    LOGWARN("failed to open %s:%s", DEVICE_PROPERTIES_FILE, strerror(errno));
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                    returnResponse(retAPIStatus);
                }

                std::string line;
                std::string make;
                while(fgets(buf, sizeof(buf), f) != NULL) {
                    line = buf;
                    size_t eq = line.find_first_of("=");

                    if (std::string::npos != eq) {
                        std::string key = line.substr(0, eq);

                        if (key == "MFG_NAME") {
                            make = line.substr(eq + 1);
                            Utils::String::trim(make);
                            break;
                        }
                    }
                }

                fclose(f);

                if (make.size() > 0) {
                    response["make"] = make;
                    retAPIStatus = true;
                } else {
                    populateResponseWithError(SysSrv_MissingKeyValues, response);
                }

                if (!queryParams.empty()) {
                    returnResponse(retAPIStatus);
                }
            }

#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
            if (!queryParams.compare(MODEL_NAME) || !queryParams.compare(HARDWARE_ID)) {
                returnResponse(getManufacturerData(queryParams, response));
		}

	    if(!queryParams.compare(FRIENDLY_ID))
	    {
		    if(getModelName(queryParams, response))
			    returnResponse(true);
            }
#endif

            std::string cmd = DEVICE_INFO_SCRIPT;
            if (!queryParams.empty()) {
                cmd += " ";
                cmd += queryParams;
            }

            std::string res = Utils::cRunScript(cmd.c_str());

            if (res.size() > 0) {
                std::string model_number;
                if (queryParams.empty()) {
                    retAPIStatus = true;

                    std::stringstream ss(res);
                    std::string line;
                    while(std::getline(ss, line))
                    {
                        size_t eq = line.find_first_of("=");

                        if (std::string::npos != eq)
                        {
                            std::string key = line.substr(0, eq);
                            std::string value = line.substr(eq + 1);

                            response[key.c_str()] = value;

                            // some tweaks for backward compatibility
                            if (key == "imageVersion") {
                                response["version"] = value; 
                                response["software_version"] = value;
                            }
                            else if (key == "cableCardVersion") {
                                response["cable_card_firmware_version"] = value;
                            }
                            else if (key == "model_number") {
                                model_number = value;
                            }
                        }
                    }
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
                    queryParams = FRIENDLY_ID;
                    getModelName(queryParams, response);
#endif
                } else {
                    retAPIStatus = true;
                    Utils::String::trim(res);
                        response[queryParams.c_str()] = res;
                    }
                }
            returnResponse(retAPIStatus);
        }
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO


	bool SystemServices::getModelName(const string& parameter, JsonObject& response)
	{
		LOGWARN("SystemService getDeviceInfo query %s", parameter.c_str());
		IARM_Bus_MFRLib_GetSerializedData_Param_t param;
		param.bufLen = 0;
		param.type = mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME;
		IARM_Result_t result = IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param));
		param.buffer[param.bufLen] = '\0';
		LOGWARN("SystemService getDeviceInfo param type %d result %s", param.type, param.buffer);
		bool status = false;
		if (result == IARM_RESULT_SUCCESS) {
			response[parameter.c_str()] = string(param.buffer);
			status = true;
		}
		else{
			LOGWARN("SystemService getDeviceInfo - Manufacturer Data Read Failed");
		}
		return status;
	}

        /***
         * @brief : To retrieve Manufacturing Serial Number.
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"mfgSerialNumber":"<string>","success":<bool>}}
         */
        uint32_t SystemServices::getMfgSerialNumber(const JsonObject& parameters, JsonObject& response)
        {
            LOGWARN("SystemService getMfgSerialNumber query");

            if (m_MfgSerialNumberValid) {
                response["mfgSerialNumber"] = m_MfgSerialNumber;
                LOGWARN("Got cached MfgSerialNumber %s", m_MfgSerialNumber.c_str());
                returnResponse(true);
            }

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_MANUFACTURING_SERIALNUMBER;
            IARM_Result_t result = IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param));
            param.buffer[param.bufLen] = '\0';

            bool status = false;
            if (result == IARM_RESULT_SUCCESS) {
                response["mfgSerialNumber"] = string(param.buffer);
                status = true;

                m_MfgSerialNumber = string(param.buffer);
                m_MfgSerialNumberValid = true;

                LOGWARN("SystemService getMfgSerialNumber Manufacturing Serial Number: %s", param.buffer);
            } else {
                LOGERR("SystemService getMfgSerialNumber Manufacturing Serial Number: NULL");
            }

            returnResponse(status);;
        }

        bool SystemServices::getManufacturerData(const string& parameter, JsonObject& response)
        {
            LOGWARN("SystemService getDeviceInfo query %s", parameter.c_str());

	    if (m_ManufacturerDataModelNameValid && !parameter.compare(MODEL_NAME)) {
		    response[parameter.c_str()] = m_ManufacturerDataModelName;
		    LOGWARN("Got cached ManufacturerData %s", m_ManufacturerDataModelName.c_str());
		    return true;
	    }

	    if (m_ManufacturerDataHardwareIdValid && !parameter.compare(HARDWARE_ID)) {
		    response[parameter.c_str()] = m_ManufacturerDataHardwareID;
		    LOGWARN("Got cached ManufacturerData %s", m_ManufacturerDataHardwareID.c_str());
		    return true;
	    }


            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_MANUFACTURER;
            if (!parameter.compare(MODEL_NAME)) {
                param.type = mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME;
            } else if (!parameter.compare(HARDWARE_ID)) {
                param.type = mfrSERIALIZED_TYPE_HWID;
            }
            IARM_Result_t result = IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_GetSerializedData, &param, sizeof(param));
            param.buffer[param.bufLen] = '\0';

            LOGWARN("SystemService getDeviceInfo param type %d result %s", param.type, param.buffer);

            bool status = false;
            if (result == IARM_RESULT_SUCCESS) {
                response[parameter.c_str()] = string(param.buffer);
                status = true;
		if(!parameter.compare(MODEL_NAME)){
			m_ManufacturerDataModelName = param.buffer;
			m_ManufacturerDataModelNameValid = true;
		}
		else if (!parameter.compare(HARDWARE_ID)) {
			m_ManufacturerDataHardwareID = param.buffer;
			m_ManufacturerDataHardwareIdValid = true;
		}
            } else {
                populateResponseWithError(SysSrv_ManufacturerDataReadFailed, response);
            }

            return status;
        }
#endif

        /***
         * @brief : Checks if Moca is Enabled or Not.
         *
         * @param1[in]  : {"params":{}}
         * @param2[out] : "result":{"mocaEnabled":<bool>,"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::queryMocaStatus(const JsonObject& parameters,
                JsonObject& response)
        {
            response["mocaEnabled"] = getMocaStatus();
            returnResponse(true);
        }

        /***
         * @brief : sends notification when system mode has changed.
         *
         * @param1[in]  : state
         * @param2[out] : {"mode": <string_mode>}
         */
        void SystemServices::onSystemModeChanged(string mode)
        {
            JsonObject params;
            params["mode"] = mode;
            LOGINFO("mode changed to '%s'\n", mode.c_str());
            sendNotify(EVT_ONSYSTEMMODECHANGED, params);
        }

        /***
         * @brief Initiates a firmware update.
         * This has no affect if update is not available. The State Observer API
         * may be used to listen to firmware update events.
         *
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::updateFirmware(const JsonObject& parameters,
                JsonObject& response)
        {
            LOGWARN("SystemService updatingFirmware\n");
            string command("/lib/rdk/swupdate_utility.sh 0 4 >> /opt/logs/swupdate.log &");
            Utils::cRunScript(command.c_str());
            returnResponse(true);
        }

        /***
         * @brief : Returns mode Information, defines two parameters mode and duration.
         *
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"modeInfo":{"mode":"<string>","duration":<int>},"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getMode(const JsonObject& parameters,
                JsonObject& response)
        {
            JsonObject modeInfo;
            LOGWARN("current mode: '%s', duration: %d\n",
                    m_currentMode.c_str(), m_remainingDuration);
            modeInfo["mode"] = m_currentMode.c_str();
            modeInfo["duration"] = m_remainingDuration;
            response["modeInfo"] = modeInfo;
            returnResponse(true);
        }

        /***
         * @brief : Sets the bootloader pattern to MFR. 
         * @param1[in]  : {"pattern":"<string>"}
         * @param2[out] : {"result":{"success":<bool>}}
         * @return              : Core::<StatusCode>
         */
        uint32_t SystemServices::setBootLoaderPattern(const JsonObject& parameters,
                JsonObject& response)
        {
                returnIfParamNotFound(parameters, "pattern");
                bool status = false;
                IARM_Bus_MFRLib_SetBLPattern_Param_t mfrparam;
                mfrparam.pattern = mfrBL_PATTERN_NORMAL;
                string strBLPattern = parameters["pattern"].String();
                if (strBLPattern == "NORMAL") {
                    mfrparam.pattern = mfrBL_PATTERN_NORMAL;
                    status = true;
                }
                else if (strBLPattern == "SILENT") {
                    mfrparam.pattern = mfrBL_PATTERN_SILENT;
                    status = true;
                }
                else if (strBLPattern == "SILENT_LED_ON") {
                    mfrparam.pattern = mfrBL_PATTERN_SILENT_LED_ON;
                    status = true;
                }
                LOGWARN("setBootLoaderPattern :%d \n", mfrparam.pattern);
                if(status == true)
                {
                   if (IARM_RESULT_SUCCESS != IARM_Bus_Call(IARM_BUS_MFRLIB_NAME, IARM_BUS_MFRLIB_API_SetBootLoaderPattern, (void *)&mfrparam, sizeof(mfrparam))){
                        status = false;
                   }
                }
                returnResponse(status);
        }

        /***
         * @brief : Sets the mode of the STB. The object consists of two properties, mode (String) and
         * duration (Integer) mode can be one of following:
         * - NORMAL - the STB is operating in normal mode
         * - EAS - the STB is operating in EAS mode. This mode is set when the device needs
         *   to perform certain tasks when entering EAS mode, such as setting the clock display,
         *   or preventing the user from using the diagnostics menu.
         * - WAREHOUSE - the STB is operating in warehouse mode.
         *
         * @param1[in]	: {"modeInfo":{"mode":"<string>","duration":<int>}}
         * @param2[out]	: {"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::setMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool changeMode  = true;
            JsonObject param;
            std::string oldMode = m_currentMode;
            bool result = true;

            if (parameters.HasLabel("modeInfo")) {
                param.FromString(parameters["modeInfo"].String());
                if (param.HasLabel("duration") && param.HasLabel("mode")) {
                    int duration = param["duration"].Number();
                    std::string newMode = param["mode"].String();

                    LOGWARN("request to switch to mode '%s' from mode '%s' \
                            with duration %d\n", newMode.c_str(),
                            oldMode.c_str(), duration);

                    if (MODE_NORMAL != newMode && MODE_WAREHOUSE != newMode &&
                            MODE_EAS != newMode) {
                        LOGERR("value of new mode is incorrect, therefore \
                                current mode '%s' not changed.\n", oldMode.c_str());
                        returnResponse(false);
                    }
                    if (MODE_NORMAL == m_currentMode && (0 == duration ||
                                (0 != duration && MODE_NORMAL == newMode))) {
                        changeMode = false;
                    } else if (MODE_NORMAL != newMode && 0 != duration) {
                        m_currentMode = newMode;
                        duration < 0 ? stopModeTimer() : startModeTimer(duration);
                    } else {
                        m_currentMode = MODE_NORMAL;
                        stopModeTimer();
                    }

                    if (changeMode) {
                        IARM_Bus_CommonAPI_SysModeChange_Param_t modeParam;
                        stringToIarmMode(oldMode, modeParam.oldMode);
                        stringToIarmMode(m_currentMode, modeParam.newMode);

                        if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_DAEMON_NAME,
                                    "DaemonSysModeChange", &modeParam, sizeof(modeParam))) {
                            LOGWARN("switched to mode '%s'\n", m_currentMode.c_str());

                            if (MODE_NORMAL != m_currentMode && duration < 0) {
                                LOGWARN("duration is negative, therefore \
                                        mode timer stopped and Receiver will keep \
                                        mode '%s', untill changing it in next call",
                                        m_currentMode.c_str());
                            }
                        } else {
                            stopModeTimer();
                            m_currentMode = MODE_NORMAL;
                            LOGERR("failed to switch to mode '%s'. Receiver \
                                    forced to switch to '%s'", newMode.c_str(), m_currentMode.c_str());
                            result = false;
                        }

                        string command = "";
                        if (MODE_WAREHOUSE == m_currentMode) {
                            command = "touch ";
                        } else {
                            command = "rm -f ";
                        }
                        command += WAREHOUSE_MODE_FILE;
                        /* TODO: replace with system alternate. */
                        int sysStat = system(command.c_str());
                        LOGINFO("system returned %d\n", sysStat);
                        //set values in temp file so they can be restored in receiver restarts / crashes
                        m_temp_settings.setValue("mode", m_currentMode);
                        m_temp_settings.setValue("mode_duration", m_remainingDuration);
                    } else {
                        LOGWARN("Current mode '%s' not changed", m_currentMode.c_str());
                    }
                } else {
                    populateResponseWithError(SysSrv_MissingKeyValues, response);
                    result = false;
                }
            } else {
                populateResponseWithError(SysSrv_MissingKeyValues, response);
                result = false;
            }

            returnResponse(result);
        }

        void SystemServices::startModeTimer(int duration)
        {
            m_remainingDuration = duration;
            m_operatingModeTimer.start();
            //set values in temp file so they can be restored in receiver restarts / crashes
            m_temp_settings.setValue("mode_duration", m_remainingDuration);
        }

        void SystemServices::stopModeTimer()
        {
            m_remainingDuration = 0;
            m_operatingModeTimer.stop();

            //set values in temp file so they can be restored in receiver restarts / crashes
            // TODO: query & confirm time duration range.
            m_temp_settings.setValue("mode_duration", m_remainingDuration);
        }

        /**
         * @brief This function is used to update duration.
         */
        void SystemServices::updateDuration()
        {
            if (m_remainingDuration > 0) {
                m_remainingDuration--;
            } else {
                m_operatingModeTimer.stop();
                JsonObject parameters, param, response;
                param["mode"] = "NORMAL";
                param["duration"] = 0;
                parameters["modeInfo"] = param;
                if (_instance) {
                    _instance->setMode(parameters,response);
                } else {
                    LOGERR("_instance is NULL.\n");
                }
            }

            //set values in temp file so they can be restored in receiver restarts / crashes
            m_temp_settings.setValue("mode_duration", m_remainingDuration);
        }

        /***
         * @brief  : Event handler of Firmware Update Info status check.
         * @param1[in] : New FW Version
         * @param2[in] : HTTP Query Status Code
         * @param3[in] : Check status code
         * @param4[in] : Current FW Version
         * @param5[in] : HTTP Payload Response
         */
        void SystemServices::reportFirmwareUpdateInfoReceived(string
                firmwareUpdateVersion, int httpStatus, bool success,
                string firmwareVersion, string responseString)
        {
            JsonObject params;
            params["status"] = httpStatus;
            params["responseString"] = responseString.c_str();
            params["rebootImmediately"] = false;

            JsonObject xconfResponse;
            if(!responseString.empty() && xconfResponse.FromString(responseString))
            {
                params["rebootImmediately"] = xconfResponse["rebootImmediately"];
            }

            if(httpStatus == STATUS_CODE_NO_SWUPDATE_CONF)
            {
                // Empty /opt/swupdate.conf
                params["status"] = 0;
                params["updateAvailable"] = false;
                params["updateAvailableEnum"] = static_cast<int>(FWUpdateAvailableEnum::EMPTY_SW_UPDATE_CONF);
                params["success"] = true;
            }
            else if(httpStatus == 404)
            {
                // if XCONF server returns 404 there is no FW available to download
                params["updateAvailable"] = false;
                params["updateAvailableEnum"] = static_cast<int>(FWUpdateAvailableEnum::FW_MATCH_CURRENT_VER);
                params["success"] = true;
            }
            else
            {
                FWUpdateAvailableEnum updateAvailableEnum = FWUpdateAvailableEnum::NO_FW_VERSION;
                bool bUpdateAvailable = false;
                if (firmwareUpdateVersion.length() > 0) {
                    params["firmwareUpdateVersion"] = firmwareUpdateVersion.c_str();
                    if (firmwareUpdateVersion.compare(firmwareVersion)) {
                        updateAvailableEnum = FWUpdateAvailableEnum::FW_UPDATE_AVAILABLE;
                        bUpdateAvailable = true;
                    } else {
                        updateAvailableEnum = FWUpdateAvailableEnum::FW_MATCH_CURRENT_VER;
                    }
                } else {
                    params["firmwareUpdateVersion"] = "";
                    updateAvailableEnum = FWUpdateAvailableEnum::NO_FW_VERSION;
                }
                params["updateAvailable"] = bUpdateAvailable ;
                params["updateAvailableEnum"] = static_cast<int>(updateAvailableEnum);
                params["success"] = success;
            }

            string jsonLog;
            params.ToString(jsonLog);
            LOGWARN("result: %s\n", jsonLog.c_str());
            sendNotify(EVT_ONFIRMWAREUPDATEINFORECEIVED, params);
        }

        /***
         * @brief : Firmware Update Info Worker.
         */
        void SystemServices::firmwareUpdateInfoReceived(void)
        {
            string env = "";
            string firmwareVersion;
            if (_instance) {
                firmwareVersion = _instance->getStbVersionString();
            } else {
                LOGERR("_instance is NULL.\n");
            }

            LOGWARN("SystemService firmwareVersion %s\n", firmwareVersion.c_str());

            if (true == findCaseInsensitive(firmwareVersion, "DEV"))
                env = "DEV";
            else if (true == findCaseInsensitive(firmwareVersion, "VBN"))
                env = "VBN";
            else if (true == findCaseInsensitive(firmwareVersion, "PROD"))
                env = "PROD";
            else if (true == findCaseInsensitive(firmwareVersion, "CQA"))
                env = "CQA";

            std::string response;
            firmwareUpdate _fwUpdate;
            
            _fwUpdate.success = false;
            _fwUpdate.httpStatus = 0;

            bool bFileExists = false;
            string xconfOverride; 
            if(env != "PROD")
            {
                xconfOverride = getXconfOverrideUrl(bFileExists);
                if(bFileExists && xconfOverride.empty())
                {
                    // empty /opt/swupdate.conf. Don't initiate FW download
                    LOGWARN("Empty /opt/swupdate.conf. Skipping FW upgrade check with xconf");
                    if (_instance) {
                        _instance->reportFirmwareUpdateInfoReceived("",
                        STATUS_CODE_NO_SWUPDATE_CONF, true, "", response);
                    }
                    return;
                }
            }

            std::system("/lib/rdk/xconfImageCheck.sh  >> /opt/logs/wpeframework.log");

            //get xconf http code
            string httpCodeStr = Utils::cRunScript("cat /tmp/xconf_httpcode_thunder.txt");
            if(!httpCodeStr.empty())
            {
                try
                {
                    _fwUpdate.httpStatus = std::stoi(httpCodeStr);
                }
                catch(const std::exception& e)
                {
                    LOGERR("exception in converting xconf http code %s", e.what());
                }
            }

            LOGINFO("xconf http code %d\n", _fwUpdate.httpStatus);

            response = Utils::cRunScript("cat /tmp/xconf_response_thunder.txt");
            LOGINFO("xconf response '%s'\n", response.c_str());
            
            if(!response.empty()) 
            {
                JsonObject httpResp;
                if(httpResp.FromString(response))
                {
                    if(httpResp.HasLabel("firmwareVersion"))
                    {
                        _fwUpdate.firmwareUpdateVersion = httpResp["firmwareVersion"].String();
                        LOGWARN("fwVersion: '%s'\n", _fwUpdate.firmwareUpdateVersion.c_str());
                        _fwUpdate.success = true;
                    }
                    else
                    {
                        LOGERR("Xconf response is not valid json and/or doesn't contain firmwareVersion. '%s'\n", response.c_str());
                        response = "";
                    }
                }
                else
                {
                    LOGERR("Error in parsing xconf json response");
                }
                 
            }
            else
            {
                LOGERR("Unable to open xconf response file");
            }
            

            if (_instance) {
                _instance->reportFirmwareUpdateInfoReceived(_fwUpdate.firmwareUpdateVersion,
                        _fwUpdate.httpStatus, _fwUpdate.success, firmwareVersion, response);
            } else {
                LOGERR("_instance is NULL.\n");
            }
        } //end of event onFirmwareInfoRecived

        /***
         * @brief  : To check Firmware Update Info
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"asyncResponse":true,"success":true}}
         */
        uint32_t SystemServices::getFirmwareUpdateInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            string callGUID = parameters["GUID"].String();
            LOGINFO("GUID = %s\n", callGUID.c_str());
            try
            {
                if (m_getFirmwareInfoThread.get().joinable()) {
                    m_getFirmwareInfoThread.get().join();
                }
                m_getFirmwareInfoThread = Utils::ThreadRAII(std::thread(firmwareUpdateInfoReceived));
                response["asyncResponse"] = true;
                returnResponse(true);
            }
            catch(const std::system_error& e)
            {
                LOGERR("exception in getFirmwareUpdateInfo %s", e.what());
                response["asyncResponse"] = false;
                returnResponse(false);
            }
        } // get FirmwareUpdateInfo

        /***
         * @brief Sets the deep sleep time out period, specified in seconds by invoking the corresponding
         * systemService method. This function used as an interface function in Java script.
         * @param1[in]	: {"jsonrpc":"2.0","id":"3","method":"org.rdk.SystemServices.1.setDeepSleepTimer",
         *				"params":{"seconds":<unsigned int>}}
         * @param2[out]	: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::setDeepSleepTimer(const JsonObject& parameters,
                JsonObject& response)
	{
		bool status = false;
		IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t param;
		if (parameters.HasLabel("seconds")) {
			param.timeout = static_cast<unsigned int>(parameters["seconds"].Number());
			if (param.timeout < 0) {
				param.timeout = 0;
			}
			IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
					IARM_BUS_PWRMGR_API_SetDeepSleepTimeOut, (void *)&param,
					sizeof(param));

			if (IARM_RESULT_SUCCESS == res) {
				status = true;
			} else {
				status = false;
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(status);
        }

        /***
         * @brief Sets the Network standby mode by invoking the corresponding
         * systemService method. This function used as an interface function in Java script.
         * @param1[in]	: {"jsonrpc":"2.0","id":"3","method":"org.rdk.SystemServices.1.setNetworkStandbyMode",
         *				"params":{"nwStandby":<bool>}}
         * @param2[out]	: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
         uint32_t SystemServices::setNetworkStandbyMode (const JsonObject& parameters,
             JsonObject& response)
         {
             bool status = false;
             IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t param;
             if (parameters.HasLabel("nwStandby")) {
                 param.bStandbyMode = parameters["nwStandby"].Boolean();
                 LOGWARN("setNetworkStandbyMode called, with NwStandbyMode : %s\n",
                          (param.bStandbyMode)?("Enabled"):("Disabled"));
                 IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                                        IARM_BUS_PWRMGR_API_SetNetworkStandbyMode, (void *)&param,
                                        sizeof(param));

                 if (IARM_RESULT_SUCCESS == res) {
                     status = true;
                     m_networkStandbyModeValid = false;
                 } else {
                     status = false;
                 }
             } else {
                 populateResponseWithError(SysSrv_MissingKeyValues, response);
             }
             returnResponse(status);
        }

        /***
         * @brief : To retrieve Device Power State.
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"nwStandby":"<bool>","success":<bool>}}
         * @return     : Core::<StatusCode>
         */
        uint32_t SystemServices::getNetworkStandbyMode(const JsonObject& parameters,
            JsonObject& response)
        {
            bool retVal = false;

            if (m_networkStandbyModeValid) {
                response["nwStandby"] = m_networkStandbyMode;
                retVal = true;
                LOGINFO("Got cached NetworkStandbyMode: '%s'", m_networkStandbyMode ? "true" : "false");
            }
            else {
                IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t param;
                IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                                       IARM_BUS_PWRMGR_API_GetNetworkStandbyMode, (void *)&param,
                                       sizeof(param));
                bool nwStandby = param.bStandbyMode;
    
                LOGWARN("getNetworkStandbyMode called, current NwStandbyMode is: %s\n",
                         nwStandby?("Enabled"):("Disabled"));
                response["nwStandby"] = nwStandby;
                if (IARM_RESULT_SUCCESS == res) {
                    retVal = true;
                    m_networkStandbyMode = nwStandby;
                    m_networkStandbyModeValid = true;
                } else {
                    retVal = false;
                }
            }

            returnResponse(retVal);
        }

        /***
         * @brief Sets and persists the preferred standby mode.
         * Invoking this function does not change the power state of the device, but sets only
         * the user preference for preferred action when setPowerState is invoked with a value of "STANDBY".
         *
         * @param1[in]	: {"jsonrpc":"2.0","id":"3","method":"org.rdk.SystemServices.1.setPreferredStandbyMode",
         *				   "params":{"standbyMode":"<string>"}}
         * @param2[out]	: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::setPreferredStandbyMode(const JsonObject& parameters,
                JsonObject& response)
        {
		bool status = false;
		JsonObject param;
		if (parameters.HasLabel("standbyMode")) {
			std::string prefMode = parameters["standbyMode"].String();
			try {
				LOGINFO("Set Preferred Stand by Mode to %s\n", prefMode.c_str());
				const device::SleepMode &mode= device::SleepMode::getInstance(prefMode);
				device::Host::getInstance().setPreferredSleepMode(mode);
				status = true;
			} catch (...) {
				LOGERR("Error setting PreferredStandbyMode\n");
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(status);
        }

        /***
         * @brief Returns the preferred standby mode.
         * Possible values are either "LIGHT_SLEEP" or "DEEP_SLEEP". This Will return
         * an empty string if the preferred mode has not been set.
         *
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"preferredStandbyMode":"<string>","success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getPreferredStandbyMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
            try {
                const device::SleepMode &mode = device::Host::getInstance().getPreferredSleepMode();
                std::string preferredStandbyMode = mode.toString();
                response["preferredStandbyMode"] = preferredStandbyMode;
                status = true;
            } catch (...) {
                LOGERR("Error getting PreferredStandbyMode\n");
                response["preferredStandbyMode"] = "";
            }
            returnResponse(status);
        }

#ifdef ENABLE_DEEP_SLEEP
        /***
         * @brief Returns the deepsleep wakeup reason.
	 * Possible values are "WAKEUP_REASON_IR", "WAKEUP_REASON_RCU_BT"
	 * "WAKEUP_REASON_RCU_RF4CE", WAKEUP_REASON_GPIO", "WAKEUP_REASON_LAN",
	 * "WAKEUP_REASON_WLAN", "WAKEUP_REASON_TIMER", "WAKEUP_REASON_FRONT_PANEL",
	 * "WAKEUP_REASON_WATCHDOG", "WAKEUP_REASON_SOFTWARE_RESET", "WAKEUP_REASON_THERMAL_RESET",
	 * "WAKEUP_REASON_WARM_RESET", "WAKEUP_REASON_COLDBOOT", "WAKEUP_REASON_STR_AUTH_FAILURE",
	 * "WAKEUP_REASON_CEC", "WAKEUP_REASON_PRESENCE", "WAKEUP_REASON_VOICE", "WAKEUP_REASON_UNKNOWN"
         *
         * @param1[in]  : {"params":{"appName":"abc"}}
         * @param2[out] : {"result":{"wakeupReason":<string>","success":<bool>}}
         * @return              : Core::<StatusCode>
         */
        uint32_t SystemServices::getWakeupReason(const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
	    DeepSleep_WakeupReason_t param;
	    std::string wakeupReason = "WAKEUP_REASON_UNKNOWN";

	    IARM_Result_t res = IARM_Bus_Call((m_isPwrMgr2RFCEnabled)? IARM_BUS_PWRMGR_NAME : IARM_BUS_DEEPSLEEPMGR_NAME,
			IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason, (void *)&param,
			sizeof(param));

            if (IARM_RESULT_SUCCESS == res)
            {
                status = true;
                if (param == DEEPSLEEP_WAKEUPREASON_IR) {
                   wakeupReason = "WAKEUP_REASON_IR";
                } else if (param == DEEPSLEEP_WAKEUPREASON_RCU_BT) {
                   wakeupReason = "WAKEUP_REASON_RCU_BT";
                } else if (param == DEEPSLEEP_WAKEUPREASON_RCU_RF4CE) {
                   wakeupReason = "WAKEUP_REASON_RCU_RF4CE";
                } else if (param == DEEPSLEEP_WAKEUPREASON_GPIO) {
                   wakeupReason = "WAKEUP_REASON_GPIO";
                } else if (param == DEEPSLEEP_WAKEUPREASON_LAN) {
                   wakeupReason = "WAKEUP_REASON_LAN";
                } else if (param == DEEPSLEEP_WAKEUPREASON_WLAN) {
                   wakeupReason = "WAKEUP_REASON_WLAN";
                } else if (param == DEEPSLEEP_WAKEUPREASON_TIMER) {
                   wakeupReason = "WAKEUP_REASON_TIMER";
                } else if (param == DEEPSLEEP_WAKEUPREASON_FRONT_PANEL) {
                   wakeupReason = "WAKEUP_REASON_FRONT_PANEL";
                } else if (param == DEEPSLEEP_WAKEUPREASON_WATCHDOG) {
                   wakeupReason = "WAKEUP_REASON_WATCHDOG";
                } else if (param == DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET) {
                   wakeupReason = "WAKEUP_REASON_SOFTWARE_RESET";
                } else if (param == DEEPSLEEP_WAKEUPREASON_THERMAL_RESET) {
                   wakeupReason = "WAKEUP_REASON_THERMAL_RESET";
                } else if (param == DEEPSLEEP_WAKEUPREASON_WARM_RESET) {
                   wakeupReason = "WAKEUP_REASON_WARM_RESET";
                } else if (param == DEEPSLEEP_WAKEUPREASON_COLDBOOT) {
                   wakeupReason = "WAKEUP_REASON_COLDBOOT";
                } else if (param == DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE) {
                   wakeupReason = "WAKEUP_REASON_STR_AUTH_FAILURE";
                } else if (param == DEEPSLEEP_WAKEUPREASON_CEC) {
                   wakeupReason = "WAKEUP_REASON_CEC";
                } else if (param == DEEPSLEEP_WAKEUPREASON_PRESENCE) {
                   wakeupReason = "WAKEUP_REASON_PRESENCE";
                } else if (param == DEEPSLEEP_WAKEUPREASON_VOICE) {
                   wakeupReason = "WAKEUP_REASON_VOICE";
                }
            }
	    else
	    {
		status = false;
	    }
	    LOGWARN("WakeupReason : %s\n", wakeupReason.c_str());
            response["wakeupReason"] = wakeupReason;

            returnResponse(status);
        }

         /***
          * @brief Returns the deepsleep wakeup keycode.
          * @param1[in]  : {"params":{"appName":"abc"}}
          * @param2[out] : {"result":{"wakeupKeycode":<int>","success":<bool>}}
          * @return      : Core::<StatusCode>
          */

         uint32_t SystemServices::getLastWakeupKeyCode(const JsonObject& parameters, JsonObject& response)
         {
              bool status = false;
              IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t param;
              uint32_t wakeupKeyCode = 0;

              IARM_Result_t res = IARM_Bus_Call((m_isPwrMgr2RFCEnabled)? IARM_BUS_PWRMGR_NAME : IARM_BUS_DEEPSLEEPMGR_NAME,
                         IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupKeyCode, (void *)&param,
                         sizeof(param));
              if (IARM_RESULT_SUCCESS == res)
              {
                  status = true;
                  wakeupKeyCode = param.keyCode;
              }
              else
              {
                  status = false;
              }

              LOGWARN("WakeupKeyCode : %d\n", wakeupKeyCode);
              response["wakeupKeyCode"] = wakeupKeyCode;

              returnResponse(status);
         }

#endif

        /***
         * @brief Returns an array of strings containing the supported standby modes.
         * Possible values are "LIGHT_SLEEP" and/or "DEEP_SLEEP".
         *
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"supportedStandbyModes":["<string1>", "<string2>"],"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getAvailableStandbyModes (const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
            JsonArray standbyModes;
            try {
                    device::List<device::SleepMode> sleepModes =
                    device::Host::getInstance().getAvailableSleepModes();
                for (unsigned int i = 0; i < sleepModes.size(); i++) {
                    standbyModes.Add(sleepModes.at(i).toString());
                }
                status = true;
            } catch (...) {
                LOGERR("Error getting AvailableStandbyModes\n");
            }
            response["supportedStandbyModes"] = standbyModes;
            returnResponse(status);
        }

        /***
         * @brief This will return configuration parameters such as firmware version, Mac, Model etc.
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"xconfParams":{"eStbMac":"<string>",
         *			"firmwareVersion":"<string>", "env":"<string>",
         *			"model":"<string>"},"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getXconfParams(const JsonObject& parameters,
                JsonObject& response)
        {
            JsonObject rConf;
            std::string stbVersion = getStbVersionString();
            string firm = stbVersion;
            LOGINFO("stbVersion = %s firm = %s\n",
                    stbVersion.c_str(), firm.c_str());
            if (convert("VBN", firm)) {
                rConf["env"] = "vbn";
            } else if(convert("PROD", firm)) {
                rConf ["env"] = "prod";
            } else if(convert("QA", firm)) {
                rConf["env"] = "qa";
            } else {
                rConf["env"] = "dev";
            }

            std::string estbMac = collectDeviceInfo("estb_mac");
            removeCharsFromString(estbMac, "\n\r");
            rConf["eStbMac"] = estbMac;
            rConf["model"] = getModel();
            rConf["firmwareVersion"] = stbVersion;
            response["xconfParams"] = rConf;
            returnResponse(true);
        }

        /***
         * @brief : Populates device serial number from TR069 Support/Query.
         */
	bool SystemServices::getSerialNumberTR069(JsonObject& response)
        {
            bool ret =  false;
            std::string paramValue;
            RFC_ParamData_t param = {0};
            param.type = WDMP_NONE;
            WDMP_STATUS status = getRFCParameter(NULL, "Device.DeviceInfo.SerialNumber", &param);
            if(WDMP_SUCCESS == status)
            {
                paramValue = param.value;
                response["serialNumber"] = paramValue;
                ret = true;
            }
            else
                populateResponseWithError(SysSrv_Unexpected, response);

            return ret;
        }

        /***
         * @brief : Populates Device Serial Number Info using SNMP Request.
         */
        bool SystemServices::getSerialNumberSnmp(JsonObject& response)
        {
            bool retAPIStatus = false;
	    if (!Utils::fileExists("/lib/rdk/getStateDetails.sh")) {
		LOGERR("/lib/rdk/getStateDetails.sh not found.");
		populateResponseWithError(SysSrv_FileNotPresent, response);
	    } else {
		/* TODO: remove system() once alternate available. */
		system("/lib/rdk/getStateDetails.sh STB_SER_NO");
		std::vector<string> lines;
		if (true == Utils::fileExists(TMP_SERIAL_NUMBER_FILE)) {
		    if (getFileContent(TMP_SERIAL_NUMBER_FILE, lines)) {
			string serialNumber = lines.front();
			response["serialNumber"] = serialNumber;
			retAPIStatus = true;
		    } else {
			LOGERR("Unexpected contents in %s file.", TMP_SERIAL_NUMBER_FILE);
			populateResponseWithError(SysSrv_FileContentUnsupported, response);
		    }
		} else {
		    LOGERR("%s file not found.", TMP_SERIAL_NUMBER_FILE);
		    populateResponseWithError(SysSrv_FileNotPresent, response);
		}
	    }
	    return retAPIStatus;
	}

        /***
         * @brief : To retrieve Device Serial Number
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"serialNumber":"<string>","success":true}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getSerialNumber(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
#ifdef USE_TR_69
            retAPIStatus = getSerialNumberTR069(response);
#else
            retAPIStatus = getSerialNumberSnmp(response);
#endif
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : To fetch Firmware Download Percentage Info.
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"downloadPercent":<long int>, "success":false}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getFirmwareDownloadPercent(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retStatus = false;
            int m_downloadPercent = -1;
            if (Utils::fileExists("/opt/curl_progress")) {
                /* TODO: replace with new implementation. */
                FILE* fp = popen(CAT_DWNLDPROGRESSFILE_AND_GET_INFO, "r");
                if (NULL != fp) {
                    char output[8];
                    if (NULL != fgets (output, 8, fp)) {
                        output[strcspn(output, "\n")] = 0;
                        if (*output) {
                            m_downloadPercent = strtol(output, NULL, 10);
                        }
                        LOGWARN("FirmwareDownloadPercent = [%d]\n", m_downloadPercent);
                    } else {
                        LOGERR("Cannot read output from command\n");
                    }
                    pclose(fp);
                } else {
                    LOGERR("Cannot run command\n");
                }

                LOGWARN("FirmwareDownloadPercent = [%d]", m_downloadPercent);
                response["downloadPercent"] = m_downloadPercent;
                retStatus = true;
            } else {
                response["downloadPercent"] = -1;
                retStatus = true;
            }
            returnResponse(retStatus);
        }

        /***
         * @brief : gets firmware downloaded info.
         * @param1[in] : {"params":{}}
         * @param2[out] : "result":{"currentFWVersion":"<string>",
         *			"downloadedFWVersion":"<string>","downloadedFWLocation":"<string>",
         *			"isRebootDeferred":<bool>, "success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getDownloadedFirmwareInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retStat = false;
            string downloadedFWVersion = "";
            string downloadedFWLocation = "";
            bool isRebootDeferred = false;
            std::vector<string> lines;

	    if (!Utils::fileExists(FWDNLDSTATUS_FILE_NAME)) {
                //If firmware download file doesn't exist we can still return the current version
                response["downloadedFWVersion"] = downloadedFWVersion;
                response["downloadedFWLocation"] = downloadedFWLocation;
                response["isRebootDeferred"] = isRebootDeferred;
                retStat = true;
                string ver =  getStbVersionString();
                if(ver == "unknown")
                {
                    response["currentFWVersion"] = "";
                    retStat = false;
                }
                else
                {
                    response["currentFWVersion"] = ver;
                    retStat = true;
                }
	    }
            else if (getFileContent(FWDNLDSTATUS_FILE_NAME, lines)) {
                for (std::vector<std::string>::const_iterator i = lines.begin();
                        i != lines.end(); ++i) {
                    std::string line = *i;
                    std::string delimiter = "|";
                    size_t pos = 0;
                    std::string token;

                    std::size_t found = line.find("Reboot|");
                    if (std::string::npos != found) {
                        while ((pos = line.find(delimiter)) != std::string::npos) {
                            token = line.substr(0, pos);
                            line.erase(0, pos + delimiter.length());
                        }
                        line = std::regex_replace(line, std::regex("^ +| +$"), "$1");
                        if (line.length() > 1) {
                            if (!((strncasecmp(line.c_str(), "1", strlen("1")))
                                        && (strncasecmp(line.c_str(), "yes", strlen("yes")))
                                        && (strncasecmp(line.c_str(), "true", strlen("true"))))) {
                                isRebootDeferred = true;
                            }
                        }
                    }
                    // return DnldVersn based on IARM Firmware Update State
                    // If Firmware Update State is Downloading or above then 
                    // return DnldVersion from FWDNLDSTATUS_FILE_NAME else return empty
                    if(m_FwUpdateState_LatestEvent >=2)
                    {
                        found = line.find("DnldVersn|");
                        if (std::string::npos != found) {
                            while ((pos = line.find(delimiter)) != std::string::npos) {
                                token = line.substr(0, pos);
                                line.erase(0, pos + delimiter.length());
                            }
                            line = std::regex_replace(line, std::regex("^ +| +$"), "$1");
                            if (line.length() > 1) {
                                downloadedFWVersion = line.c_str();
                            }
                        }
                        found = line.find("DnldURL|");
                        if (std::string::npos != found) {
                            while ((pos = line.find(delimiter)) != std::string::npos) {
                                token = line.substr(0, pos);
                                line.erase(0, pos + delimiter.length());
                            }
                            line = std::regex_replace(line, std::regex("^ +| +$"), "$1");
                            if (line.length() > 1) {
                                downloadedFWLocation = line.c_str();
                            }
                        }
                    }
                }
                response["currentFWVersion"] = getStbVersionString();
                response["downloadedFWVersion"] = downloadedFWVersion;
                response["downloadedFWLocation"] = downloadedFWLocation;
                response["isRebootDeferred"] = isRebootDeferred;
                retStat = true;
            } else {
                populateResponseWithError(SysSrv_FileContentUnsupported, response);
            }
            returnResponse(retStat);
        }

        /***
         * @brief : gets firmware update state.
         * @param2[out] : {"result":{"firmwareUpdateState":<string fw update state>,"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getFirmwareUpdateState(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retStatus = false;
            FirmwareUpdateState fwUpdateState =(FirmwareUpdateState)m_FwUpdateState_LatestEvent;
            response["firmwareUpdateState"] = (int)fwUpdateState;
            retStatus = true;
            returnResponse(retStatus);
        }

        /***
         * @brief : sends notification when firmware update state has changed.
         *
         * @param1[in]  : newstate
         * @param2[out] : {"jsonrpc": "2.0","method":
         *			"org.rdk.SystemServices.events.1.onFirmwareUpdateStateChange",
         *			"param":{"firmwareUpdateState":<enum:0-6>}}
         */
        void SystemServices::onFirmwareUpdateStateChange(int newState)
        {
            JsonObject params;

            const FirmwareUpdateState firmwareUpdateState = (FirmwareUpdateState)newState;
            m_FwUpdateState_LatestEvent=(int)firmwareUpdateState;
            params["firmwareUpdateStateChange"] = (int)firmwareUpdateState;
            LOGINFO("New firmwareUpdateState = %d\n", (int)firmwareUpdateState);
            sendNotify(EVT_ONFIRMWAREUPDATESTATECHANGED, params);
        }

        /***
         * @brief : sends notification when time source state has changed.
         *
         */
        void SystemServices::onClockSet()
        {
            JsonObject params;
            sendNotify(EVT_ON_SYSTEM_CLOCK_SET, params);
        }

        /***
         * @brief : sends notification when time source state has changed.
         *
         */
        void SystemServices::onLogUpload(int newState)
        {
            lock_guard<mutex> lck(m_uploadLogsMutex);

            if (-1 != m_uploadLogsPid) {
                JsonObject params;

                params["logUploadStatus"] = newState == IARM_BUS_SYSMGR_LOG_UPLOAD_SUCCESS ? LOG_UPLOAD_STATUS_SUCCESS :
                    newState == IARM_BUS_SYSMGR_LOG_UPLOAD_ABORTED ? LOG_UPLOAD_STATUS_ABORTED : LOG_UPLOAD_STATUS_FAILURE;

                sendNotify(EVT_ONLOGUPLOAD, params);
                GetHandler(2)->Notify(EVT_ONLOGUPLOAD, params);

                pid_t wp;
                int status;

                if ((wp = waitpid(m_uploadLogsPid, &status, 0)) != m_uploadLogsPid) {
                    LOGERR("Waitpid for failed: %d, status: %d", m_uploadLogsPid, status);
                }

                m_uploadLogsPid = -1;
            } else {
                LOGERR("Upload Logs script isn't runing");
            }
        }

        /***
         * @brief : Worker to fetch details of various MAC addresses.
         * @Event : {"ecm_mac":"<MAC>","estb_mac":"<MAC>","moca_mac":"<MAC>",
         *     "eth_mac":"<MAC>","wifi_mac":"<MAC>","info":"Details fetch status",
         *     "success":<bool>}
         */
        void SystemServices::getMacAddressesAsync(SystemServices *pSs)
        {
	    long unsigned int i=0;
            long unsigned int listLength = 0;
            JsonObject params;
            string macTypeList[] = {"ecm_mac", "estb_mac", "moca_mac",
                "eth_mac", "wifi_mac", "bluetooth_mac", "rf4ce_mac"};
            string tempBuffer, cmdBuffer;

            for (i = 0; i < sizeof(macTypeList)/sizeof(macTypeList[0]); i++) {
                cmdBuffer.clear();
                cmdBuffer = "/lib/rdk/getDeviceDetails.sh read " + macTypeList[i];
                LOGWARN("cmd = %s\n", cmdBuffer.c_str());
                tempBuffer.clear();
                tempBuffer = Utils::cRunScript(cmdBuffer.c_str());
                removeCharsFromString(tempBuffer, "\n\r");
                LOGWARN("resp = %s\n", tempBuffer.c_str());
                params[macTypeList[i].c_str()] = (tempBuffer.empty()? "00:00:00:00:00:00" : tempBuffer.c_str());
                listLength++;
            }
            if (listLength != i) {
                params["info"] = "Details fetch: all are not success";
            }
            if (listLength) {
                params["success"] = true;
            } else {
                params["success"] = false;
            }
            if (pSs) {
                pSs->Notify(EVT_ONMACADDRESSRETRIEVED, params);
            } else {
                LOGERR("SystemServices *pSs is NULL\n");
            }
        }

        /***
         * @brief : get device mac addresses
         * @param2[out]: {"result":{"asyncResponse":<bool>,"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getMacAddresses(const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
            string guid = parameters["GUID"].String();

            LOGINFO("guid = %s\n", guid.c_str());
            if (!Utils::fileExists("/lib/rdk/getDeviceDetails.sh")) {
                response["SysSrv_Message"] = "File: getDeviceDetails.sh";
                populateResponseWithError(SysSrv_FileNotPresent, response);
            } else {
                try
                {
                    if (thread_getMacAddresses.get().joinable())
                        thread_getMacAddresses.get().join();

                    thread_getMacAddresses = Utils::ThreadRAII(std::thread(getMacAddressesAsync, this));
                    response["asyncResponse"] = true;
                    status = true;
                }
                catch(const std::system_error& e)
                {
                    LOGERR("exception in getFirmwareUpdateInfo %s", e.what());
                    response["asyncResponse"] = false;
                    status = false;
                }
            }
            returnResponse(status);
        }

        /***
         * @brief : called when Temperature Threshold is changed
         * @param1[in]  : string threshold type
         * @param1[in]  : bool exceed
         * @param1[in]  : float temperature
         * @param2[out] : {param:{"thresholdType":"<string>","exceeded":<bool>,"temperature":<string>}}
         */
        void SystemServices::onTemperatureThresholdChanged(string thresholdType,
                bool exceed, float temperature)
        {
            JsonObject params;
            params["thresholdType"] = thresholdType;
            params["exceeded"] = exceed;
            params["temperature"] = to_string(temperature);
            LOGWARN("thresholdType = %s exceed = %d temperature = %f\n",
                    thresholdType.c_str(), exceed, temperature);
            sendNotify(EVT_ONTEMPERATURETHRESHOLDCHANGED, params);
        }

        /***
         * @brief : To set the Time to TZ_FILE.
         * @param1[in]	: {"params":{"timeZone":"<string>"}}
         * @param2[out]	: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::setTimeZoneDST(const JsonObject& parameters,
                JsonObject& response)
	{
		bool resp = true;
		if (parameters.HasLabel("timeZone")) {
			std::string dir = dirnameOf(TZ_FILE);
			std::string timeZone = "";
			try {
				timeZone = parameters["timeZone"].String();
				size_t pos = timeZone.find("/");
				if (timeZone.empty() || (timeZone == "null")) {
					LOGERR("Empty timeZone received.");
				}
				else if( (pos == string::npos) ||  ( (pos != string::npos) &&  (pos+1 == timeZone.length())  )   )
				{
					LOGERR("Invalid timezone format received : %s . Timezone should be in Olson format  Ex : America/New_York .  \n", timeZone.c_str());
				}
				else {
					std::string path =ZONEINFO_DIR;
					path += "/";
					std::string country = timeZone.substr(0,pos);
					std::string city = path+timeZone;
					if( dirExists(path+country)  && Utils::fileExists(city.c_str()) ) 
					{
						if (!dirExists(dir)) {
							std::string command = "mkdir -p " + dir + " \0";
							Utils::cRunScript(command.c_str());
						} else {
							//Do nothing//
						}
						std::string oldTimeZoneDST = getTimeZoneDSTHelper();
						
						if (oldTimeZoneDST != timeZone) {
							FILE *f = fopen(TZ_FILE, "w");
							if (f) {
								if (timeZone.size() != fwrite(timeZone.c_str(), 1, timeZone.size(), f))
								{
									LOGERR("Failed to write %s", TZ_FILE);
									resp = false;
								}

								fflush(f);
								fsync(fileno(f));
								fclose(f);
#ifdef ENABLE_LINK_LOCALTIME
								// Now create the linux link back to the zone info file to our writeable localtime
                                				if (Utils::fileExists(LOCALTIME_FILE)) {
                                					remove (LOCALTIME_FILE);
                                				}

								LOGWARN("Linux localtime linked to %s\n", city.c_str());
								symlink(city.c_str(), LOCALTIME_FILE);
#endif
							} else {
								LOGERR("Unable to open %s file.\n", TZ_FILE);
								populateResponseWithError(SysSrv_FileAccessFailed, response);
								resp = false;
							}
						}

						std::string oldAccuracy = getTimeZoneAccuracyDSTHelper();
						std::string accuracy = oldAccuracy;

						if (parameters.HasLabel("accuracy")) {
							accuracy = parameters["accuracy"].String();
							if (accuracy != TZ_ACCURACY_INITIAL && accuracy != TZ_ACCURACY_INTERIM && accuracy != TZ_ACCURACY_FINAL) {
								LOGERR("Wrong TimeZone Accuracy: %s", accuracy.c_str());
								accuracy = oldAccuracy;
							}
						}

						if (accuracy != oldAccuracy) {
							FILE *f = fopen(TZ_ACCURACY_FILE, "w");
							if (f) {
								if (accuracy.size() != fwrite(accuracy.c_str(), 1, accuracy.size(), f))
								{
									LOGERR("Failed to write %s", TZ_ACCURACY_FILE);
									resp = false;
								}

								fflush(f);
								fsync(fileno(f));
								fclose(f);
							}
						}

						if (SystemServices::_instance && (oldTimeZoneDST != timeZone || oldAccuracy != accuracy))
							SystemServices::_instance->onTimeZoneDSTChanged(oldTimeZoneDST,timeZone,oldAccuracy, accuracy);

					}
					else{
						LOGERR("Invalid timeZone  %s received. Timezone not supported in TZ Database. \n", timeZone.c_str());
						populateResponseWithError(SysSrv_FileNotPresent, response);
						resp = false;
					}

#ifdef DISABLE_GEOGRAPHY_TIMEZONE
                    std::string tzenv = ":";
                    tzenv += timeZone;
                    Core::SystemInfo::SetEnvironment(_T("TZ"), tzenv.c_str());
#endif
				}
			} catch (...) {
				LOGERR("catch block : parameters[\"timeZone\"]...");
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(resp);
	}

        uint32_t SystemServices::getFriendlyName(const JsonObject& parameters, JsonObject& response)
        {
            bool resp = true;
            response["friendlyName"] = m_friendlyName;
            returnResponse(resp);
        }

        uint32_t SystemServices::setFriendlyName(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            returnIfParamNotFound(parameters, "friendlyName");
            string friendlyName = parameters["friendlyName"].String();
            bool success = true;
            LOGWARN("SystemServices::setFriendlyName  :%s \n", friendlyName.c_str());
            if(m_friendlyName != friendlyName)
            {
                m_friendlyName = friendlyName;
                JsonObject params;
                params["friendlyName"] = m_friendlyName;
                sendNotify("onFriendlyNameChanged", params);
                //write to persistence storage
                WDMP_STATUS status = setRFCParameter((char*)"thunderapi",
                       TR181_SYSTEM_FRIENDLY_NAME,m_friendlyName.c_str(),WDMP_STRING);
                if ( WDMP_SUCCESS == status ){
                    LOGINFO("Success Setting the friendly name value\n");
                }
                else {
                    LOGINFO("Failed Setting the friendly name value %s\n",getRFCErrorString(status));
                }
            }
            returnResponse(success);
        }

	uint32_t SystemServices::setTerritory(const JsonObject& parameters, JsonObject& response)
	{
		bool resp = false;
		if(parameters.HasLabel("territory")){
			struct stat st = {0};
			if (stat("/opt/secure/persistent/System", &st) == -1) {
				int ret = mkdir("/opt/secure/persistent/System", 0700);
				LOGWARN(" --- SubDirectories created from mkdir %d ", ret);
			}
			string regionStr = "";
			readTerritoryFromFile();//Read existing territory and Region from file
			string territoryStr = parameters["territory"].String();
			LOGWARN(" Territory Value : %s ", territoryStr.c_str());
			try{
				int index = m_strStandardTerritoryList.find(territoryStr);
				if((territoryStr.length() == 3) && (index >=0 && index <= 1100) ){
					if(parameters.HasLabel("region")){
						regionStr = parameters["region"].String();
						if(regionStr != ""){
							if(isRegionValid(regionStr)){
								resp = writeTerritory(territoryStr,regionStr);
								LOGWARN(" territory name %s ", territoryStr.c_str());
								LOGWARN(" region name %s", regionStr.c_str());
							}else{
								JsonObject error;
								error["message"] = "Invalid region";
								response["error"] = error;
								LOGWARN("Please enter valid region");
								returnResponse(resp);
							}
						}
					}else{
						resp = writeTerritory(territoryStr,regionStr);
						LOGWARN(" Region is empty, only territory is updated. territory name %s ", territoryStr.c_str());
					}
				}else{
					JsonObject error;
					error["message"] =  "Invalid territory";
					response["error"] = error;
					LOGWARN("Please enter valid territory Parameter value.");
					returnResponse(resp);
				}
				if(resp == true){
					//call event on Territory changed
					if (SystemServices::_instance)
						SystemServices::_instance->onTerritoryChanged(m_strTerritory,territoryStr,m_strRegion,regionStr);
				}
			}
			catch(...){
				LOGWARN(" caught exception...");
			}
		}else{
			JsonObject error;
			error["message"] =  "Invalid territory name";
			response["error"] = error;
			LOGWARN("Please enter valid territory Parameter name.");
			resp = false;
		}
		returnResponse(resp);
	}

	uint32_t SystemServices::writeTerritory(string territory, string region)
	{
		bool resp = false;
		ofstream outdata(TERRITORYFILE);
		if(!outdata){
			LOGWARN(" Territory : Failed to open the file");
			return resp;
		}
		if (territory != ""){
			outdata << "territory:" + territory+"\n";
			resp = true;
		}
		if (region != ""){
			outdata << "region:" + region+"\n";
			resp = true;
		}
		outdata.close();
		return resp;
	}

	uint32_t SystemServices::getTerritory(const JsonObject& parameters, JsonObject& response)
	{
		bool resp = true;
		m_strTerritory = "";
		m_strRegion = "";
		resp = readTerritoryFromFile();
		response["territory"] = m_strTerritory;
		response["region"] = m_strRegion;
		returnResponse(resp);
	}

	bool SystemServices::readTerritoryFromFile()
	{
		bool retValue = true;
		if(Utils::fileExists(TERRITORYFILE)){
			ifstream inFile(TERRITORYFILE);
			string str;
			getline (inFile, str);
			if(str.length() > 0){
				retValue = true;
				m_strTerritory = str.substr(str.find(":")+1,str.length());
				int index = m_strStandardTerritoryList.find(m_strTerritory);
				if((m_strTerritory.length() == 3) && (index >=0 && index <= 1100) ){

					getline (inFile, str);
					if(str.length() > 0){
						m_strRegion = str.substr(str.find(":")+1,str.length());
						if(!isRegionValid(m_strRegion)){
							m_strTerritory = "";
							m_strRegion = "";
							LOGERR("Territory file corrupted  - region : %s",m_strRegion.c_str());
							LOGERR("Returning empty values");
						}
					}
				}
				else{
					m_strTerritory = "";
					m_strRegion = "";
					LOGERR("Territory file corrupted - territory : %s",m_strTerritory.c_str());
					LOGERR("Returning empty values");
				}
			}
			else{
				LOGERR("Invalid territory file");
			}
			inFile.close();

		}else{
			LOGERR("Territory is not set");
		}
		return retValue;
	}

	bool SystemServices::isStrAlphaUpper(string strVal)
	{
		try{
			long unsigned int i=0;
			for(i=0; i<= strVal.length()-1; i++)
			{
				if((isalpha(strVal[i])== 0) || (isupper(strVal[i])==0))
				{
					LOGERR(" -- Invalid Territory ");
					return false;
					break;
				}
			}
		}
		catch(...){
			LOGERR(" Exception caught");
			return false;
		}
		return true;
	}


	bool SystemServices::isRegionValid(string regionStr)
	{
		bool retVal = false;
		if(regionStr.length() < 7){
			string strRegion = regionStr.substr(0,regionStr.find("-"));
			if( strRegion.length() == 2){
				if (isStrAlphaUpper(strRegion)){
					strRegion = regionStr.substr(regionStr.find("-")+1,regionStr.length());
					if(strRegion.length() >= 2){
						retVal = isStrAlphaUpper(strRegion);
					}
				}
			}
		}
		return retVal;
	}

	void SystemServices::onTerritoryChanged(string oldTerritory, string newTerritory, string oldRegion, string newRegion)
	{
		JsonObject params;
		params["oldTerritory"] = oldTerritory;
		params["newTerritory"] = newTerritory;
		LOGWARN(" Notifying Territory changed - oldTerritory: %s - newTerritory: %s",oldTerritory.c_str(),newTerritory.c_str());
		if(newRegion != ""){
			params["oldRegion"] = oldRegion;
			params["newRegion"] = newRegion;
			LOGWARN(" Notifying Region changed - oldRegion: %s - newRegion: %s",oldRegion.c_str(),newRegion.c_str());
		}
		//Notify territory changed
		sendNotify(EVT_ONTERRITORYCHANGED, params);
	}

	void SystemServices::onTimeZoneDSTChanged(string oldTimeZone, string newTimeZone, string oldAccuracy, string newAccuracy)
	{
		JsonObject params;
		params["oldTimeZone"] = oldTimeZone;
		params["newTimeZone"] = newTimeZone;
		params["oldAccuracy"] = oldAccuracy;
		params["newAccuracy"] = newAccuracy;
		LOGWARN(" Notifying TimeZone changed - oldTimeZone: %s - newTimeZone: %s, oldAccuracy: %s - newAccuracy: %s",oldTimeZone.c_str(),newTimeZone.c_str(),oldAccuracy.c_str(),newAccuracy.c_str());
		//Notify TimeZone changed
		sendNotify(EVT_ONTIMEZONEDSTCHANGED, params);
	}

    uint32_t SystemServices::uploadLogsAsync(const JsonObject& parameters, JsonObject& response)
    {
        LOGWARN("");

        pid_t uploadLogsPid = -1;

        {
            lock_guard<mutex> lck(m_uploadLogsMutex);
            uploadLogsPid = m_uploadLogsPid;
        }

        if (-1 != uploadLogsPid) {
            LOGWARN("Another instance of log upload script is running");
            abortLogUpload(parameters, response);
        }

        lock_guard<mutex> lck(m_uploadLogsMutex);
        m_uploadLogsPid = UploadLogs::logUploadAsync();

        returnResponse(true);
    }

    uint32_t SystemServices::abortLogUpload(const JsonObject& parameters, JsonObject& response)
    {

        lock_guard<mutex> lck(m_uploadLogsMutex);

        if (-1 != m_uploadLogsPid) {

            // Kill child processes
            std::stringstream cmd;
            cmd << "pgrep -P " << m_uploadLogsPid;

            FILE* fp = popen(cmd.str().c_str(), "r");
            if (NULL != fp) {

                char output[1024];
                while (NULL != fgets (output, sizeof(output) - 1, fp)) {
                    std::string line = output;
                    line = trim(line);

                    char *end;
                    int pid = strtol(line.c_str(), &end, 10);

                    if (line.c_str() != end && 0 != pid && 1 != pid) {
                        kill(pid, SIGKILL);
                    } else
                        LOGERR("Bad pid: %d", pid);
                }

                pclose(fp);
            } else {
                LOGERR("Cannot run command\n");
            }

            kill(m_uploadLogsPid, SIGKILL);

            int status;
            waitpid(m_uploadLogsPid, &status, 0);

            m_uploadLogsPid = -1;

            JsonObject params;
            params["logUploadStatus"] = LOG_UPLOAD_STATUS_ABORTED;
            sendNotify(EVT_ONLOGUPLOAD, params);

            returnResponse(true);
        }

        LOGERR("Upload logs script is not running");
        returnResponse(false);
    }

        /***
         * @brief : To fetch timezone from TZ_FILE.
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {","id":3,"result":{"timeZone":"<String>","success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getTimeZoneDST(const JsonObject& parameters,
                JsonObject& response)
        {
            std::string timezone;
            bool resp = false;

            if (Utils::fileExists(TZ_FILE)) {
                if(readFromFile(TZ_FILE, timezone)) {
                    LOGWARN("Fetch TimeZone: %s\n", timezone.c_str());
                    response["timeZone"] = timezone;
                    resp = true;
                } else {
                    LOGERR("Unable to open %s file.\n", TZ_FILE);
                    response["timeZone"] = "null";
                    resp = false;
                }
            } else {
                LOGERR("File not found %s, returning default.\n", TZ_FILE);
                response["timeZone"] = TZ_DEFAULT;
                resp = true;
            }

            if (resp) {
                response["accuracy"] = getTimeZoneAccuracyDSTHelper();
            }

            returnResponse(resp);
        }

        bool SystemServices::processTimeZones(std::string dir, JsonObject& out)
        {
            bool ret = true;
            std::string cmd = "zdump ";
            cmd += dir;
            cmd += "/*";

            FILE *p = popen(cmd.c_str(), "r");

            if(!p)
            {
                LOGERR("failed to start %s: %s", cmd.c_str(), strerror(errno));
                return false;

            }

            std::vector <std::string> dirs;

            char buf[4096];
            while(fgets(buf, sizeof(buf), p) != NULL)
            {
                std::string line(buf);

                line.erase(0, line.find_first_not_of(" \n\r\t"));
                line.erase(line.find_last_not_of(" \n\r\t") + 1);

                size_t fileEnd = line.find_first_of(" \t");

                std::string fullName;

                if (std::string::npos == fileEnd)
                {
                    LOGERR("Failed to parse '%s'", line.c_str());
                    continue;
                }

                fullName = line.substr(0, fileEnd);

                struct stat deStat;
                if (stat(fullName.c_str(), &deStat))
                {
                    LOGERR("stat() failed: %s", strerror(errno));
                    continue;
                }

                if (S_ISDIR(deStat.st_mode))
                {
                    dirs.push_back(fullName);
                }
                else
                {

                    std::string name = fullName;

                    size_t pathEnd = fullName.find_last_of("/") + 1;
                    if (std::string::npos != pathEnd)
                        name = fullName.substr(pathEnd);
                    else
                        LOGWARN("No '/' in %s", fullName.c_str());

                    line.erase(0, line.find_first_of(" \t"));
                    line.erase(0, line.find_first_not_of(" \n\r\t"));

                    out[name.c_str()] = line;
                }
            }

            int err = pclose(p);

            if (0 != err)
            {    
                LOGERR("%s failed with code %d", cmd.c_str(), err);
                return false;
            }

	    long unsigned int n=0;
            for (n = 0 ; n < dirs.size(); n++) {
                std::string name = dirs[n];

                size_t pathEnd = name.find_last_of("/") + 1;

                if (std::string::npos != pathEnd)
                    name = name.substr(pathEnd);
                else
                    LOGWARN("No '/' in %s", name.c_str());

                JsonObject dirObject;
                processTimeZones(dirs[n], dirObject);
                out[name.c_str()] = dirObject;
            }

            return ret;
        }

        uint32_t SystemServices::getTimeZones(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO("called");

            JsonObject dirObject;
            bool resp = processTimeZones(ZONEINFO_DIR, dirObject);
            response["zoneinfo"] = dirObject;

            returnResponse(resp);
        }

        /***
         * @brief : To fetch core temperature
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"temperature":<float>,"success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getCoreTemperature(const JsonObject& parameters,
                JsonObject& response)
        {
            bool resp = false;
            float temperature;
#ifdef ENABLE_THERMAL_PROTECTION
            resp = CThermalMonitor::instance()->getCoreTemperature(temperature);
            LOGWARN("core temperature is %.1f degrees centigrade\n",
                    temperature);
#else
            temperature = -1;
            resp = false;
            LOGERR("Thermal Protection disabled for this platform\n");
	    populateResponseWithError(SysSrv_SupportNotAvailable, response);
#endif
            response["temperature"] = to_string(temperature);
            returnResponse(resp);
        }

        /***
         * @brief : To get cashed value .
         * @param1[in]  : {"params":{"key":"<string>"}}
         * @param2[out] : {"result":{"<cachekey>":"<string>","success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getCachedValue(const JsonObject& parameters,
			JsonObject& response)
	{
		bool retStat = false;
		bool deprecated = true;
		if (parameters.HasLabel("key")) {
			std::string key = parameters["key"].String();
			LOGWARN("key: '%s'\n", key.c_str());
			if (key.length()) {
				response[(key.c_str())] = (m_cacheService.getValue(key).String().empty()?
						"" : m_cacheService.getValue(key).String());
				retStat = true;
			} else {
				populateResponseWithError(SysSrv_UnSupportedFormat, response);
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		response["deprecated"] = deprecated;
		returnResponse(retStat);
	}

        /***
         * @brief : To set cache value.
         * @param1[in]  : {"params":{"key":"<string>","value":<double>}}
         * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::setCachedValue(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retStat = false;
	    bool deprecated = true;

	    if (parameters.HasLabel("key") && parameters.HasLabel("value")) {
		    std::string key = parameters["key"].String();
		    std::string value = parameters["value"].String();
		    LOGWARN("key: '%s' value: '%s'\n", key.c_str(), value.c_str());
		    if (key.length() && value.length()) {
			    if (m_cacheService.setValue(key, value)) {
				    retStat = true;
			    } else {
				    LOGERR("Accessing m_cacheService.setValue failed\n.");
				    populateResponseWithError(SysSrv_Unexpected, response);
			    }
		    } else {
			    populateResponseWithError(SysSrv_UnSupportedFormat, response);
		    }
	    } else {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
	    }
	    response["deprecated"] = deprecated;
	    returnResponse(retStat);
        }

        /***
         * @brief : To check if key value present in cache.
         * @param1[in]  : {"params":{"key":"<string>"}}
         * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::cacheContains(const JsonObject& parameters,
                JsonObject& response)
        {
		bool retStat = false;
		bool deprecated = true;
		if (parameters.HasLabel("key")) {
			std::string key = parameters["key"].String();
			if (key.length()) {
				if (m_cacheService.contains(key)) {
					retStat = true;
				} else {
					LOGERR("Accessing m_cacheService.contains; no matching key '%s'\n.", key.c_str());
					populateResponseWithError(SysSrv_KeyNotFound, response);
				}
			} else {
				populateResponseWithError(SysSrv_UnSupportedFormat, response);
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		response["deprecated"] = deprecated;
		returnResponse(retStat);
        }

        /***
         * @brief : To delete the key value present in cache.
         * @param1[in]  : {"params":{"key":"<string>"}}
         * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::removeCacheKey(const JsonObject& parameters,
                JsonObject& response)
        {
		bool retStat = false;
		bool deprecated = true;
		if (parameters.HasLabel("key")) {
			std::string key = parameters["key"].String();
			if (key.length()) {
				if (m_cacheService.remove(key)) {
					retStat = true;
				} else {
					LOGERR("Accessing m_cacheService.remove failed\n.");
					populateResponseWithError(SysSrv_Unexpected, response);
				}
			} else {
				populateResponseWithError(SysSrv_UnSupportedFormat, response);
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		response["deprecated"] = deprecated;
		returnResponse(retStat);
        }

        /***
         * @brief : To get previous boot info.
         * @param1[in]	: {"params":{}}
         * @param2[out]	: {"result":{"timeStamp":"<string>","reason":"<string>",
         *				   "success":<bool>}}
         * @return		: Core::<StatusCode>
         */
        uint32_t SystemServices::getPreviousRebootInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            if (!Utils::fileExists(REBOOT_INFO_LOG_FILE)) {
                LOGERR("Cant't determine previous reboot info, %s not found or can't be opened for reading", REBOOT_INFO_LOG_FILE);
                returnResponse(false);
            }

            string rebootInfo;
            if (!getFileContent(REBOOT_INFO_LOG_FILE, rebootInfo)) {
                LOGERR("Cant't determine previous reboot info, %s not found or can't be opened for reading", REBOOT_INFO_LOG_FILE);
                returnResponse(false);
            }

            if (rebootInfo.length() < 1) {
                LOGERR("No reboot info, file %s is empty", REBOOT_INFO_LOG_FILE);
                returnResponse(false);
            }

            smatch match;

            string timeStamp = "Unknown";
            string reason = "Unknown";
            string source = "Unknown";
            string customReason = "Unknown";
            string otherReason = "Unknown";

            string temp;
            if (regex_search(rebootInfo, match, regex("(?:PreviousRebootTime:)([^\\n]+)")) &&  match.size() > 1) temp = trim(match[1]);
            if (temp.size() > 0) timeStamp = temp;

            if (regex_search(rebootInfo, match, regex("(?:PreviousRebootReason: RebootReason:)([^\\n]+)")) &&  match.size() > 1) temp = trim(match[1]);
            if (temp.size() > 0) reason = temp;

            if (regex_search(rebootInfo, match, regex("(?:PreviousRebootInitiatedBy:)([^\\n]+)")) &&  match.size() > 1) temp = trim(match[1]);
            if (temp.size() > 0) source = temp;

            if (regex_search(rebootInfo, match, regex("(?:PreviousCustomReason:)([^\\n]+)")) &&  match.size() > 1) temp = trim(match[1]);
            if (temp.size() > 0) customReason = temp;

            if (regex_search(rebootInfo, match, regex("(?:PreviousOtherReason:)([^\\n]+)")) &&  match.size() > 1) temp = trim(match[1]);
            if (temp.size() > 0) otherReason = temp;

            response["timeStamp"] = timeStamp;
            response["reason"] = reason;
            response["source"] = source;
            response["customReason"] = customReason;
            response["otherReason"] = otherReason;

            returnResponse(true);
        }

        /***
         * @brief : Used to retrieve last deep sleep reason.
         * @param1[in]  : query parameter.
         * @param2[out] : "result":{"lastDeepSleepReason":"<string>","success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getLastDeepSleepReason(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
            string reason;

            if (Utils::fileExists(STANDBY_REASON_FILE)) {
                std::ifstream inFile(STANDBY_REASON_FILE);
                if (inFile) {
                    std::getline(inFile, reason);
                    inFile.close();
                    retAPIStatus = true;
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }

            if (retAPIStatus && reason.length()) {
                response["reason"] = reason;
            } else {
                response["reason"] = "";
            }
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : Used to clear last deep sleep reason.
         * @param1[in]  : query parameter.
         * @param2[out] : "result":{"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::clearLastDeepSleepReason(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;

                /* FIXME: popen in use */
                FILE *pipe = NULL;
                char cmd[128] = {'\0'};

                snprintf(cmd, 127, "rm -f %s", STANDBY_REASON_FILE);
                pipe = popen(cmd, "r");
                if (pipe) {
                    retAPIStatus = ((pclose(pipe) != -1)? true: false);
                    if (false == retAPIStatus) {
                        populateResponseWithError(SysSrv_Unexpected, response);
                    }
                } else {
                    populateResponseWithError(SysSrv_Unexpected, response);
            }

            returnResponse(retAPIStatus);
        }

#ifdef ENABLE_THERMAL_PROTECTION
        /***
         * @brief : To retrieve Temperature Threshold values.
         * @param1[in] : {"params":{}}
         * @param2[out] : "result":{"temperatureThresholds":{"WARN":"100.000000",
         *     "MAX":"200.000000","temperature":"62.000000"},"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getTemperatureThresholds(const JsonObject& parameters,
                JsonObject& response)
        {
            JsonObject value;
            float high = 0.0, critical = 0.0, temperature = 0.0;
            bool resp1 = CThermalMonitor::instance()->getCoreTempThresholds(high, critical);
            bool resp2 = CThermalMonitor::instance()->getCoreTemperature(temperature);
            LOGWARN("Got current temperature thresholds: WARN: %f, MAX: %f, ret[resp1 = %d resp = %d]\n",
                    high, critical, resp1, resp2);
            if (resp1) {
                value["WARN"] = to_string(high);
                value["MAX"] = to_string(critical);
            }
            if (resp2) {
                value["temperature"] = to_string(temperature);
            }
            if (resp1 & resp2) {
                response["temperatureThresholds"] = value;
            }
            returnResponse(((resp1 & resp2)? true: false));
        }

        /***
         * @brief : To set Temperature Threshold values.
         * @param1[in] : {"params":{"thresholds":{"WARN":"99.000000","MAX":"199.000000"}}}
         * @param2[out] : {"result":{"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::setTemperatureThresholds(const JsonObject& parameters,
                JsonObject& response)
        {
            JsonObject args;
            float high = 0.0;
            float critical = 0.0;
	    bool resp = false;
	    if (parameters.HasLabel("thresholds")) {
		    args.FromString(parameters["thresholds"].String());
		    string warn = args["WARN"].String();
		    string max = args["MAX"].String();

		    high = atof(warn.c_str());
		    critical = atof(max.c_str());

		    resp =  CThermalMonitor::instance()->setCoreTempThresholds(high, critical);
		    LOGWARN("Set temperature thresholds: WARN: %f, MAX: %f\n", high, critical);
	    } else {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
	    }
            returnResponse(resp);
        }

	/***
         * @brief : To retrieve Overtemparature grace interval value.
         * @param1[in] : {"params":{}}
         * @param2[out] : "result":{"graceInterval":"600",},"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getOvertempGraceInterval(const JsonObject& parameters,
                JsonObject& response)
        {
            int graceInterval = 0;
            bool resp = CThermalMonitor::instance()->getOvertempGraceInterval(graceInterval);
            LOGWARN("Got current grace interval: %d ret[resp = %d]\n",
                    graceInterval, resp);
            if (resp) {
                response["graceInterval"] = to_string(graceInterval);
            }
            returnResponse(resp);
        }

	/***
         * @brief : To set Overtemparature grace interval value.
         * @param1[in] : {"params":{"graceInterval":"600"}}
         * @param2[out] : {"result":{"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::setOvertempGraceInterval(const JsonObject& parameters,
                JsonObject& response)
        {
            int graceInterval  = 0;
            bool resp = false;
            if (parameters.HasLabel("graceInterval")) {
                    string grace = parameters["graceInterval"].String();

                    graceInterval = atoi(grace.c_str());

                    resp =  CThermalMonitor::instance()->setOvertempGraceInterval(graceInterval);
                    LOGWARN("Set Grace Interval : %d\n", graceInterval);
            } else {
                    populateResponseWithError(SysSrv_MissingKeyValues, response);
            }
            returnResponse(resp);
        }
#endif /* ENABLE_THERMAL_PROTECTION */

        /***
         * @brief : Used to retrieve previous reboot details.
         * @param1[in]  : query parameter.
         * @param2[out] : "result":{"rebootInfo":{"timestamp":"string","source":"string",
         *  "reason":"string","customReason":"string", "lastHardPowerReset":"<string>"}
         *  ,"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getPreviousRebootInfo2(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
            uint8_t parseStatus = 0;
            JsonObject respData;
            string timestamp, source, reason, customReason, lastHardPowerReset;
            string rebootInfo;
            string hardPowerInfo;

            if (Utils::fileExists(SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE)) {
                retAPIStatus = getFileContent(
                        SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE, rebootInfo);
                if (retAPIStatus && rebootInfo.length()) {
                    JsonObject rebootInfoJson;
                    rebootInfoJson.FromString(rebootInfo);
                    timestamp = rebootInfoJson["timestamp"].String();
                    source = rebootInfoJson["source"].String();
                    reason = rebootInfoJson["reason"].String();
                    customReason = rebootInfoJson["customReason"].String();
                    parseStatus++;
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }

            if (Utils::fileExists(SYSTEM_SERVICE_HARD_POWER_INFO_FILE)) {
                retAPIStatus = getFileContent(
                        SYSTEM_SERVICE_HARD_POWER_INFO_FILE, hardPowerInfo);
                if (retAPIStatus && hardPowerInfo.length()) {
                    JsonObject hardPowerInfoJson;
                    hardPowerInfoJson.FromString(hardPowerInfo);
                    lastHardPowerReset = hardPowerInfoJson["lastHardPowerReset"].String();
                    parseStatus++;
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                retAPIStatus = false;
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }

            if (retAPIStatus && (2 == parseStatus)) {
                respData["timestamp"] = timestamp;
                respData["source"] = source;
                respData["reason"] = reason;
                respData["customReason"] = customReason;
                respData["lastHardPowerReset"] = lastHardPowerReset;

                response["rebootInfo"] = respData;
            }
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : Used to retrieve last reboot reason.
         * @param1[in]  : query parameter.
         * @param2[out] : "result":{"rebootReason":<string>","success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getPreviousRebootReason(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
            uint8_t parseStatus = 0;
            string reason;
            string rebootInfo;

            if (Utils::fileExists(SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE)) {
                retAPIStatus = getFileContent(
                        SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE, rebootInfo);
                if (retAPIStatus && rebootInfo.length()) {
                    JsonObject rebootInfoJson;
                    rebootInfoJson.FromString(rebootInfo);
                    reason = rebootInfoJson["reason"].String();
                    parseStatus++;
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
                retAPIStatus = false;
            }

            if (retAPIStatus && (1 == parseStatus)) {
                response["reason"] = reason;
            }
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : To get RFC Configs.
         * @param1[in]  : "params":{"rfclist":["<rfc>","<rfc>"]}.
         * @param2[out] : {"result":{"RFCConfig":{"<rfc>":"<value>","<rfc>":"<value>"},
         *          "success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getRFCConfig(const JsonObject& parameters,
                JsonObject& response)
        {
            const std::regex re("(\\w|-|\\.)+");
            bool retAPIStatus = false;
            JsonObject hash;
            JsonArray jsonRFCList;
	    if (!parameters.HasLabel("rfcList")) {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
		    returnResponse(retAPIStatus);
	    }
            jsonRFCList = parameters["rfcList"].Array();
            std::string cmdParams, cmdResponse;

            if (!jsonRFCList.Length()) {
                populateResponseWithError(SysSrv_UnSupportedFormat, response);
            } else {
                for (int i = 0; i < jsonRFCList.Length(); i++) {
                    LOGINFO("jsonRFCList[%d] = %s\n",
                            i, jsonRFCList[i].String().c_str());
                    if (!std::regex_match(jsonRFCList[i].String(), re)) {
                        LOGERR("ignore '%s': Contains unsupported charset.\
                                [Use only alpha-numeric, underscores, hyphens and period]",
                                jsonRFCList[i].String().c_str());
                        hash[jsonRFCList[i].String().c_str()] = "Invalid charset found";
                        continue;
                    } else {
                        cmdResponse = "";

                        WDMP_STATUS wdmpStatus;
                        RFC_ParamData_t rfcParam;
			char sysServices[] = "SystemServices";

                        memset(&rfcParam, 0, sizeof(rfcParam));
                        wdmpStatus = getRFCParameter(sysServices, jsonRFCList[i].String().c_str(), &rfcParam);
                        if(WDMP_SUCCESS == wdmpStatus || WDMP_ERR_DEFAULT_VALUE == wdmpStatus)
                            cmdResponse = rfcParam.value;
                        else
                            LOGERR("Failed to get %s with %d", jsonRFCList[i].String().c_str(), wdmpStatus);

                        if (!cmdResponse.empty()) {
                            removeCharsFromString(cmdResponse, "\n\r");
                            hash[jsonRFCList[i].String().c_str()] = cmdResponse;
                            retAPIStatus = true;
                        } else {
                            hash[jsonRFCList[i].String().c_str()] = "Empty response received";
                        }
                    }
                }
                response["RFCConfig"] = hash;
            }
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : To fetch the list of milestones.
         * @param1[in]  : {params":{}}
         * @param2[out] : "result":{"milestones":["<string>","<string>","<string>"],
         *      "success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getMilestones(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retAPIStatus = false;
	    bool deprecated = true;
	    std::vector<string> milestones;

            if (Utils::fileExists(MILESTONES_LOG_FILE)) {
                retAPIStatus = getFileContent(MILESTONES_LOG_FILE, milestones);
                if (retAPIStatus) {
                    setJSONResponseArray(response, "milestones", milestones);
                } else {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }
	    response["deprecated"] = deprecated;
            returnResponse(retAPIStatus);
        }

        /***
         * @brief : Enables XRE Connection Retension option.
         * @param1[in]  : {"params":{"enable":<bool>}}
         * @param2[out] : "result":{"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::enableXREConnectionRetention(const JsonObject& parameters,
                JsonObject& response)
	{
		bool enable = false, retstatus = false;
		int status = SysSrv_Unexpected;
		if (parameters.HasLabel("enable")) {
			enable = parameters["enable"].Boolean();
			if ((status = enableXREConnectionRetentionHelper(enable)) == SysSrv_OK) {
				retstatus = true;
			} else {
				populateResponseWithError(status, response);
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(retstatus);
	}

        /***
         * @brief : collect device state info.
         * @param1[in]  : {"params":{"param":"<queryState>"}}
         * @param2[out] : {"result":{"<queryState>":<value>,"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::getStateInfo(const JsonObject& parameters,
                JsonObject& response)
        {
            int32_t retVal = E_NOK;
            JsonObject param;
            JsonObject resParam;
            string methodType;
	    if (parameters.HasLabel("param")) {
                methodType = parameters["param"].String();
                if (SYSTEM_CHANNEL_MAP == methodType) {
                    LOGERR("methodType : %s\n", methodType.c_str());
                    IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates,
                            &paramGetSysState, sizeof(paramGetSysState));
                    response[SYSTEM_CHANNEL_MAP] = paramGetSysState.channel_map.state;
                    LOGWARN("SystemService querying channel_map, return\
                            channel_map state : %d\n", paramGetSysState.channel_map.state);
                    retVal = E_OK;
                } else if (SYSTEM_CARD_DISCONNECTED == methodType) {
                    response[SYSTEM_CARD_DISCONNECTED] = paramGetSysState.disconnect_mgr_state.state;
                    retVal = E_OK;
                } else if (SYSTEM_TUNE_READY == methodType) {
                    response[SYSTEM_TUNE_READY] = paramGetSysState.TuneReadyStatus.state;
                    retVal = E_OK;
                } else if (SYSTEM_CMAC == methodType) {
                    response[SYSTEM_CMAC] = paramGetSysState.cmac.state;
                    retVal = E_OK;
                } else if (SYSTEM_MOTO_ENTITLEMENT == methodType) {
                    response[SYSTEM_MOTO_ENTITLEMENT] = paramGetSysState.card_moto_entitlements.state;
                    retVal = E_OK;
                } else if (SYSTEM_MOTO_HRV_RX == methodType) {
                    response[SYSTEM_MOTO_HRV_RX] = paramGetSysState.card_moto_hrv_rx.state;
                    retVal = E_OK;
                } else if (SYSTEM_CARD_CISCO_STATUS == methodType) {
                    response[SYSTEM_CARD_CISCO_STATUS] = paramGetSysState.card_cisco_status.state;
                    retVal = E_OK;
                } else if (SYSTEM_VIDEO_PRESENTING == methodType) {
                    response[SYSTEM_VIDEO_PRESENTING] = paramGetSysState.video_presenting.state;
                    retVal = E_OK;
                } else if (SYSTEM_HDMI_OUT == methodType) {
                    response[SYSTEM_HDMI_OUT] = paramGetSysState.hdmi_out.state;
                    retVal = E_OK;
                } else if (SYSTEM_HDCP_ENABLED == methodType) {
                    response[SYSTEM_HDCP_ENABLED] = paramGetSysState.hdcp_enabled.state;
                    retVal = E_OK;
                } else if (SYSTEM_HDMI_EDID_READ == methodType) {
                    response[SYSTEM_HDMI_EDID_READ] = paramGetSysState.hdmi_edid_read.state;
                    retVal = E_OK;
                } else if (SYSTEM_FIRMWARE_DWNLD == methodType) {
                    response[SYSTEM_FIRMWARE_DWNLD] = paramGetSysState.firmware_download.state;
                    retVal = E_OK;
                } else if (SYSTEM_TIME_SOURCE == methodType) {
                    response[SYSTEM_TIME_SOURCE] = paramGetSysState.time_source.state;
                    retVal = E_OK;
                } else if (SYSTEM_TIME_ZONE == methodType) {
                    response[SYSTEM_TIME_ZONE] = paramGetSysState.time_zone_available.state;
                    retVal = E_OK;
                } else if (SYSTEM_CA_SYSTEM == methodType) {
                    response[SYSTEM_CA_SYSTEM] = paramGetSysState.ca_system.state;
                    retVal = E_OK;
                } else if (SYSTEM_ESTB_IP == methodType) {
                    response[SYSTEM_ESTB_IP] = paramGetSysState.estb_ip.state;
                    retVal = E_OK;
                } else if (SYSTEM_ECM_IP == methodType) {
                    response[SYSTEM_ECM_IP] = paramGetSysState.ecm_ip.state;
                    retVal = E_OK;
                } else if (SYSTEM_LAN_IP == methodType) {
                    response[SYSTEM_LAN_IP] = paramGetSysState.lan_ip.state;
                    retVal = E_OK;
                } else if (SYSTEM_MOCA == methodType) {
                    response[SYSTEM_MOCA] = paramGetSysState.moca.state;
                    retVal = E_OK;
                } else if (SYSTEM_DOCSIS == methodType) {
                    response[SYSTEM_DOCSIS] = paramGetSysState.docsis.state;
                    retVal = E_OK;
                } else if (SYSTEM_DSG_BROADCAST_CHANNEL == methodType) {
                    response[SYSTEM_DSG_BROADCAST_CHANNEL] = paramGetSysState.dsg_broadcast_tunnel.state;
                    retVal = E_OK;
                } else if (SYSTEM_DSG_CA_TUNNEL == methodType) {
                    response[SYSTEM_DSG_CA_TUNNEL] = paramGetSysState.dsg_ca_tunnel.state;
                    retVal = E_OK;
                } else if (SYSTEM_CABLE_CARD == methodType) {
                    response[SYSTEM_CABLE_CARD] = paramGetSysState.cable_card.state;
                    retVal = E_OK;
                } else if (SYSTEM_CABLE_CARD_DWNLD == methodType) {
                    response[SYSTEM_CABLE_CARD_DWNLD] = paramGetSysState.cable_card_download.state;
                    retVal = E_OK;
                } else if (SYSTEM_CVR_SUBSYSTEM == methodType) {
                    response[SYSTEM_CVR_SUBSYSTEM] = paramGetSysState.cvr_subsystem.state;
                    retVal= E_OK;
                } else if (SYSTEM_DOWNLOAD == methodType) {
                    response[SYSTEM_DOWNLOAD] = paramGetSysState.download.state;
                    retVal = E_OK;
                } else if (SYSTEM_VOD_AD == methodType) {
                    response[SYSTEM_VOD_AD] = paramGetSysState.vod_ad.state;
                    retVal = E_OK;
                } else {
                    populateResponseWithError(SysSrv_Unexpected, response);
                }
	    } else {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
	    }
            returnResponse(( E_OK == retVal)? true: false);
        }//end of getStateInfo

#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
        /***
         * @brief : To retrieve Device Power State.
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"powerState":"<mode>","success":<bool>}}
         * @return     : Core::<StatusCode>
         */
        uint32_t SystemServices::getDevicePowerState(const JsonObject& parameters,
                JsonObject& response)
        {
            bool retVal = false;
            string powerState;

            {
                std::string currentState = "UNKNOWN";
                IARM_Bus_PWRMgr_GetPowerState_Param_t param;
                IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_GetPowerState,
                    (void*)&param, sizeof(param));

                if (res == IARM_RESULT_SUCCESS) {
                    if (param.curState == IARM_BUS_PWRMGR_POWERSTATE_ON)
                        currentState = "ON";
                    else if ((param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY) || (param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP) || (param.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP))
                        currentState = "STANDBY";
                }
                
                powerState = currentState;
            }

            LOGWARN("getPowerState called, power state : %s\n",
                    powerState.c_str());
            response["powerState"] = powerState;
            if (powerState != "UNKNOWN") {
                retVal = true;
            }
            returnResponse(retVal);
        }//GET POWER STATE END

        /***
         * @brief : To set the Device Power State. The possible value will be STANDBY, DEEP_SLEEP,
         * LIGHT_SLEEP, ON.
         *
         * @param1[in] : {"jsonrpc":"2.0","id":"3","method":"org.rdk.SystemServices.1.setPowerState",
         * "params":{"param":{"powerState":<string>, "standbyReason":<string>}}}
         * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}

         * @return : Core::<StatusCode>
         */
        uint32_t SystemServices::setDevicePowerState(const JsonObject& parameters,
                JsonObject& response)
	{
		bool retVal = false;
		string sleepMode;
		ofstream outfile;
		JsonObject paramIn, paramOut;
		if (parameters.HasLabel("powerState")) {
			string state = parameters["powerState"].String();
			string reason = parameters["standbyReason"].String();
			/* Power state defaults standbyReason is "application". */
			reason = ((reason.length()) ? reason : "application");
            LOGINFO("SystemServices::setDevicePowerState state: %s\n", state.c_str());

            if (state == "STANDBY") {
                if (SystemServices::_instance) {
					SystemServices::_instance->getPreferredStandbyMode(paramIn, paramOut);
					/* TODO: parse abd get the sleepMode from paramOut */
					sleepMode= paramOut["preferredStandbyMode"].String();
					LOGWARN("Output of preferredStandbyMode: '%s'", sleepMode.c_str());
				} else {
					LOGWARN("SystemServices::_instance is NULL.\n");
				}
				if (convert("DEEP_SLEEP", sleepMode)) {
					retVal = setPowerState(sleepMode);
				} else {
					retVal = setPowerState(state);
				}
				outfile.open(STANDBY_REASON_FILE, ios::out);
				if (outfile.is_open()) {
					outfile << reason;
					outfile.close();
				} else {
					LOGERR("Can't open file '%s' for write mode\n", STANDBY_REASON_FILE);
					populateResponseWithError(SysSrv_FileAccessFailed, response);
				}
			} else {
				retVal = setPowerState(state);
			}
            m_current_state=state; /* save the old state */
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(retVal);
	}//end of setPower State
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */

        /***
         * @brief : To check if Reboot has been requested or not.
         *
         * @param1[in]  : query parameter.
         * @param2[out] : {"result":{"rebootRequested":false,"success":<bool>}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::isRebootRequested(const JsonObject& parameters,
                JsonObject& response)
        {
            response["rebootRequested"] = false;
            returnResponse(true);
        }//end of isRebootRequested

        /***
         * @brief : To set GZ Status.
         *
         * @param1[in]  : {"params":{"enabled":true}}
         * @param2[out] : "result":{"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::setGZEnabled(const JsonObject& parameters,
                JsonObject& response)
        {
            bool enabled = false;
	    bool result = false;
	    bool deprecated = true;
	    int32_t retVal = E_NOK;
	    if (parameters.HasLabel("enabled")) {
		    enabled = parameters["enabled"].Boolean();
		    result  = setGzEnabled(enabled);
		    if (true == result) {
			    retVal = E_OK;
		    } else {
			    //do nothing
		    }
	    } else {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
	    }
	    response["deprecated"] = deprecated;
            returnResponse(( E_OK == retVal)? true: false);
        } //ent of SetGZEnabled

        /***
         * @brief : To check GZ Status.
         *
         * @param1[in]  : {"params":{}}
         * @param2[out] : {"result":{"enabled":false,"success":true}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::isGZEnabled(const JsonObject& parameters,
                JsonObject& response)
        {
            bool enabled = false;
	    bool deprecated = true;

            isGzEnabledHelper(enabled);
            response["enabled"] = enabled;
	    response["deprecated"] = deprecated;

            returnResponse(true);
        } //end of isGZEnbaled

        /***
         * @brief : To retrieve STB Version String
         */
        string SystemServices::getStbVersionString()
        {
            bool versionFound = false;
            vector<string> lines;

            if (!m_stbVersionString.empty()) {
                LOGWARN("stb version already found: %s\n", m_stbVersionString.c_str());
                return m_stbVersionString;
            }

            if (getFileContent(VERSION_FILE_NAME, lines)) {
                for (int i = 0; i < (int)lines.size(); ++i) {
                    string line = lines.at(i);

                    if (strstr(line.c_str(), "imagename:")) {
                        std::string gp = line.c_str();
                        std::string delimiter = ":";
                        std::string token = gp.substr((gp.find(delimiter)+1), string::npos);
                        if (token.length()){
                            versionFound = true;
                            m_stbVersionString = token;
                            LOGWARN("m_stbVersion: %s\n", m_stbVersionString.c_str());
                        }
                        if (versionFound) {
                            return m_stbVersionString;
                        } else {
                            LOGWARN("stb version not found in file %s\n",
                                    VERSION_FILE_NAME);
                        }
                    }
                }
            }

#ifdef STB_VERSION_STRING
            {
                m_stbVersionString = string(STB_VERSION_STRING);
            }
#else /* !STB_VERSION_STRING */
            {
                m_stbVersionString = "unknown";
            }
#endif /* !STB_VERSION_STRING */
            LOGWARN("stb version assigned to: %s\n", m_stbVersionString.c_str());
            return m_stbVersionString;
        }

        string SystemServices::getClientVersionString()
        {
            static string clientVersionStr;
            if (clientVersionStr.length())
                return clientVersionStr;

            std::string str;
            std::string str2 = "VERSION=";
            vector<string> lines;

            if (getFileContent(VERSION_FILE_NAME, lines)) {
                for (int i = 0; i < (int)lines.size(); ++i) {
                    string line = lines.at(i);

                    std::string trial = line.c_str();
                    if (!trial.compare(0, 8, str2)) {
                        std::string gp = trial.c_str();
                        std::string delimiter = "=";
                        clientVersionStr = gp.substr((gp.find(delimiter)+1), 12);
                        break;
                    }
                }
                if (clientVersionStr.length()) {
                    LOGWARN("getClientVersionString::client \
                            version found in file: '%s'\n", clientVersionStr.c_str());
                    return clientVersionStr;
                } else {
                    LOGWARN("getClientVersionString::could \
                            not find 'client_version:' in '%s'\n", VERSION_FILE_NAME);
                }
            } else {
                LOGERR("file %s open failed\n", VERSION_FILE_NAME);
            }
#ifdef CLIENT_VERSION_STRING
            return string(CLIENT_VERSION_STRING);
#else
            return "unknown";
#endif
        }

        string SystemServices::getStbTimestampString()
        {
            static string dateTimeStr;

            if (dateTimeStr.length()) {
                return dateTimeStr;
            }

            std::string  buildTimeStr;
            std::string str2 = "BUILD_TIME=";
            vector<string> lines;

            if (getFileContent(VERSION_FILE_NAME, lines)) {
                for (int i = 0; i < (int)lines.size(); ++i) {
                    string line = lines.at(i);
                    std::string trial = line.c_str();

                    if (trial.compare(0, 11, str2) == 0) {
                        std::string gp = trial.c_str();
                        std::string delimiter = "=";
                        buildTimeStr = gp.substr((gp.find(delimiter)+2), 19);
                        char *t1= (char *)buildTimeStr.c_str();
                        dateTimeStr = stringTodate(t1);
                        LOGWARN("versionFound : %s\n", dateTimeStr.c_str());
                        break;
                    }
                }

                if (dateTimeStr.length()) {
                    LOGWARN("getStbTimestampString::stb timestamp found in file: '%s'\n",
                            dateTimeStr.c_str());
                    return dateTimeStr;
                } else {
                    LOGWARN("getStbTimestampString::could not parse BUILD_TIME from '%s' - '%s'\n",
                            VERSION_FILE_NAME, buildTimeStr.c_str());
                }
            } else {
                LOGERR("file %s open failed\n", VERSION_FILE_NAME);
            }

#ifdef STB_TIMESTAMP_STRING
            return string(STB_TIMESTAMP_STRING);
#else
            return "unknown";
#endif
        }
	string SystemServices::getStbBranchString()
	{
		static string stbBranchStr;
		if (stbBranchStr.length())
			return stbBranchStr;

		std::string str;
		std::string str2 = "BRANCH=";
		vector<string> lines;

		if (getFileContent(VERSION_FILE_NAME, lines)) {
			for (int i = 0; i < (int)lines.size(); ++i) {
				string line = lines.at(i);

				std::string trial = line.c_str();
				if (!trial.compare(0, 7, str2)) {
					std::string temp = trial.c_str();
					std::string delimiter = "=";
					temp = temp.substr((temp.find(delimiter)+1));
					delimiter = "_";
					stbBranchStr = temp.substr((temp.find(delimiter)+1));
					break;
				}
			}
			if (stbBranchStr.length()) {
				LOGWARN("getStbBranchString::STB's branch found in file: '%s'\n", stbBranchStr.c_str());
				return stbBranchStr;
			} else {
				LOGWARN("getStbBranchString::could not find 'BRANCH=' in '%s'\n", VERSION_FILE_NAME);
				return "unknown";
			}
		} else {
			LOGERR("file %s open failed\n", VERSION_FILE_NAME);
			return "unknown";
		}
	}

        /***
         * TODO: Stub implementation; Decide whether needed or not since setProperty
         * and getProperty functionalities are XRE/RTRemote dependent.
         *  Note: ThunderWrapper need to handle it.
         */
        bool SystemServices::setProperties(const JsonObject& propertyNames)
        {
            _systemParams = propertyNames;
            return true;
        }

        bool SystemServices::getProperties(const JsonObject& propertyNames,
                JsonObject& returnProperties)
        {
            return true;
        }

        /***
         * @brief : To retrieve system version details
         * @param1[in] : {"params":{}}
         * @aparm2[in] : {"result":{"stbVersion":"<string>",
         *      "receiverVersion":"<string>","stbTimestamp":"<string>","success":<bool>}}
         */
        uint32_t SystemServices::getSystemVersions(const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
	    response["stbVersion"]      = getStbVersionString();
	    string  stbBranchString     = getStbBranchString();            
            std::regex stbBranchString_regex("^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");
            if (std::regex_match (stbBranchString, stbBranchString_regex))
            {             
                    response["receiverVersion"] = stbBranchString;
            }
            else
            {                    
                    response["receiverVersion"] = getClientVersionString();
            }

            response["stbTimestamp"]    = getStbTimestampString();
            status = true;
            returnResponse(status);
        }

        /***
         * @brief : To retrieve is power state is managed by device
         * @param1[in] : {"params":{}}
         * @aparm2[in] : {"result":{"powerStateManagedByDevice":"<bool>",
         *      "success":<bool>}}
         */
        uint32_t SystemServices::getPowerStateIsManagedByDevice(const JsonObject& parameters, JsonObject& response)
        {
            bool status = false;
            bool isPowerStateManagedByDevice = false;
            char *env_var= getenv("RDK_ACTION_ON_POWER_KEY");
            if (env_var)
            {
                int isPowerStateManagedByDeviceValue = atoi(env_var);
                if (1 == isPowerStateManagedByDeviceValue)
                {
                    isPowerStateManagedByDevice = true;
                }
            }
            response["powerStateManagedByDevice"] = isPowerStateManagedByDevice;
            status = true;
            returnResponse(status);
        }

        /***
         * @brief : To retrieve Device Power State before reboot.
         * @param1[in] : {"params":{}}
         * @param2[out] : {"result":{"":"<bool>","success":<bool>}}
         * @return     : Core::<StatusCode>
         */
        uint32_t SystemServices::getPowerStateBeforeReboot (const JsonObject& parameters,
            JsonObject& response)
        {
            bool retVal = false;

            if (m_powerStateBeforeRebootValid) {
                response["state"] = m_powerStateBeforeReboot;
                retVal = true;
                LOGINFO("Got cached powerStateBeforeReboot: '%s'", m_powerStateBeforeReboot.c_str());
            } else {
                IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t param;
                IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                                       IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot, (void *)&param,
                                       sizeof(param));
    
                LOGWARN("getPowerStateBeforeReboot called, current powerStateBeforeReboot is: %s\n",
                         param.powerStateBeforeReboot);
                response["state"] = string (param.powerStateBeforeReboot);
                if (IARM_RESULT_SUCCESS == res) {
                    retVal = true;
                    m_powerStateBeforeReboot = param.powerStateBeforeReboot;
                    m_powerStateBeforeRebootValid = true;
                } else {
                    retVal = false;
                }
            }

            returnResponse(retVal);
        }

#ifdef ENABLE_SET_WAKEUP_SRC_CONFIG
	/***
         * @brief : To set the wakeup source configuration.
         * @param1[in] : {"params":{ "wakeupSrc": <int>, "config": <int>}
         * @param2[out] : {"result":{"success":<bool>}}
         * @return     : Core::<StatusCode>
         */
        uint32_t SystemServices::setWakeupSrcConfiguration(const JsonObject& parameters,
                JsonObject& response)
        {
            bool status = false;
            string src, value;
            WakeupSrcType_t srcType;
            bool config;
            int paramErr = 0;
            IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t param;
            if (parameters.HasLabel("wakeupSrc") && parameters.HasLabel("config")) {
                src = parameters["wakeupSrc"].String();
                srcType = (WakeupSrcType_t)atoi(src.c_str());
                value = parameters["config"].String();
                config = (bool)atoi(value.c_str());

                switch(srcType){
                    case WAKEUPSRC_VOICE:
                    case WAKEUPSRC_PRESENCE_DETECTION:
                    case WAKEUPSRC_BLUETOOTH:
                    case WAKEUPSRC_WIFI:
                    case WAKEUPSRC_IR:
                    case WAKEUPSRC_POWER_KEY:
                    case WAKEUPSRC_TIMER:
                    case WAKEUPSRC_CEC:
                    case WAKEUPSRC_LAN:
                        param.srcType = srcType;
                        param.config = config;
                        break;
                    default:
                        LOGERR("setWakeupSrcConfiguration invalid parameter\n");
                        status = false;
                        paramErr = 1;
                }

                if(paramErr == 0) {

                    IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
                                           IARM_BUS_PWRMGR_API_SetWakeupSrcConfig, (void *)&param,
                                           sizeof(param));

                    if (IARM_RESULT_SUCCESS == res) {
                        status = true;
                    } else {
                        status = false;
                    }
                }
            } else {
                LOGERR("setWakeupSrcConfiguration Missing Key Values\n");
                populateResponseWithError(SysSrv_MissingKeyValues, response);
            }
            returnResponse(status);
        }
#endif //ENABLE_SET_WAKEUP_SRC_CONFIG

        /***
         * @brief : To handle the event of Power State change.
         *     The event is registered to the IARM event handle on powerStateChange.
         *     Connects the change event to SystemServices::onSystemPowerStateChanged()
         *
         * @param1[in]  : owner of the event
         * @param2[in]  : eventID of the event
         * @param3[in]  : data passed from the IARMBUS event
         * @param4[in]  : len
         * @param2[out] : connect call to onSystemPowerStateChanged
         * @return      : <void>
         */
        void _powerEventHandler(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len)
        {
            switch (eventId) {
                case  IARM_BUS_PWRMGR_EVENT_MODECHANGED:
                    {
                        IARM_Bus_PWRMgr_EventData_t *eventData = (IARM_Bus_PWRMgr_EventData_t *)data;
			std::string curState,newState = "";

			if(eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_ON) {
				curState = "ON";
            } else if (eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_OFF) {
                curState = "OFF";
			} else if ((eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY)||
				   (eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP)) {
				curState = "LIGHT_SLEEP";
			} else if (eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP) {
				curState = "DEEP_SLEEP";
			} else if (eventData->data.state.curState == IARM_BUS_PWRMGR_POWERSTATE_OFF) {
				curState = "OFF";
			}

			if(eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_ON) {
				newState = "ON";
            } else if(eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_OFF) {
                newState = "OFF";
			} else if((eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY)||
				  (eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP)) {
                                newState = "LIGHT_SLEEP";
			} else if(eventData->data.state.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP) {
                                newState = "DEEP_SLEEP";
			}
                        LOGWARN("IARM Event triggered for PowerStateChange.\
                                Old State %s, New State: %s\n",
                                curState.c_str() , newState.c_str());
                        if (SystemServices::_instance) {
                            SystemServices::_instance->onSystemPowerStateChanged(curState, newState);
                        } else {
                            LOGERR("SystemServices::_instance is NULL.\n");
                        }
                    }
                    
                    break;
                case  IARM_BUS_PWRMGR_EVENT_REBOOTING:
                    {
                        IARM_Bus_PWRMgr_RebootParam_t *eventData = (IARM_Bus_PWRMgr_RebootParam_t *)data;

                        if (SystemServices::_instance) {
                            SystemServices::_instance->onPwrMgrReboot(eventData->requestor, eventData->reboot_reason_other);
                        } else {
                            LOGERR("SystemServices::_instance is NULL.\n");
                        }
                    }

                    break;

            case  IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED:
                {
                    IARM_Bus_PWRMgr_EventData_t *eventData = (IARM_Bus_PWRMgr_EventData_t *)data;

                    if (SystemServices::_instance) {
                        SystemServices::_instance->onNetorkModeChanged(eventData->data.bNetworkStandbyMode);
                    } else {
                        LOGERR("SystemServices::_instance is NULL.\n");
                    }
                }

                break;
            }
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        /***
         * @brief : To receive System Mode Changed Event from IARM
         * @param1[in] : pointer to received data buffer.
         */
        IARM_Result_t _SysModeChange(void *arg)
        {
            IARM_Bus_CommonAPI_SysModeChange_Param_t *param =
                (IARM_Bus_CommonAPI_SysModeChange_Param_t *)arg;

            std::string mode = iarmModeToString(param->newMode);

#ifdef HAS_API_POWERSTATE
            if (SystemServices::_instance) {
                SystemServices::_instance->onSystemModeChanged(mode);
            } else {
                LOGERR("SystemServices::_instance is NULL.\n");
            }
#else
            LOGINFO("HAS_API_POWERSTATE is not defined.\n");
#endif /* HAS_API_POWERSTATE */
            return IARM_RESULT_SUCCESS;
        }

        /***
         * @brief : To receive Firmware Update State Change events from IARM.
         * @param1[in]  : owner of the event
         * @param2[in]  : eventID of the event
         * @param3[in]  : data passed from the IARMBUS event
         * @param4[in]  : len
         */
        void _systemStateChanged(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len)
        {
            int seconds = 600; /* 10 Minutes to Reboot */

            LOGINFO("len = %zu\n", len);
            /* Only handle state events */
            if (eventId != IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE) return;

            IARM_Bus_SYSMgr_EventData_t *sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
            IARM_Bus_SYSMgr_SystemState_t stateId = sysEventData->data.systemStates.stateId;
            int state = sysEventData->data.systemStates.state;

            switch (stateId) {
                case IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE:
                    {
                        LOGWARN("IARMEvt: IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE = '%d'\n", state);
                        if (SystemServices::_instance)
                        {
                            if (IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_CRITICAL_REBOOT == state) {
                                LOGWARN(" Critical reboot is required. \n ");
                                SystemServices::_instance->onFirmwarePendingReboot(seconds);
                            } else {
                                SystemServices::_instance->onFirmwareUpdateStateChange(state);
                            }
                        } else {
                            LOGERR("SystemServices::_instance is NULL.\n");
                        }
                    } break;

                case IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE:
                    {
                        if (sysEventData->data.systemStates.state)
                        {
                            LOGWARN("Clock is set.");
                            if (SystemServices::_instance) {
                                SystemServices::_instance->onClockSet();
                            } else {
                                LOGERR("SystemServices::_instance is NULL.\n");
                            }
                        }
                    } break;
                case IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD:
                    {
                        LOGWARN("IARMEvt: IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD = '%d'", state);
                        if (SystemServices::_instance)
                        {
                            SystemServices::_instance->onLogUpload(state);
                        } else {
                            LOGERR("SystemServices::_instance is NULL.\n");
                        }
                    } break;


                default:
                    /* Nothing to do. */;
            }
        }

#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
#ifdef ENABLE_THERMAL_PROTECTION
        /***
         * @brief : To handle the event of Thermal Level change. THe event is registered
         *	       to the IARM event handle on _EVENT_THERMAL_MODECHANGED.
         *
         * @param1[in]  : owner of the event
         * @param2[in]  : eventID of the event
         * @param3[in]  : data passed from the IARMBUS event
         * @param4[in]  : len

         * @param2[out] : connect call to onTemperatureThresholdChanged
         * @return      : <void>
         */
        void _thermMgrEventsHandler(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len)
        {
            if (!strcmp(IARM_BUS_PWRMGR_NAME, owner)) {
                if (IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED  == eventId) {
                    LOGWARN("IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED event received\n");
                    handleThermalLevelChange((IARM_Bus_PWRMgr_EventData_t *)data);
                }
            }
        }

        /***
         * @brief : To validate the parameters in event data send from the IARMBUS, so as
         *		   to initiate  onTemperatureThresholdChanged event.
         *
         * @param1[in]  : data passed from the IARMBUS event

         * @param2[out] : connect call to onTemperatureThresholdChanged
         * @return      : <void>
         */
        static void handleThermalLevelChange(IARM_Bus_PWRMgr_EventData_t *param)
        {
            bool crossOver;
            bool validparams = true;
            std::string thermLevel;

            switch (param->data.therm.newLevel) {
                case IARM_BUS_PWRMGR_TEMPERATURE_NORMAL:
                    {
                        switch (param->data.therm.curLevel) {
                            case IARM_BUS_PWRMGR_TEMPERATURE_HIGH:
                            case IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL:
                                crossOver = false;
                                thermLevel = "WARN";
                                break;
                            default:
                                validparams = false;
                                LOGERR("[%s] Invalid temperature levels \n", __FUNCTION__);
                        }

                    }
                    break;
                case IARM_BUS_PWRMGR_TEMPERATURE_HIGH:
                    {
                        switch (param->data.therm.curLevel) {
                            case IARM_BUS_PWRMGR_TEMPERATURE_NORMAL:
                                crossOver = true;
                                thermLevel = "WARN";
                                break;
                            case IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL:
                                crossOver = false;
                                thermLevel = "MAX";
                                break;
                            default:
                                validparams = false;
                                LOGERR("Invalid temperature levels \n");
                        }

                    }
                    break;
                case IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL:
                    {
                        switch (param->data.therm.curLevel) {
                            case IARM_BUS_PWRMGR_TEMPERATURE_HIGH:
                            case IARM_BUS_PWRMGR_TEMPERATURE_NORMAL:
                                crossOver = true;
                                thermLevel = "MAX";
                                break;
                            default:
                                validparams = false;
                                LOGERR("Invalid temperature levels \n");
                        }

                    }
                    break;
                default:
                    validparams = false;
                    LOGERR("Invalid temperature levels \n");
            }
            if (validparams) {
                LOGWARN("Invalid temperature levels \n");
                if (SystemServices::_instance) {
                    SystemServices::_instance->onTemperatureThresholdChanged(thermLevel,
                            crossOver, param->data.therm.curTemperature);
                } else {
                    LOGERR("SystemServices::_instance is NULL.\n");
                }
            }
        }
#endif /* ENABLE_THERMAL_PROTECTION */

        /***
         * @brief : sends notification when system reboot is requested.
         *
         * @param1[in]  : string; requested application name
         * @param2[in]  : string; reboot reason
         * @Event [out] : {"rebootReason": <string_ReasonForReboot>}
         */
        void SystemServices::onRebootRequest(string reason)
        {
            JsonObject params;
            params["rebootReason"] = reason;
            LOGINFO("Notifying onRebootRequest\n");
            sendNotify(EVT_ONREBOOTREQUEST, params);
        }

        /***
         * @brief : upload STB logs to the specified URL.
         * @param1[in] : url::String
         */
        uint32_t SystemServices::uploadLogs(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();

            bool success = false;

            string url;
            getStringParameter("url", url);
            auto err = UploadLogs::upload(url);
            if (err != UploadLogs::OK)
                response["error"] = UploadLogs::errToText(err);
            else
                success = true;

            returnResponse(success);
        }

        uint32_t SystemServices::getLastFirmwareFailureReason(const JsonObject& parameters, JsonObject& response)
        {
            bool retStatus = true;
            FwFailReason failReason = FwFailReasonNone;

            std::vector<string> lines;
            if (getFileContent(FWDNLDSTATUS_FILE_NAME, lines)) {
                std::string str;
                for (auto i = lines.begin(); i != lines.end(); ++i) {
                    std::smatch m;
                    if (std::regex_match(*i, m, std::regex("^FailureReason\\|(.*)$"))) {
                        str = m.str(1);
                    }
                }

                LOGINFO("Lines read:%d. FailureReason|%s", (int) lines.size(), C_STR(str));

                auto it = find_if(FwFailReasonFromText.begin(), FwFailReasonFromText.end(),
                                  [&str](const pair<string, FwFailReason> & t) {
                                      return strcasecmp(C_STR(t.first), C_STR(str)) == 0;
                                  });
                if (it != FwFailReasonFromText.end())
                    failReason = it->second;
                else if (!str.empty())
                    LOGWARN("Unrecognised FailureReason!");
            } else {
                LOGINFO("Could not read file %s", FWDNLDSTATUS_FILE_NAME);
            }

            response["failReason"] = FwFailReasonToText.at(failReason);
            returnResponse(retStatus);
        }
        /***
         * @brief : To set Telemetry Opt Out Status
         *
         * @param1[in]  : {"params":{"Opt-Out":<bool>}}
         * @param2[out] : "result":{"success":<bool>}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::setOptOutTelemetry(
                const JsonObject& parameters,
                JsonObject& response)
        {
            bool optout = false;
	    bool result = false;
            ofstream optfile;
	    if (parameters.HasLabel("Opt-Out")) {
		    optout = parameters["Opt-Out"].Boolean();
                    optfile.open(OPTOUT_TELEMETRY_STATUS, ios::out);
                    if (optfile) {
                        optfile << (optout ? "true" :"false");
                        optfile.close();
                        result = true;
                        LOGINFO("TelemetryOptOut flag set to %s\n", 
                                optout ? "true" :"false");
                    } else {
                        LOGERR("Couldn't update Telemetry Opt Out flag\n");
                    }
	    } else {
		    populateResponseWithError(SysSrv_MissingKeyValues, response);
	    }
            returnResponse(result);
        } //end of setOptOutTelemetry

        /***
         * @brief : To check Telemetry Opt Out status
         *
         * @param1[in]  : {"params":{}}
         * @param2[out] : {"result":{"Opt-Out":<bool>,"success":true}}
         * @return      : Core::<StatusCode>
         */
        uint32_t SystemServices::isOptOutTelemetry(const JsonObject& parameters,
                JsonObject& response)
        {
            bool optout = false;
            string optOutStatus;

            bool retVal = getFileContent(OPTOUT_TELEMETRY_STATUS, optOutStatus);
            if (retVal && optOutStatus.length()) {
                if (optOutStatus.find("true") != string::npos) {
                    optout = true;
                } else {
                    optout = false;
                }
            }

            LOGINFO("Current TelemetryOptOut flag is %d\n", optout);
            response["Opt-Out"] = optout;
            returnResponse(true);
        } //end of isOptOutTelemetry

        uint32_t SystemServices::getStoreDemoLink(const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;
            if (Utils::fileExists(STORE_DEMO_FILE)) {
                result = true;
                response["fileURL"] = STORE_DEMO_LINK;
            } else {
                response["error"] = "missing";
            }
            returnResponse(result);
        }

        /***
         * @brief : Deletes persistent path associated with a callsign
         *
         * @param[in]   : callsign: string - the callsign for which to delete persistent path
         * @return      : none
         */
        uint32_t SystemServices::deletePersistentPath(const JsonObject& parameters, JsonObject& response)
        {
          LOGINFOMETHOD();

          bool result = false;

          do
          {
            if (m_shellService == nullptr)
            {
              response["message"] = "internal: service shell is unavailable";
              break;
            }

            if (parameters.HasLabel("callsign") == false && parameters.HasLabel("type") == false)
            {
              response["message"] = "no 'callsign' (nor 'type' of execution envirionment) specified";
              break;
            }

            std::string callsignOrType = parameters.HasLabel("callsign")
              ? parameters.Get("callsign").String()
              : parameters.Get("type").String();
            if (callsignOrType.empty() == true)
            {
              response["message"] = "specified 'callsign' or 'type' is empty";
              break;
            }

            PluginHost::IShell* service(m_shellService->QueryInterfaceByCallsign<PluginHost::IShell>(callsignOrType));
            if (service == nullptr)
            {
              response["message"] = "no service found for: '" + callsignOrType + "'";
              break;
            }

            // Special case for Netflix
            if (service->ClassName().compare(0, 7, "Netflix") == 0)
            {
              Core::File file(string("/opt/netflix"));
              if (file.Exists())
              {
                if (file.IsDirectory() == true)
                {
                  Core::Directory dir(file.Name().c_str());
#ifndef USE_THUNDER_R4
                  if (dir.Destroy(true) == false)
#else
                  if (dir.Destroy() == false)
#endif
                  {
                    response["message"] = "failed to delete dir: '" + file.Name() + "'";
                    break;
                  }
                }
                if (file.Destroy() == false)
                {
                  response["message"] = "failed to delete: '" + file.Name() + "'";
                  break;
                }
              }
            }

            std::string persistentPath = service->PersistentPath();

            Core::File file(persistentPath);
            if (file.Exists() == false)
            {
              LOGINFO("persistent path '%s' for '%s' does not exist, return success = true", persistentPath.c_str(), callsignOrType.c_str());
              result = true;
              break;
            }

            if (file.IsDirectory() == true)
            {
              Core::Directory dir(persistentPath.c_str());
#ifndef USE_THUNDER_R4
              if (dir.Destroy(true) == false)
#else
              if (dir.Destroy() == false)
#endif
              {
                response["message"] = "failed to delete dir: '" + persistentPath + "'";
                break;
              }
            }

            if (file.Destroy() == false)
            {
                response["message"] = "failed to delete: '" + persistentPath + "'";
                break;
            }

            // Everything is OK
            LOGINFO("Successfully deleted persistent path for '%s' (path = '%s')", callsignOrType.c_str(), persistentPath.c_str());

	    //Calling container_setup.sh along with callsign as container bundle also gets deleted from the persistent path
            std::string command = "/lib/rdk/container_setup.sh " + callsignOrType;
            system(command.c_str());
            LOGINFO("Calling %s \n", command.c_str());

            result = true;

          } while(false);

          if (!result)
          {
            std::string errorMessage = response["message"].String();
            LOGERR("Failed to delete persistent path. Error: %s", errorMessage.c_str());
          }

          returnResponse(result);
        }

        uint32_t SystemServices::getPlatformConfiguration(const JsonObject &parameters, PlatformCaps &response)
        {
          LOGINFOMETHOD();

          const string query = parameters.HasLabel("query") ? parameters["query"].String() : "";

          response.Load(m_shellService, query);

          return Core::ERROR_NONE;
        }
    } /* namespace Plugin */
} /* namespace WPEFramework */

