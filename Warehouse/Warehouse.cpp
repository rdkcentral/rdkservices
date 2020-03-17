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

#include "Warehouse.h"

#include <algorithm>
#include <fstream>

#include <regex.h>

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIBus.h"
#include "sysMgr.h"
#include "pwrMgr.h"
#endif

#include "utils.h"

#include "frontpanel.h"

#define WAREHOUSE_METHOD_RESET_DEVICE "resetDevice"
#define WAREHOUSE_METHOD_GET_DEVICE_INFO "getDeviceInfo"
#define WAREHOUSE_METHOD_GET_VERSION "getVersion"
#define WAREHOUSE_METHOD_SET_FRONT_PANEL_STATE "setFrontPanelState"
#define WAREHOUSE_METHOD_INTERNAL_RESET "internalReset"
#define WAREHOUSE_METHOD_LIGHT_RESET "lightReset"

#define WAREHOUSE_EVT_DEVICE_INFO_RETRIEVED "deviceInfoRetrieved"
#define WAREHOUSE_EVT_RESET_DONE "resetDone"

#define HOSTS_FILE "/etc/warehouseHosts.conf"
#define DEFAULT_CNAME_TAIL ".warehouse.ccp.xcal.tv"

#define PARAM_SUCCESS "success"
#define PARAM_ERROR "error"

#define DEVICE_INFO_SCRIPT "sh /lib/rdk/getDeviceDetails.sh read"
#define VERSION_FILE_NAME "/version.txt"

#define LIGHT_RESET_SCRIPT "rm -rf /opt/netflix/* SD_CARD_MOUNT_PATH/netflix/* XDG_DATA_HOME/* XDG_CACHE_HOME/* XDG_CACHE_HOME/../.sparkStorage/ /opt/QT/home/data/* /opt/hn_service_settings.conf /opt/apps/common/proxies.conf /opt/lib/bluetooth"
#define INTERNAL_RESET_SCRIPT "rm -rf /opt/drm /opt/www/whitebox /opt/www/authService && /rebootNow.sh -s WarehouseService &"

#define FRONT_PANEL_NONE -1
#define FRONT_PANEL_INPROGRESS 1
#define FRONT_PANEL_FAILED 3
#define FRONT_PANEL_INTERVAL 5000


namespace WPEFramework
{
    namespace Plugin
    {
#ifdef HAS_FRONT_PANEL
        static Core::TimerType<LedInfo> ledTimer(64 * 1024, "LedTimer");
#endif
        SERVICE_REGISTRATION(Warehouse, 1, 0);

        Warehouse* Warehouse::_instance = nullptr;

        Warehouse::Warehouse()
        : AbstractPlugin()
#ifdef HAS_FRONT_PANEL
        , m_ledTimer(this)
#endif
        {
            LOGINFO();
            Warehouse::_instance = this;

            registerMethod(WAREHOUSE_METHOD_RESET_DEVICE, &Warehouse::resetDeviceWrapper, this);
            registerMethod(WAREHOUSE_METHOD_GET_DEVICE_INFO, &Warehouse::getDeviceInfoWrapper, this);
            registerMethod(WAREHOUSE_METHOD_GET_VERSION, &Warehouse::getDeviceInfoWrapper, this);
            registerMethod(WAREHOUSE_METHOD_SET_FRONT_PANEL_STATE, &Warehouse::setFrontPanelStateWrapper, this);
            registerMethod(WAREHOUSE_METHOD_INTERNAL_RESET, &Warehouse::internalResetWrapper, this);
            registerMethod(WAREHOUSE_METHOD_LIGHT_RESET, &Warehouse::lightResetWrapper, this);
        }

        Warehouse::~Warehouse()
        {
            LOGINFO();
            Warehouse::_instance = nullptr;

            if (m_resetThread.joinable())
                m_resetThread.join();
        }

        /**
         * @brief This function is used to write the client name in the host file if
         * the string list is empty by default ".warehouse.ccp.xcal.tv" will write in
         * to the host file.
         *
         * @return Attribute of string value list.
         */
        std::vector<std::string> Warehouse::getAllowedCNameTails()
        {
            std::vector<std::string> cnameTails;
            std::ifstream file(HOSTS_FILE);

            LOGWARN("%s: attempt to open file '%s' for read mode with result %s", __FUNCTION__
                    , HOSTS_FILE, file.good() ? "sucess" : "failure");

            std::string line;

            while(std::getline(file, line))
            {
                cnameTails.push_back(line);
            }

            if (cnameTails.size() == 0)
                cnameTails.push_back(DEFAULT_CNAME_TAIL);

            return cnameTails;
        }

        /**
         * @brief Creates a new task for resetting the device.
         * An event will be dispatched after reset is done by notifying success code.
         * @ingroup SERVMGR_WAREHOUSE_API
         */
        void Warehouse::resetDevice()
        {
            resetDevice(false);
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        static void WareHouseResetIARM(Warehouse *wh, bool suppressReboot)
        {
            IARM_Bus_PWRMgr_WareHouseReset_Param_t whParam;
            whParam.suppressReboot = suppressReboot;
            IARM_Result_t err = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_WareHouseReset, &whParam, sizeof(whParam));

            bool ok = err == IARM_RESULT_SUCCESS;
            JsonObject params;

            params[PARAM_SUCCESS] = ok;

            if (!ok)
                params[PARAM_ERROR] = "Reset failed";

            string json;
            params.ToString(json);
            LOGINFO("Notify %s %s\n", WAREHOUSE_EVT_RESET_DONE, json.c_str());
            wh->Notify(WAREHOUSE_EVT_RESET_DONE, params);
        }

        static bool RunScriptIARM(const std::string& script, std::string& error)
        {
            IARM_Bus_SYSMgr_RunScript_t runScriptParam;
            runScriptParam.return_value = -1;
            size_t len = sizeof(runScriptParam.script_path)/sizeof(char);
            if(script.length() > (len - 1))
            {
                std::stringstream errorss;
                errorss << "Length of script greater than allowed limit of " << len << ".";
                error = errorss.str();

                LOGWARN("%s", error.c_str());
                return false;
            }

            strcpy(runScriptParam.script_path, script.c_str());
            IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_RunScript, &runScriptParam, sizeof(runScriptParam));
            bool ok = runScriptParam.return_value == 0;

            std::stringstream message;
            message << "script returned: " << runScriptParam.return_value;

            LOGINFO("%s", message.str().c_str());
            if (!ok)
                error = message.str();
            return ok;
        }
#endif

        void Warehouse::resetDevice(bool suppressReboot)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)

            LOGWARN("Received request to terminate CoPilot");

            if (m_resetThread.joinable())
                m_resetThread.join();

            m_resetThread = std::thread(WareHouseResetIARM, this, suppressReboot);
#else

            JsonObject params;

            bool ok = false;
            params[PARAM_SUCCESS] = ok;

            if (!ok)
                params[PARAM_ERROR] = "No IARMBUS";

            sendNotify(WAREHOUSE_EVT_RESET_DONE, params);
#endif
        }

        /**
         * @brief Returns a hash of properties for STB Device Information.
         * This includes values for the following keys (uses /lib/rdk/getDeviceDetails.sh):
         * Software version, IP and MAC of ecm, estb, moca etc.
         * @ingroup SERVMGR_WAREHOUSE_API
         */
        void Warehouse::getDeviceInfo(JsonObject &params)
        {
            char buf[256];

            FILE *p = popen(DEVICE_INFO_SCRIPT, "r");

            if(!p)
            {
                LOGWARN("failed to run %s:%s", DEVICE_INFO_SCRIPT, strerror(errno));
                return;
            }

            std::string res;
            while(fgets(buf, sizeof(buf), p) != NULL)
                res += buf;

            int errCode = pclose(p);

            if (0 != errCode)
            {
                params[PARAM_SUCCESS] = false;
                params[PARAM_ERROR] = (const char *)strerror(errno);
            }

            LOGINFO("'%s' returned: %s", DEVICE_INFO_SCRIPT, res.c_str());

            std::stringstream ss(res);
            std::string line;
            while(std::getline(ss, line))
            {
                size_t eq = line.find_first_of("=");

                if (std::string::npos != eq)
                {
                    std::string key = line.substr(0, eq);
                    std::string value = line.substr(eq + 1);

                    params[key.c_str()] = value;

                    // some tweaks for backward compatibility
                    if (key == "imageVersion")
                    {
                        params["version"] = value; 
                        params["software_version"] = value;
                    }
                    else if (key == "cableCardVersion")
                        params["cable_card_firmware_version"] = value;
                }
            }
        }

#ifdef HAS_FRONT_PANEL
        static bool SetFrontPanelLights(int state, int iteration)
        {
            bool ledData = false;
            bool ledRecord = false;
            int ledBrightness = 100;
            if (state == FRONT_PANEL_INPROGRESS)
            {
                // i    0   1   2   3   4   5   6   7   8
                // DA   1   1   1   1   0   0   0   0   1
                // RE   0   0   0   0   1   1   1   1   0
                // BR   100 75  50  25  0   25  50  75  100
                ledData = (iteration / 4) % 2 == 0;
                ledRecord = !ledData;
                ledBrightness = abs(100 - 25 * (iteration % 8));
            }
            else if (state == FRONT_PANEL_FAILED)
            {
                // i    0   1   2
                // DA   1   0   1
                // RE   0   1   0
                // BR   100 100 100
                ledData = iteration % 2 == 0;
                ledRecord = !ledData;
                ledBrightness = 100;
            }
            LOGINFO("SetFrontPanelLights set Brightness=%d (LEDs: Data=%d Record=%d)",
                       ledBrightness, ledData?1:0, ledRecord?1:0);
            CFrontPanel* helper = CFrontPanel::instance();
            bool didSet = false;
            if (ledData)
                didSet |= helper->powerOnLed(FRONT_PANEL_INDICATOR_MESSAGE);
            else
                didSet |= helper->powerOffLed(FRONT_PANEL_INDICATOR_MESSAGE);
            if (ledRecord)
                didSet |= helper->powerOnLed(FRONT_PANEL_INDICATOR_RECORD);
            else
                didSet |= helper->powerOffLed(FRONT_PANEL_INDICATOR_RECORD);
            didSet |= helper->setBrightness(ledBrightness);
            return didSet;
        }
#endif

        /**
         * @brief set the state of the front panel LEDs to indicate the download state of the STB
         * software image. Possible state values: NONE, DOWNLOAD IN PROGRESS, DOWNLOAD FAILED.
         *
         * @param[in] state Integer value of front panel state to set.
         *
         * @return Returns true when all LED states set successfully, else returns false.
         * @ingroup SERVMGR_WAREHOUSE_API
         */
        void Warehouse::setFrontPanelState(int state, JsonObject& response)
        {
#ifdef HAS_FRONT_PANEL
            if (state != FRONT_PANEL_NONE &&
                state != FRONT_PANEL_INPROGRESS &&
                state != FRONT_PANEL_FAILED)
            {
                LOGERR("FrontPanelState incorrect state %d", state);
                response[PARAM_SUCCESS] = false;
                response[PARAM_ERROR] = "incorrect state";
            }
            else
            {
                ledTimer.Revoke(m_ledTimer);
                bool didSet = SetFrontPanelLights(state, 0);
                LOGINFO("FrontPanelState %s to %d", didSet ? "set" : "not set", state);
                response[PARAM_SUCCESS] = didSet;
                if (!didSet)
                {
                    response[PARAM_ERROR] = "front panel not set";
                }
                else if (state != FRONT_PANEL_NONE)
                {
                    LOGINFO("Triggering FrontPanel update by timer");
                    m_ledTimerIteration = 1;
                    m_ledState = state; 
                    ledTimer.Schedule(Core::Time::Now().Add(FRONT_PANEL_INTERVAL), m_ledTimer);
                }
            }
#else
            LOGERR("FrontPanel unsupported");
            response[PARAM_SUCCESS] = false;
            response[PARAM_ERROR] = "unsupported";
#endif
        }

        void Warehouse::internalReset(JsonObject& response)
        {
            bool isProd = false;

            if (0 == access(VERSION_FILE_NAME, R_OK))
            {
                std::ifstream file(VERSION_FILE_NAME);

                LOGWARN("%s: attempt to open file '%s' for read mode with result %s", __FUNCTION__
                        , VERSION_FILE_NAME, file.good() ? "sucess" : "failure");

                std::string line;
                while(std::getline(file, line))
                {
                    if (line.find("imagename:") != std::string::npos)
                    {
                        isProd = line.find("PROD") != std::string::npos;
                        break;
                    }
                }
            }

            if (isProd)
            {
                response[PARAM_SUCCESS] = false;
                response[PARAM_ERROR] = "version is PROD";
            }
            else
            {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                std::string error;
                bool ok = RunScriptIARM(INTERNAL_RESET_SCRIPT, error);
                response[PARAM_SUCCESS] = ok;
                if (!ok)
                    response[PARAM_ERROR] = error;
#else
                response[PARAM_SUCCESS] = false;
                response[PARAM_ERROR] = "No IARMBUS";
#endif
            }
        }

        void Warehouse::lightReset(JsonObject& response)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)

            std::string script(LIGHT_RESET_SCRIPT);
            regex_t rx;
            regcomp(&rx, "(\\s+)([A-Z_][0-9A-Z_]*)(\\S*)", REG_EXTENDED);
            regmatch_t rm[4];

            int pos = 0;
            while (regexec(&rx, script.c_str() + pos, 4, rm, 0) == REG_NOERROR)
            {
                std::string var = script.substr(pos + rm[2].rm_so, rm[2].rm_eo - rm[2].rm_so);
                std::string replace;
                const char *envVar = getenv(var.c_str());

                std::string scmp;
                if ("SD_CARD_MOUNT_PATH" == var && (!envVar || 0 == *envVar))
                {

                    FILE *p = popen("cat  /proc/mounts | grep mmcblk0p1 | awk '{print $2}' ", "r");

                    if(!p)
                    {
                        LOGWARN("failed to run script to get SD_CARD_MOUNT_PATH");
                    }
                    else
                    {
                        char buf[256];
                        while(fgets(buf, sizeof(buf), p) != NULL)
                            scmp += buf;

                        scmp.erase(0, scmp.find_first_not_of(" \n\r\t"));
                        scmp.erase(scmp.find_last_not_of(" \n\r\t") + 1);

                        envVar = scmp.c_str();
                        pclose(p);
                    }
                }

                if (envVar && *envVar)
                {
                    replace += envVar;
                    replace += script.substr(pos + rm[3].rm_so, rm[3].rm_eo - rm[3].rm_so);
                    if (replace.size() > 0)
                    {
                        if ('/' == replace[0])
                        {
                            size_t nr = replace.find_first_not_of('/');
                            if (string::npos != nr && '*' != replace[nr]) // Check if that is not root dir
                                replace = script.substr(pos + rm[1].rm_so, rm[1].rm_eo - rm[1].rm_so) + replace;
                            else 
                                replace = "";
                        }
                        else
                            replace = script.substr(pos + rm[1].rm_so, rm[1].rm_eo - rm[1].rm_so) + replace;
                    }
                }
                script.replace(pos + rm[0].rm_so, rm[0].rm_eo - rm[0].rm_so, replace);
                pos += rm[0].rm_so + replace.size();
            }

            LOGWARN("lightReset: %s", script.c_str());

            std::string error;
            bool ok = RunScriptIARM(script, error);
            response[PARAM_SUCCESS] = ok;
            if (ok)
            {
                LOGWARN("lightReset succeeded");
            }
            else
            {
                LOGERR("lightReset failed. %s", error.c_str());
                response[PARAM_ERROR] = error;
            }
#else
            LOGERR("lightReset failed: No IARMBUS");
            response[PARAM_SUCCESS] = false;
            response[PARAM_ERROR] = "No IARMBUS";
#endif
        }

        uint32_t Warehouse::resetDeviceWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            bool suppressReboot = false;
            if (parameters.HasLabel("suppressReboot"))
            {
                getBoolParameter("suppressReboot", suppressReboot);
            }

            resetDevice(suppressReboot);

            response["PARAM_SUCCESS"] = true;
            returnResponse(true);
        }

        uint32_t Warehouse::getDeviceInfoWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            JsonObject deviceInfo;
            getDeviceInfo(deviceInfo);

            response["deviceInfo"] = deviceInfo;
            response["PARAM_SUCCESS"] = true;
            returnResponse(true);
        }

        uint32_t Warehouse::setFrontPanelStateWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            int state = 0;
            if (parameters.HasLabel("state"))
                getNumberParameter("state", state);

            setFrontPanelState(state, response);
            return Core::ERROR_NONE;
        }

        uint32_t Warehouse::internalResetWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            if (parameters.HasLabel("passPhrase") && parameters["passPhrase"].String() == "FOR TEST PURPOSES ONLY")
            {
                internalReset(response);
                return (Core::ERROR_NONE); 
            }
            else
            {
                response[PARAM_SUCCESS] = false;
                response[PARAM_ERROR] = "incorrect pass phrase";
                returnResponse(false);
            }
        }

        uint32_t Warehouse::lightResetWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFO();

            lightReset(response);
            return Core::ERROR_NONE;
        }


#ifdef HAS_FRONT_PANEL
        void Warehouse::onSetFrontPanelStateTimer()
        {
            SetFrontPanelLights(m_ledState, m_ledTimerIteration);
            ++m_ledTimerIteration;
            ledTimer.Schedule(Core::Time::Now().Add(FRONT_PANEL_INTERVAL), m_ledTimer);
        }

        uint64_t LedInfo::Timed(const uint64_t scheduledTime)
        {
            LOGINFO();
            uint64_t result = 0;
            m_warehouse->onSetFrontPanelStateTimer();
            return(result);
        }
#endif


    } // namespace Plugin
} // namespace WPEFramework
