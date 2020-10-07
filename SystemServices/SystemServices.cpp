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

/**
 * @file SystemServices.cpp
 * @brief Thunder Plugin based Implementation for System service API's.
 * @reference RDK-25849.
 */
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
#include "utils.h"

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#endif /* USE_IARMBUS || USE_IARM_BUS */

#ifdef ENABLE_THERMAL_PROTECTION
#include "thermonitor.h"
#endif /* ENABLE_THERMAL_PROTECTION */

#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
#include "powerstate.h"
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */

#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
#include "mfrMgr.h"
#endif

using namespace std;

#define SYSSRV_MAJOR_VERSION 1
#define SYSSRV_MINOR_VERSION 0

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

    char lines[32] = {'\0'};
    string gzStatus = "";
    retVal = getFileContentToCharBuffer(GZ_STATUS.c_str(), lines);
    if (retVal) {
        gzStatus = strtok(lines," ");
        if ("true" == gzStatus) {
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

#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

/**
 * @brief WPEFramework class for SystemServices
 */
namespace WPEFramework {
    namespace Plugin {
        //Prototypes
        std::string   SystemServices::m_currentMode = "";
        cTimer    SystemServices::m_operatingModeTimer;
        int       SystemServices::m_remainingDuration = 0;
        JsonObject SystemServices::_systemParams;
        const string SystemServices::MODEL_NAME = "modelName";
        const string SystemServices::HARDWARE_ID = "hardwareID";
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
        static void _firmwareUpdateStateChanged(const char *owner,
                IARM_EventId_t eventId, void *data, size_t len);
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

        SERVICE_REGISTRATION(SystemServices, SYSSRV_MAJOR_VERSION,
                SYSSRV_MINOR_VERSION);

        SystemServices* SystemServices::_instance = nullptr;
        cSettings SystemServices::m_temp_settings(SYSTEM_SERVICE_TEMP_FILE);

        /**
         * Register SystemService module as wpeframework plugin
         */
        SystemServices::SystemServices()
            : AbstractPlugin()
              , m_cacheService(SYSTEM_SERVICE_SETTINGS_FILE)
        {
            SystemServices::_instance = this;

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
            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            registerMethod("sampleSystemServiceAPI", &SystemServices::sampleAPI, this);
#endif /* DEBUG */
            registerMethod("getDeviceInfo", &SystemServices::getDeviceInfo, this);
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
#endif /* ENABLE_THERMAL_PROTECTION */
            registerMethod("getPreviousRebootInfo2",
                    &SystemServices::getPreviousRebootInfo2, this);
            registerMethod("getPreviousRebootReason",
                    &SystemServices::getPreviousRebootReason, this);
            registerMethod("getRFCConfig", &SystemServices::getRFCConfig, this);
            registerMethod("getMilestones", &SystemServices::getMilestones, this);
            registerMethod("getSystemVersions", &SystemServices::getSystemVersions, this);
            registerMethod("setNetworkStandbyMode", &SystemServices::setNetworkStandbyMode, this);
            registerMethod("getNetworkStandbyMode", &SystemServices::getNetworkStandbyMode, this);
        }

        SystemServices::~SystemServices()
        {
            SystemServices::_instance = nullptr;
        }

        const string SystemServices::Initialize(PluginHost::IShell*)
        {
            LOGINFO();
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void SystemServices::Deinitialize(PluginHost::IShell*)
        {
            LOGINFO();
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void SystemServices::InitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_RegisterCall(IARM_BUS_COMMON_API_SysModeChange, _SysModeChange));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, _firmwareUpdateStateChanged));
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, _powerEventHandler));
#ifdef ENABLE_THERMAL_PROTECTION
                IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, _thermMgrEventsHandler));
#endif //ENABLE_THERMAL_PROTECTION
            }
        }

        void SystemServices::DeinitializeIARM()
        {
            LOGINFO();

            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE));
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED));
    #ifdef ENABLE_THERMAL_PROTECTION
                IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED));
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
            string rebootCommand = "";
            string rebootReason = "";
            string customReason = "";
            string otherReason = "";
            bool result = false;

            //TODO: Replace system command
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
            if (Utils::fileExists("/rebootNow.sh")) {
                rebootCommand = "/rebootNow.sh";
            } else if (Utils::fileExists("/lib/rdk/rebootNow.sh")) {
                rebootCommand = "/lib/rdk/rebootNow.sh";
            } else {
                LOGINFO("rebootNow.sh is not present in /lib/rdk or \
                        /root\n");
            }

            if (!(rebootCommand.empty())) {
                rebootReason = "System Plugin";
                customReason = "No custom reason provided";
                otherReason = "No other reason supplied";

                if (parameters.HasLabel("rebootReason")) {
                    customReason = parameters["rebootReason"].String();
                    otherReason = customReason;
                }

                rebootCommand += " -s \"" + rebootReason + "\"";
                rebootCommand += " -r \"" + customReason + "\"";
                rebootCommand += " -o \"" + otherReason + "\"";
                rebootCommand += " &";

                LOGINFO("IARM_BUS RunScript: '%s'\n", rebootCommand.c_str());
                IARM_Bus_SYSMgr_RunScript_t runScriptParam;
                runScriptParam.return_value = -1;
                strcpy(runScriptParam.script_path, rebootCommand.c_str());
                IARM_Result_t iarmcallstatus = IARM_Bus_Call(IARM_BUS_SYSMGR_NAME,
                        IARM_BUS_SYSMGR_API_RunScript,
                        &runScriptParam, sizeof(runScriptParam));

                nfxResult = !runScriptParam.return_value;
                response["IARM_Bus_Call_STATUS"] = nfxResult;
                result = true;

                /* Trigger rebootRequest event if IARMCALL is success. */
                if (IARM_RESULT_SUCCESS == iarmcallstatus) {
                    SystemServices::_instance->onRebootRequest(customReason);
                } else {
                    LOGERR("iarmcallstatus = %d; onRebootRequest event will not be fired.\n",
                            iarmcallstatus);
                }
            } else {
                LOGINFO("Rebooting failed as rebootNow.sh is not present\n");
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }
            returnResponse(result);
        }//end of requestSystemReboot

        /***
         * @brief : send notification when system power state is changed
         *
         * @param1[in]  : powerState
         * @param2[out] : {"jsonrpc": "2.0",
         *		"method": "org.rdk.SystemServices.events.1.onSystemPowerStateChanged",
         *		"param":{"powerState": <string new power state mode>}}
         */
        void SystemServices::onSystemPowerStateChanged(string powerState)
        {
            JsonObject params;
            params["powerState"] = powerState;
            LOGINFO("power state changed to '%s'", powerState.c_str());
            sendNotify(EVT_ONSYSTEMPOWERSTATECHANGED, params);
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
            string queryParams = parameters["params"].String();
            removeCharsFromString(queryParams, "[\"]");
#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
            if (!queryParams.compare(MODEL_NAME) || !queryParams.compare(HARDWARE_ID)) {
                returnResponse(getManufacturerData(queryParams, response));
            }
#endif
            string methodType = queryParams;
            string respBuffer;
            string fileName = "/tmp/." + methodType;
            LOGERR("accessing fileName : %s\n", fileName.c_str());
            if (Utils::fileExists(fileName.c_str())) {
                respBuffer = collectDeviceInfo(methodType);
                removeCharsFromString(respBuffer, "\n\r");
                LOGERR("respBuffer : %s\n", respBuffer.c_str());
                if (respBuffer.length() <= 0) {
                    populateResponseWithError(SysSrv_FileAccessFailed, response);
                } else {
                    response[methodType.c_str()] = respBuffer;
                    retAPIStatus = true;
                }
            } else {
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }
            returnResponse(retAPIStatus);
        }

#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
        bool SystemServices::getManufacturerData(const string& parameter, JsonObject& response)
        {
            LOGWARN("SystemService getDeviceInfo query %s", parameter.c_str());

            IARM_Bus_MFRLib_GetSerializedData_Param_t param;
            param.bufLen = 0;
            param.type = mfrSERIALIZED_TYPE_MANUFACTURER;
            if (!parameter.compare(MODEL_NAME)) {
                param.type = mfrSERIALIZED_TYPE_SKYMODELNAME;
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
            string command("/lib/rdk/deviceInitiatedFWDnld.sh 0 4 >> /opt/logs/swupdate.log &");
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

            int updateAvailableEnum = 0;
            if (firmwareUpdateVersion.length() > 0) {
                params["firmwareUpdateVersion"] = firmwareUpdateVersion.c_str();
                if (firmwareUpdateVersion.compare(firmwareVersion)) {
                    updateAvailableEnum = 0;
                } else {
                    updateAvailableEnum = 1;
                }
            } else {
                params["firmwareUpdateVersion"] = "";
                updateAvailableEnum = 2;
            }
            params["updateAvailable"] = !updateAvailableEnum ;
            params["updateAvailableEnum"] = updateAvailableEnum;
            params["success"] = success;

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
            string model;
            string firmwareVersion;
            string eStbMac = "";
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

            string ipAddress = collectDeviceInfo("estb_ip");
            removeCharsFromString(ipAddress, "\n\r");
            model = getModel();

            eStbMac = collectDeviceInfo("estb_mac");
            removeCharsFromString(eStbMac, "\n\r");
            LOGWARN("ipAddress = '%s', eStbMac = '%s'\n", (ipAddress.empty()? "empty" : ipAddress.c_str()),
                (eStbMac.empty()? "empty" : eStbMac.c_str()));

            std::string response;
            long http_code = 0;
            CURL *curl_handle = NULL;
            CURLcode res = CURLE_OK;
            firmwareUpdate _fwUpdate;
            string pdriVersion = "";
            string partnerId = "";
            string accountId = "";
            string match = "http://";
            std::vector<std::pair<std::string, std::string>> fields;

            string xconfOverride = getXconfOverrideUrl();
            string fullCommand = (xconfOverride.empty()? URL_XCONF : xconfOverride);
            size_t start_pos = fullCommand.find(match);
            if (std::string::npos != start_pos) {
                fullCommand.replace(start_pos, match.length(), "https://");
            }
            LOGWARN("fullCommand : '%s'\n", fullCommand.c_str());
            pdriVersion = Utils::cRunScript("/usr/bin/mfr_util --PDRIVersion");
            pdriVersion = trim(pdriVersion);

            partnerId = Utils::cRunScript("sh -c \". /lib/rdk/getPartnerId.sh; getPartnerId\"");
            partnerId = trim(partnerId);

            accountId = Utils::cRunScript("sh -c \". /lib/rdk/getAccountId.sh; getAccountId\"");
            accountId = trim(accountId);

            string timeZone = getTimeZoneDSTHelper();
            string utcDateTime = currentDateTimeUtc("%a %B %e %I:%M:%S %Z %Y");
            LOGINFO("timeZone = '%s', utcDateTime = '%s'\n", timeZone.c_str(), utcDateTime.c_str());

            curl_handle = curl_easy_init();
            _fwUpdate.success = false;

            if (curl_handle) {
                struct curl_slist *headers = NULL;

                /* url encode the payload portion alone. */
                fields.push_back(make_pair("eStbMac", urlEncodeField(curl_handle, eStbMac)));
                fields.push_back(make_pair("env", urlEncodeField(curl_handle, env)));
                fields.push_back(make_pair("model", urlEncodeField(curl_handle, model)));
                fields.push_back(make_pair("timezone", urlEncodeField(curl_handle, timeZone)));
                fields.push_back(make_pair("localtime", urlEncodeField(curl_handle, utcDateTime)));
                fields.push_back(make_pair("firmwareVersion", urlEncodeField(curl_handle, firmwareVersion)));
                fields.push_back(make_pair("capabilities", "rebootDecoupled"));
                fields.push_back(make_pair("capabilities", "RCDL"));
                fields.push_back(make_pair("capabilities", "supportsFullHttpUrl"));
                fields.push_back(make_pair("additionalFwVerInfo", urlEncodeField(curl_handle, pdriVersion)));
                fields.push_back(make_pair("partnerId", urlEncodeField(curl_handle, partnerId)));
                fields.push_back(make_pair("accountID", urlEncodeField(curl_handle, accountId)));

                for (std::vector<std::pair<std::string, std::string>>::const_iterator iter = fields.begin();
                        iter != fields.end(); ++iter) {
                    if (iter == fields.begin()) {
                        fullCommand += "?" + iter->first + "=" + iter->second;
                    } else {
                        fullCommand += "&" + iter->first + "=" + iter->second;
                    }
                }
                LOGINFO("curl url (enc): '%s'\n", fullCommand.c_str());

                curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
                curl_easy_setopt(curl_handle, CURLOPT_URL, fullCommand.c_str());
                /* when redirected, follow the redirections */
                curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeCurlResponse);
                curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
                curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
                curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L);
                curl_easy_setopt(curl_handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 3000L);
                curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
                curl_easy_setopt(curl_handle, CURLOPT_TRANSFER_ENCODING, 1L);
                //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
                //curl_easy_setopt(curl_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
                curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
                res = curl_easy_perform(curl_handle);
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
                LOGWARN("curl result code: %d, http response code: %ld\n", res, http_code);
                if (CURLE_OK != res) {
                    LOGERR("curl_easy_perform failed; reason: '%s'\n", curl_easy_strerror(res));
                }
                _fwUpdate.httpStatus = http_code;
                curl_easy_cleanup(curl_handle);
            } else {
                LOGWARN("Could not perform curl\n");
            }

            if (CURLE_OK == res) {
                LOGINFO("curl response '%s'\n", response.c_str());
                JsonObject httpResp;
                httpResp.FromString(response.c_str());
                _fwUpdate.firmwareUpdateVersion = httpResp["firmwareVersion"].String();
                LOGWARN("fwVersion: '%s'\n", _fwUpdate.firmwareUpdateVersion.c_str());
                _fwUpdate.success = true;
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
            string callGUID;

                callGUID = parameters["GUID"].String();
            LOGINFO("GUID = %s\n", callGUID.c_str());
                if (m_getFirmwareInfoThread.joinable()) {
                    m_getFirmwareInfoThread.join();
                }
                m_getFirmwareInfoThread = std::thread(firmwareUpdateInfoReceived);
                response["asyncResponse"] = true;
            returnResponse(true);
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
            } else {
                retVal = false;
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
                const device::List<device::SleepMode> sleepModes =
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
            bool ret = false;
            std::string curlResponse;
            struct write_result write_result_buf;
            CURLcode res = CURLE_OK;
            CURL *curl = curl_easy_init();
            char *data;
            long http_code = 0;
            struct curl_slist *headers = NULL;

            data = (char*)malloc(CURL_BUFFER_SIZE);
            if (!data) {
                LOGERR("Error allocating %d bytes.\n", CURL_BUFFER_SIZE);
                populateResponseWithError(SysSrv_DynamicMemoryAllocationFailed, response);
                return ret;
            }
            write_result_buf.data = data;
            write_result_buf.pos = 0;

            if (curl) {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:10999/");
                headers = curl_slist_append(headers, "cache-control: no-cache");
                headers = curl_slist_append(headers, "content-type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                        "{\"paramList\":[{\"name\":\"Device.DeviceInfo.SerialNumber\"}]}");
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result_buf);
                res = curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

                /* null terminate the string */
                data[write_result_buf.pos] = '\0';
                curlResponse = data;
                free(data);
                curl_easy_cleanup(curl);
                curl_global_cleanup();
            }
            if (CURLE_OK == res) {
                /* Eg: {"paramList":[{"name":"Device.DeviceInfo.SerialNumber",
                   "value":"M11806TK0519"}]} */
                LOGWARN("curl response : %s\n", curlResponse.c_str());
                JsonObject curlRespJson;
                JsonArray paramListJson;
                curlRespJson.FromString(curlResponse);
                paramListJson = curlRespJson["paramList"].Array();

                for (int i = 0; i < paramListJson.Length(); i++) {
                    curlRespJson = paramListJson[i].Object();
                    response["serialNumber"] = curlRespJson["value"].String();
                    ret = true;
                    break;
                }
            } else {
                populateResponseWithError(SysSrv_LibcurlError, response);
            }
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
		    populateResponseWithError(SysSrv_FileNotPresent, response);
		    returnResponse(retStat);
	    }

            if (getFileContent(FWDNLDSTATUS_FILE_NAME, lines)) {
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
            FirmwareUpdateState fwUpdateState = FirmwareUpdateStateUninitialized;
            std::vector<string> lines;
            if (!Utils::fileExists(FWDNLDSTATUS_FILE_NAME)) {
                populateResponseWithError(SysSrv_FileNotPresent, response);
                returnResponse(retStatus);
            }
            if (getFileContent(FWDNLDSTATUS_FILE_NAME, lines)) {
                for (std::vector<std::string>::const_iterator i = lines.begin();
                        i != lines.end(); ++i) {
                    std::string line = *i;
                    std::size_t found = line.find("FwUpdateState|");
                    std::string delimiter = "|";
                    size_t pos = 0;
                    std::string token;
                    if (std::string::npos != found) {
                        while ((pos = line.find(delimiter)) != std::string::npos) {
                            token = line.substr(0, pos);
                            line.erase(0, pos + delimiter.length());
                        }
                        line = std::regex_replace(line, std::regex("^ +| +$"), "$1");

                        if (!strcmp(line.c_str(), "Requesting")) {
                            fwUpdateState = FirmwareUpdateStateRequesting;
                        } else if (!strcmp(line.c_str(), "Downloading")) {
                            fwUpdateState = FirmwareUpdateStateDownloading;
                        } else if (!strcmp(line.c_str(), "Failed")) {
                            fwUpdateState = FirmwareUpdateStateFailed;
                        } else if (!strcmp(line.c_str(), "Download complete")) {
                            fwUpdateState = FirmwareUpdateStateDownloadComplete;
                        } else if (!strcmp(line.c_str(), "Validation complete")) {
                            fwUpdateState = FirmwareUpdateStateValidationComplete;
                        } else if (!strcmp(line.c_str(), "Preparing to reboot")) {
                            fwUpdateState = FirmwareUpdateStatePreparingReboot;
                        }
                    }
                }
                response["firmwareUpdateState"] = (int)fwUpdateState;
                retStatus = true;
            } else {
                LOGERR("Could not read file %s\n", FWDNLDSTATUS_FILE_NAME);
                populateResponseWithError(SysSrv_FileNotPresent, response);
            }
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
            params["firmwareUpdateStateChange"] = (int)firmwareUpdateState;
            LOGINFO("New firmwareUpdateState = %d\n", (int)firmwareUpdateState);
            sendNotify(EVT_ONFIRMWAREUPDATESTATECHANGED, params);
        }

        /***
         * @brief : Worker to fetch details of various MAC addresses.
         * @Event : {"ecm_mac":"<MAC>","estb_mac":"<MAC>","moca_mac":"<MAC>",
         *     "eth_mac":"<MAC>","wifi_mac":"<MAC>","info":"Details fetch status",
         *     "success":<bool>}
         */
        void SystemServices::getMacAddressesAsync(SystemServices *pSs)
        {
            int i, listLength = 0;
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
                if (thread_getMacAddresses.joinable())
                    thread_getMacAddresses.join();

                thread_getMacAddresses = std::thread(getMacAddressesAsync, this);
                response["asyncResponse"] = true;
                status = true;
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
		bool resp = false;
		if (parameters.HasLabel("timeZone")) {
			std::string dir = dirnameOf(TZ_FILE);
			ofstream outfile;
			std::string timeZone = "";
			try {
				timeZone = parameters["timeZone"].String();
				if (timeZone.empty() || (timeZone == "null")) {
					LOGERR("Empty timeZone received.");
				} else {
					if (!dirExists(dir)) {
						std::string command = "mkdir -p " + dir + " \0";
						Utils::cRunScript(command.c_str());
					} else {
						//Do nothing//
					}

					outfile.open(TZ_FILE,ios::out);
					if (outfile) {
						outfile << timeZone;
						outfile.close();
						LOGWARN("Set TimeZone: %s\n", timeZone.c_str());
						resp = true;
					} else {
						LOGERR("Unable to open %s file.\n", TZ_FILE);
						populateResponseWithError(SysSrv_FileAccessFailed, response);
						resp = false;
					}
				}
			} catch (...) {
				LOGERR("catch block : parameters[\"timeZone\"]...");
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
		returnResponse(resp);
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
                LOGERR("File not found %s.\n", TZ_FILE);
                populateResponseWithError(SysSrv_FileAccessFailed, response);
                resp = false;
            }
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
		if (parameters.HasLabel("key")) {
			std::string key = parameters["key"].String();
			if (key.length()) {
				if (m_cacheService.contains(key)) {
					retStat = true;
				} else {
					LOGERR("Accessing m_cacheService.contains failed\n.");
					populateResponseWithError(SysSrv_Unexpected, response);
				}
			} else {
				populateResponseWithError(SysSrv_UnSupportedFormat, response);
			}
		} else {
			populateResponseWithError(SysSrv_MissingKeyValues, response);
		}
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
            if (!regex_search(rebootInfo, match, regex("(?:PreviousRebootReason:\\s*)(\\d{2}\\.\\d{2}\\.\\d{4}_\\d{2}:\\d{2}\\.\\d{2})(?:\\s*RebootReason:)([^\\n]*)"))
                    || match.size() < 3) {
                LOGERR("%s doesn't have timestamp with reboot reason information", REBOOT_INFO_LOG_FILE);
                returnResponse(false);
            }

            string timeStamp = trim(match[1]);
            string reason = trim(match[2]);
            string source = "Unknown";
            string customReason = "Unknown";
            string otherReason = "Unknown";

            string temp;
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
		    if (args.HasLabel("WARN") && args.HasLabel("MAX")) {
		        string warn = args["WARN"].String();
		        string max = args["MAX"].String();

                high = atof(warn.c_str());
                critical = atof(max.c_str());

                resp =  CThermalMonitor::instance()->setCoreTempThresholds(high, critical);
                LOGWARN("Set temperature thresholds: WARN: %f, MAX: %f\n", high, critical);
            } else {
		        populateResponseWithError(SysSrv_MissingKeyValues, response);
            }
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
            char rebootInfo[1024] = {'\0'};
            char hardPowerInfo[1024] = {'\0'};

            if (Utils::fileExists(SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE)) {
                retAPIStatus = getFileContentToCharBuffer(
                        SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE, rebootInfo);
                if (retAPIStatus && strlen(rebootInfo)) {
                    string dataBuf(rebootInfo);
                    JsonObject rebootInfoJson;
                    rebootInfoJson.FromString(dataBuf);
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
                retAPIStatus = getFileContentToCharBuffer(
                        SYSTEM_SERVICE_HARD_POWER_INFO_FILE, hardPowerInfo);
                if (retAPIStatus && strlen(hardPowerInfo)) {
                    string dataBuf(hardPowerInfo);
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
            char rebootInfo[1024] = {'\0'};

            if (Utils::fileExists(SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE)) {
                retAPIStatus = getFileContentToCharBuffer(
                        SYSTEM_SERVICE_PREVIOUS_REBOOT_INFO_FILE, rebootInfo);
                if (retAPIStatus && strlen(rebootInfo)) {
                    string dataBuf(rebootInfo);
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
            const std::string baseCommand = "tr181Set -g ";
            const std::string redirection = " 2>&1";
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
                        cmdParams = baseCommand + jsonRFCList[i].String() + redirection + "\0";
                        LOGINFO("executing %s\n", cmdParams.c_str());
                        cmdResponse = Utils::cRunScript(cmdParams.c_str());
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
            vector<string> milestones;

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
            returnResponse(retAPIStatus);
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
#ifdef HAS_STATE_OBSERVER
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
#else /* !HAS_STATE_OBSERVER */
                populateResponseWithError(SysSrv_SupportNotAvailable, response);
#endif /* !HAS_STATE_OBSERVER */
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
            string powerState = CPowerState::instance()->getPowerState();

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
					retVal = CPowerState::instance()->setPowerState(sleepMode);
				} else {
					retVal = CPowerState::instance()->setPowerState(state);
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
				retVal = CPowerState::instance()->setPowerState(state);
				LOGERR("this platform has no API System and/or Powerstate\n");
			}
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
		bool result = false;

		result = isGzEnabledHelper(enabled);
		response["enabled"] = enabled;
		returnResponse(result);
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
                        std::string token = gp.substr((gp.find(delimiter)+1), 43);
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
            response["receiverVersion"] = getClientVersionString();
            response["stbTimestamp"]    = getStbTimestampString();
            status = true;
            returnResponse(status);
        }

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
                        std::string curState = (eventData->data.state.curState ==
                                IARM_BUS_PWRMGR_POWERSTATE_ON) ? "ON" : "STANDBY";
                        std::string newState = (eventData->data.state.newState ==
                                IARM_BUS_PWRMGR_POWERSTATE_ON) ? "ON" : "STANDBY";
                        LOGWARN("IARM Event triggered for PowerStateChange.\
                                Old State %s, New State: %s\n",
                                curState.c_str() , newState.c_str());
                        if (SystemServices::_instance) {
                            SystemServices::_instance->onSystemPowerStateChanged(newState);
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
        void _firmwareUpdateStateChanged(const char *owner, IARM_EventId_t eventId,
                void *data, size_t len)
        {
            LOGINFO("len = %d\n", len);
            switch (eventId) {
                case IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE:
                    {
                        int newState = IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_UNINITIALIZED;
                        IARM_Bus_SYSMgr_EventData_t *eventData = (IARM_Bus_SYSMgr_EventData_t *)data;
                        LOGWARN("IARM Event: [State/Error/Payload]=[%d/%d/%s]\n",
                                eventData->data.systemStates.state,
                                eventData->data.systemStates.error,
                                eventData->data.systemStates.payload);
                        if (SystemServices::_instance) {
                            SystemServices::_instance->onFirmwareUpdateStateChange(newState);
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
    } /* namespace Plugin */
} /* namespace WPEFramework */

