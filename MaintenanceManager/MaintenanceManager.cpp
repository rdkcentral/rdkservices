/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2021 RDK Management
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
 * @file  MaintenanceManager.cpp
 * @author Livin Sunny
 * @brief Thunder Plugin based Implementation for RDK MaintenanceManager service API's.
 * @reference RDK-29959.
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
#include <array>
#include <unistd.h>

#include "MaintenanceManager.h"
#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilscRunScript.h"
#include "UtilsfileExists.h"

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#endif

#ifdef ENABLE_DEVICE_MANUFACTURER_INFO
#include "mfrMgr.h"
#endif

#ifdef ENABLE_DEEP_SLEEP
#include "deepSleepMgr.h"
#endif

#include "maintenanceMGR.h"

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 42
#define SERVER_DETAILS  "127.0.0.1:9998"

#define PROC_DIR "/proc"
#define RDK_PATH "/lib/rdk/"

#define MAINTENANCE_MANAGER_RFC_CALLER_ID "MaintenanceManager"
#define TR181_AUTOREBOOT_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"
#define TR181_STOP_MAINTENANCE  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.StopMaintenance.Enable"
#define TR181_RDKVFWUPGRADER  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKFirmwareUpgrader.Enable"

#if defined(ENABLE_WHOAMI)
#define TR181_PARTNER_ID "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.PartnerName"
#define TR181_TARGET_OS_CLASS "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.OsClass"
#define TR181_XCONFURL "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Bootstrap.XconfUrl"
#endif

#define RFC_TASK RDK_PATH "Start_RFC.sh"
#define SWUPDATE_TASK RDK_PATH "swupdate_utility.sh"
#define LOGUPLOAD_TASK RDK_PATH "Start_uploadSTBLogs.sh"


/**
 * @brief Converts a maintenance status enum to its corresponding string representation.
 *
 * This function maps each `Maint_notify_status_t` enumeration value to a specific
 * string for easy readability and logging purposes.
 *
 * @param status The maintenance status to convert, one of the values from the `Maint_notify_status_t` enum.
 * @return A string that represents the given maintenance status.
 *         - "MAINTENANCE_IDLE" for MAINTENANCE_IDLE
 *         - "MAINTENANCE_STARTED" for MAINTENANCE_STARTED
 *         - "MAINTENANCE_ERROR" for MAINTENANCE_ERROR
 *         - "MAINTENANCE_COMPLETE" for MAINTENANCE_COMPLETE
 *         - "MAINTENANCE_INCOMPLETE" for MAINTENANCE_INCOMPLETE
 *         - "MAINTENANCE_ERROR" for any unknown status
 */
string notifyStatusToString(Maint_notify_status_t &status)
{
    string ret_status="";
    switch(status)
    {
        case MAINTENANCE_IDLE:
            ret_status="MAINTENANCE_IDLE";
            break;
        case MAINTENANCE_STARTED:
            ret_status="MAINTENANCE_STARTED";
            break;
        case MAINTENANCE_ERROR:
            ret_status="MAINTENANCE_ERROR";
            break;
        case MAINTENANCE_COMPLETE:
            ret_status="MAINTENANCE_COMPLETE";
            break;
        case MAINTENANCE_INCOMPLETE:
            ret_status="MAINTENANCE_INCOMPLETE";
            break;
        default:
            ret_status="MAINTENANCE_ERROR";
    }
    return ret_status;
}

/**
 * @brief Checks if a given Opt-out mode is valid.
 *
 * This function verifies if the provided Opt-out mode exists within the predefined list
 * of valid modes.
 *
 * @param OptoutModes The Opt-out mode to check.
 * @return true if the Opt-out mode is valid, false otherwise.
 */
bool checkValidOptOutModes(string OptoutModes)
{
    vector<string> modes{
        "ENFORCE_OPTOUT",
        "BYPASS_OPTOUT",
        "IGNORE_UPDATE",
        "NONE"
    };

    return ( find( modes.begin(), modes.end(), OptoutModes) != modes.end() ) ? true : false;
}

/**
 * @brief Converts a module status enum to its corresponding string representation.
 *
 * This function maps each `IARM_Maint_module_status_t` enumeration value to a specific
 * string for easy readability and logging purposes.
 *
 * @param status The module status to convert, one of the values from the `IARM_Maint_module_status_t` enum.
 * @return A string that represents the given module status:
 *         - "MAINTENANCE_RFC_COMPLETE" for MAINT_RFC_COMPLETE
 *         - "MAINTENANCE_RFC_ERROR" for MAINT_RFC_ERROR
 *         - "MAINTENANCE_LOGUPLOAD_COMPLETE" for MAINT_LOGUPLOAD_COMPLETE
 *         - "MAINTENANCE_LOGUPLOAD_ERROR" for MAINT_LOGUPLOAD_ERROR
 *         - "MAINTENANCE_FWDOWNLOAD_COMPLETE" for MAINT_FWDOWNLOAD_COMPLETE
 *         - "MAINTENANCE_FWDOWNLOAD_ERROR" for MAINT_FWDOWNLOAD_ERROR
 *         - "MAINTENANCE_REBOOT_REQUIRED" for MAINT_REBOOT_REQUIRED
 *         - "MAINTENANCE_FWDOWNLOAD_ABORTED" for MAINT_FWDOWNLOAD_ABORTED
 *         - "MAINTENANCE_CRITICAL_UPDATE" for MAINT_CRITICAL_UPDATE
 *         - "MAINTENANCE_EMPTY" for any unknown status
 */
string moduleStatusToString(IARM_Maint_module_status_t &status)
{
    string ret_status="";
    switch(status)
    {
        case MAINT_RFC_COMPLETE:
            ret_status="MAINTENANCE_RFC_COMPLETE";
            break;
        case MAINT_RFC_ERROR:
            ret_status="MAINTENANCE_RFC_ERROR";
            break;
        case MAINT_LOGUPLOAD_COMPLETE:
            ret_status="MAINTENANCE_LOGUPLOAD_COMPLETE";
            break;
        case MAINT_LOGUPLOAD_ERROR:
            ret_status="MAINTENANCE_LOGUPLOAD_ERROR";
            break;
        case MAINT_FWDOWNLOAD_COMPLETE:
            ret_status="MAINTENANCE_FWDOWNLOAD_COMPLETE";
            break;
        case MAINT_FWDOWNLOAD_ERROR:
            ret_status="MAINTENANCE_FWDOWNLOAD_ERROR";
            break;
        case MAINT_REBOOT_REQUIRED:
            ret_status="MAINTENANCE_REBOOT_REQUIRED";
            break;
        case MAINT_FWDOWNLOAD_ABORTED:
            ret_status="MAINTENANCE_FWDOWNLOAD_ABORTED";
            break;
        case MAINT_CRITICAL_UPDATE:
            ret_status="MAINTENANCE_CRITICAL_UPDATE";
            break;
        default:
            ret_status="MAINTENANCE_EMPTY";
    }
    return ret_status;
}

/**
 * @brief WPEFramework class for Maintenance Manager
 */
namespace WPEFramework 
{
    namespace 
    {
        static Plugin::Metadata<Plugin::MaintenanceManager> metadata(
            /* Version (Major, Minor, Patch) */
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            /* Preconditions */
            {},
            /* Terminations */
            {},
            /* Controls */
            {}
        );
    } /* end of namespace */

    namespace Plugin 
    {
        namespace 
        {
            /**
             * @brief Retrieves the state of a specified service.
             *
             * This function queries the state of a service identified by its callsign
             * through the provided PluginHost::IShell interface.
             *
             * @param shell The PluginHost::IShell instance used to query the service.
             * @param callsign The identifier for the service to query.
             * @param state The output parameter to store the state of the service.
             * @return uint32_t indicating the result of the query operation:
             *         - Core::ERROR_NONE if the service was found and its state retrieved.
             *         - Core::ERROR_UNAVAILABLE if the service was not found.
             */
            uint32_t getServiceState(PluginHost::IShell *shell, const string &callsign, PluginHost::IShell::state &state) /* MaintenanceManager uses interfaces */
            {
                uint32_t result;
                auto interface = shell->QueryInterfaceByCallsign<PluginHost::IShell>(callsign);
                if (interface == nullptr)
                {
                    result = Core::ERROR_UNAVAILABLE;
                    std::cout << "no IShell for " << callsign << std::endl;
                }
                else
                {
                    result = Core::ERROR_NONE;
                    state = interface->State();
                    std::cout << "IShell state " << state << " for " << callsign << std::endl;
                    interface->Release();
                }
                return result;
            } /* end of getServiceState */
        } /* end of namespace*/

        /* Prototypes */
        SERVICE_REGISTRATION(MaintenanceManager, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
        /* Global time variable */
        MaintenanceManager* MaintenanceManager::_instance = nullptr;

        cSettings MaintenanceManager::m_setting(MAINTENANCE_MGR_RECORD_FILE);

        /* Static Member Definitions */
        timer_t MaintenanceManager::timerid;
        string MaintenanceManager::currentTask;
        bool MaintenanceManager::g_task_timerCreated = false;

        string task_names_foreground[] = {
            "/lib/rdk/Start_RFC.sh",
            "/lib/rdk/swupdate_utility.sh",
            "/lib/rdk/Start_uploadSTBLogs.sh"
        };

        vector<string> tasks;

        const int task_complete_status[] = {
            RFC_COMPLETE,
            SWUPDATE_COMPLETE,
            LOGUPLOAD_COMPLETE
        };

        std::map<string, int> task_status_map = {
            {RFC_TASK, RFC_COMPLETE},
            {SWUPDATE_TASK, SWUPDATE_COMPLETE},
            {LOGUPLOAD_TASK, LOGUPLOAD_COMPLETE}
        };

        string task_names[] = {
            "RFCbase.sh",
            "swupdate_utility.sh",
            "uploadSTBLogs.sh"
        };

        static const array<string, 3> kDeviceInitContextKeyVals = {
            "partnerId",
            "osClass",
            "regionalConfigService"
        };

        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager():PluginHost::JSONRPC()
        {
            MaintenanceManager::_instance = this;
            if (Utils::directoryExists(MAINTENANCE_MGR_RECORD_FILE))
            {
                LOGINFO("File %s detected as folder, deleting..", MAINTENANCE_MGR_RECORD_FILE);
                if (rmdir(MAINTENANCE_MGR_RECORD_FILE) == 0)
                {
                    cSettings mtemp(MAINTENANCE_MGR_RECORD_FILE);
                    MaintenanceManager::m_setting = mtemp;
                }
                else
                {
                     LOGINFO("Unable to delete folder: %s", MAINTENANCE_MGR_RECORD_FILE);
                }
            }

            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            Register("sampleMaintenanceManagerAPI", &MaintenanceManager::sampleAPI, this);
#endif

            Register("getMaintenanceActivityStatus", &MaintenanceManager::getMaintenanceActivityStatus,this);
            Register("getMaintenanceStartTime", &MaintenanceManager::getMaintenanceStartTime,this);
            Register("setMaintenanceMode", &MaintenanceManager::setMaintenanceMode,this);
            Register("startMaintenance", &MaintenanceManager::startMaintenance,this);
            Register("stopMaintenance", &MaintenanceManager::stopMaintenance,this);
            Register("getMaintenanceMode", &MaintenanceManager::getMaintenanceMode,this);

            MaintenanceManager::m_task_map[task_names_foreground[0].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[1].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[2].c_str()]=false;

#if defined(ENABLE_WHOAMI)
            MaintenanceManager::m_param_map[kDeviceInitContextKeyVals[0].c_str()] = TR181_PARTNER_ID;
            MaintenanceManager::m_param_map[kDeviceInitContextKeyVals[1].c_str()] = TR181_TARGET_OS_CLASS;
            MaintenanceManager::m_param_map[kDeviceInitContextKeyVals[2].c_str()] = TR181_XCONFURL;

            MaintenanceManager::m_paramType_map[kDeviceInitContextKeyVals[0].c_str()] = DATA_TYPE::WDMP_STRING;
            MaintenanceManager::m_paramType_map[kDeviceInitContextKeyVals[1].c_str()] = DATA_TYPE::WDMP_STRING;
            MaintenanceManager::m_paramType_map[kDeviceInitContextKeyVals[2].c_str()] = DATA_TYPE::WDMP_STRING;
#endif /* end of ENABLE_WHOAMI */
        }

        void MaintenanceManager::task_execution_thread()
        {
            int i = 0;
            string task = "";
            bool internetConnectStatus = false;
            bool delayMaintenanceStarted = false;
            bool exitOnNoNetwork = false;
            int retry_count = TASK_RETRY_COUNT;
            bool isTaskTimerStarted = false;

            std::unique_lock<std::mutex> wailck(m_waiMutex);
            LOGINFO("Executing Maintenance tasks");

#if defined(ENABLE_WHOAMI)
            /* Purposefully delaying MAINTENANCE_STARTED status to honor POWER compliance */
            if (UNSOLICITED_MAINTENANCE == g_maintenance_type)
            {
                delayMaintenanceStarted = true;
            }
#endif

            if (!delayMaintenanceStarted)
            {
                m_statusMutex.lock();
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
                m_statusMutex.unlock();
            }

            /* cleanup tasks vector, if not empty */
            if (!tasks.empty())
            {
                tasks.erase(tasks.begin(), tasks.end());
            }

#if defined(SUPPRESS_MAINTENANCE) && !defined(ENABLE_WHOAMI)
            bool activationStatus = false;
            bool skipFirmwareCheck = false;
            activationStatus = getActivatedStatus(skipFirmwareCheck); /* Activation Check */

            /* we proceed with network check only if activationStatus is
             * "activation-connect",
             * "activation-ready",
             * "not-activated",
             * "activated" */
            if (activationStatus)
            {
                internetConnectStatus = isDeviceOnline(); /* Network Check */
            }
#else
            internetConnectStatus = isDeviceOnline(); /* Network Check */
#endif

#if defined(ENABLE_WHOAMI)
            if (UNSOLICITED_MAINTENANCE == g_maintenance_type) /* Unsolicited Maintenance in WHOAMI */
            {
                string activation_status = checkActivatedStatus(); /* Device Activation Status Check */
                bool whoAmIStatus = knowWhoAmI(activation_status); /* WhoAmI Response & Set Status Check */
                LOGINFO("knowWhoAmI() returned %s", (whoAmIStatus) ? "successfully" : "false");

                if (!whoAmIStatus && activation_status != "activated")
                {
                    LOGINFO("knowWhoAmI() returned false and Device is not already Activated");
                    g_listen_to_deviceContextUpdate = true;
                    LOGINFO("Waiting for onDeviceInitializationContextUpdate event");
                    task_thread.wait(wailck);
                }
                else if (!internetConnectStatus && activation_status == "activated")
                {
                    LOGINFO("Device is not connected to the Internet and Device is already Activated");
                    exitOnNoNetwork = true;
                }
            }
            else /* Solicited Maintenance in WHOAMI */
            {
                if (!internetConnectStatus)
                    exitOnNoNetwork = true;
            }
#else
            if (!internetConnectStatus)
            {
                exitOnNoNetwork = true;
            }
#endif
            if (exitOnNoNetwork) /* Exit Maintenance Cycle if no Internet */
            {
                m_statusMutex.lock();
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
                m_statusMutex.unlock();
                LOGINFO("Maintenance is exiting as device is not connected to internet.");
                if (UNSOLICITED_MAINTENANCE == g_maintenance_type && !g_unsolicited_complete)
                {
                    g_unsolicited_complete = true;
                    g_listen_to_nwevents = true;
                }
                return;
            }

            if (delayMaintenanceStarted)
            {
                m_statusMutex.lock();
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
                m_statusMutex.unlock();
            }

            LOGINFO("Reboot_Pending :%s", g_is_reboot_pending.c_str());
            LOGINFO("%s", UNSOLICITED_MAINTENANCE == g_maintenance_type ? "---------------UNSOLICITED_MAINTENANCE--------------" : "=============SOLICITED_MAINTENANCE===============");

#if defined(SUPPRESS_MAINTENANCE) && !defined(ENABLE_WHOAMI)
            if (skipFirmwareCheck)
            {
                /* set the task status of Firmware Download */
                SET_STATUS(g_task_status, SWUPDATE_SUCCESS);
                SET_STATUS(g_task_status, SWUPDATE_COMPLETE);
                /* Skip Firmware Download Task and add other tasks */
                tasks.push_back(task_names_foreground[0].c_str());
                tasks.push_back(task_names_foreground[2].c_str());
            }
            else
            {
                tasks.push_back(task_names_foreground[0].c_str());
                tasks.push_back(task_names_foreground[1].c_str());
                tasks.push_back(task_names_foreground[2].c_str());
            }
#else
            tasks.push_back(task_names_foreground[0].c_str());
            tasks.push_back(task_names_foreground[1].c_str());
            tasks.push_back(task_names_foreground[2].c_str());
#endif
            std::unique_lock<std::mutex> lck(m_callMutex);
            for (i = 0; i < static_cast<int>(tasks.size()) && !m_abort_flag; i++)
            {
                int task_status = -1;
                task = tasks[i];
                currentTask = task;
                task += " &";
                task += "\0";

                if (!m_abort_flag)
                {
                    if (retry_count == TASK_RETRY_COUNT)
                    {
                        LOGINFO("Starting Timer for %s \n", currentTask.c_str());
                        isTaskTimerStarted = task_startTimer();
                    }
                    if (isTaskTimerStarted)
                    {
                        m_task_map[tasks[i]] = true;
                        LOGINFO("Starting Task :  %s \n", task.c_str());
                        task_status = system(task.c_str());
                    }
                    /* Set task_status purposefully to non-zero value to verify failure logic*/
                    // task_status = -1;
                    if (task_status != 0) /* system() call fails */
                    {
                        m_task_map[tasks[i]] = false;
                        LOGINFO("%s invocation failed with return status %d", tasks[i].c_str(), WEXITSTATUS(task_status));
                        if (retry_count > 0 && isTaskTimerStarted)
                        {
                            LOGINFO("Retry %s after %d seconds (%d retry left)\n", tasks[i].c_str(), TASK_RETRY_DELAY, retry_count);
                            sleep(TASK_RETRY_DELAY);
                            i--; /* Decrement iterator to retry same task again */
                            retry_count--;
                            continue;
                        }
                        else
                        {
                            LOGINFO("Task Failed");
                            auto it = task_status_map.find(tasks[i]);
                            if (it != task_status_map.end())
                            {
                                LOGINFO("Setting task as Error");
                                int complete_status = it->second;
                                SET_STATUS(g_task_status, complete_status);
                            }
                            if (task_stopTimer())
                            {
                                LOGINFO("Stopped Timer Successfully");
                            }
                            else
                            {
                                LOGERR("task_stopTimer() did not stop the Timer");
                            }
                        }
                    }
                    else /* System() executes successfully */
                    {
                        LOGINFO("Waiting to unlock.. [%d/%d]", i + 1, (int)tasks.size());
                        task_thread.wait(lck);
                        if (task_stopTimer())
                        {
                            LOGINFO("Stopped Timer Successfully");
                        }
                        else
                        {
                            LOGERR("task_stopTimer() did not stop the Timer");
                        }
                    }
                }
                retry_count = TASK_RETRY_COUNT; /* Reset Retry Count for next Task*/
            }
            if (m_abort_flag)
            {
                m_abort_flag = false;
                if (task_stopTimer())
                {
                    LOGINFO("Stopped Timer Successfully");
                }
                else
                {
                    LOGERR("task_stopTimer() did not stop the Timer");
                }
            }
            LOGINFO("Worker Thread Completed");
        } /* end of task_execution_thread() */

#if defined(ENABLE_WHOAMI)
        /**
         * @brief Determines the device identity by querying the Security Manager.
         *
         * This function repeatedly attempts to query the Security Manager (`org.rdk.SecManager`)
         * for the device initialization context until the Security Manager is activated or the device
         * is already activated. If the Security Manager provides a valid device initialization context,
         * it sets this context using the `setDeviceInitializationContext` method.
         *
         * @param activation_status A reference to a string containing the current activation status of the device.
         *                          This status is used to determine if retries are needed.
         * @return true if the device initialization context was successfully obtained and set, false otherwise.
         */
        bool MaintenanceManager::knowWhoAmI(string &activation_status)
        {
            bool success = false;
            const char *secMgr_callsign = "org.rdk.SecManager";
            const char *secMgr_callsign_ver = "org.rdk.SecManager.1";
            PluginHost::IShell::state state;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *thunder_client = nullptr;

            do
            {
                if ((getServiceState(m_service, secMgr_callsign, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    LOGINFO("%s is active", secMgr_callsign);
                    thunder_client = getThunderPluginHandle(secMgr_callsign_ver);
                    if (thunder_client != nullptr)
                    {
                        JsonObject params;
                        JsonObject joGetResult;
                        thunder_client->Invoke<JsonObject, JsonObject>(5000, "getDeviceInitializationContext", params, joGetResult);
                        if (joGetResult.HasLabel("success") && joGetResult["success"].Boolean())
                        {
                            static const char *kDeviceInitializationContext = "deviceInitializationContext";
                            if (joGetResult.HasLabel(kDeviceInitializationContext))
                            {
                                LOGINFO("%s found in the response", kDeviceInitializationContext);
                                success = setDeviceInitializationContext(joGetResult);
                            }
                            else
                            {
                                LOGERR("%s is not available in the response", kDeviceInitializationContext);
                            }
                        }
                        else
                        {
                            LOGERR("getDeviceInitializationContext failed");
                        }
                    }
                    else
                    {
                        LOGERR("Failed to get plugin handle");
                    }
                    if (!g_subscribed_for_deviceContextUpdate)
                    {
                        LOGINFO("onDeviceInitializationContextUpdate event not subscribed...");
                        g_subscribed_for_deviceContextUpdate = subscribeToDeviceInitializationEvent();
                    }
                    return success;
                }
                else
                {
                    g_subscribed_for_deviceContextUpdate = false;
                    if (activation_status != "activated")
                    {
                        LOGINFO("%s is not active. Retry after %d seconds", secMgr_callsign, SECMGR_RETRY_INTERVAL);
                        sleep(SECMGR_RETRY_INTERVAL);
                    }
                    else
                    {
                        LOGINFO("%s is not active. Device is already Activated. Hence exiting from knoWhoAmI()", secMgr_callsign);
                        return success;
                    }
                }
            } while (true);
        }
#endif /* end of ENABLE_WHOAMI */

        /**
         * @brief Retrieves a handle to the specified Thunder plugin with authentication.
         *
         * This function generates a security token using the SecurityAgent and sets up the
         * Thunder plugin handle for the specified plugin callsign.
         *
         * @param callsign The callsign of the Thunder plugin to retrieve.
         * @return A pointer to a JSONRPC link for the specified plugin.
         */
        WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *MaintenanceManager::getThunderPluginHandle(const char *callsign)
        {
            string token;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *thunder_client = nullptr;

            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr)
            {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t *>(payload.c_str()),
                        token) == Core::ERROR_NONE)
                {
                    LOGINFO("MaintenanceManager got security token");
                }
                else
                {
                    LOGERR("MaintenanceManager failed to get security token");
                }
                security->Release();
            }
            else
            {
                LOGERR("No security agent");
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            thunder_client = new WPEFramework::JSONRPC::LinkType<Core::JSON::IElement>(callsign, "", false, query);
            return thunder_client;
        }

        /**
         * @brief Creates a new task timer.
         *
         * This function initializes and creates a new task timer if it does not already exist or is not running.
         *
         * @return true if the timer was successfully created, false otherwise.
         */
        bool MaintenanceManager::maintenance_initTimer()
        {
            if (g_task_timerCreated)
            {
                LOGINFO("Timer has already been created, no need to create a Timer.");
                return g_task_timerCreated;
            }

            struct sigevent sev = {0};
            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = SIGALRM;
            sev.sigev_value.sival_ptr = &timerid;

            if (timer_create(BASE_CLOCK, &sev, &timerid) == -1)
            {
                LOGERR("timer_create() failed to create the Timer");
            }
            else{
                g_task_timerCreated = true; // Timer Created
                LOGINFO("Timer created successfully.");
            }
            return g_task_timerCreated;
        }

        /**
         * @brief Starts or restarts the task timer.
         *
         * This function starts or restarts the task timer with a predefined timeout.
         *
         * @return true if the timer was successfully started, false otherwise.
         */
        bool MaintenanceManager::task_startTimer()
        {
            bool status = false;
            if (g_task_timerCreated)
            {
                LOGINFO("Timer has already been created, start the Timer");
            }
            else
            {
                LOGINFO("Timer has not been created already, create a new Timer.");
                if (!maintenance_initTimer())
                {
                    return status;
                }
            }

            struct itimerspec its;
            its.it_value.tv_sec = TASK_TIMEOUT;
            its.it_value.tv_nsec = 0;
            its.it_interval.tv_sec = 0;
            its.it_interval.tv_nsec = 0;

            if (timer_settime(timerid, 0, &its, NULL) == -1)
            {
                LOGERR("timer_settime() failed to start the Timer");
            }
            else
            {
                LOGINFO("Timer started for %d seconds for %s", TASK_TIMEOUT, currentTask.c_str());
                status = true;
            }
            return status;
        }

        /**
         * @brief Stops the task timer.
         *
         * This function stops the task timer if it is currently running.
         *
         * @return true if the timer was successfully stopped, false otherwise.
         */
        bool MaintenanceManager::task_stopTimer()
        {
            bool status = false;
            if (!g_task_timerCreated)
            {
                LOGINFO("Timer has not been created already, cannot stop the Timer");
                return status;
            }

            struct itimerspec its = {0};
            its.it_value.tv_sec = 0;
            its.it_value.tv_nsec = 0;

            if (timer_settime(timerid, 0, &its, NULL) == -1)
            {
                LOGERR("timer_settime() failed to stop the Timer");
            }
            else
            {
                LOGINFO("Timer stopped for %s", currentTask.c_str());
                status = true;
            }
            return status;
        }

        /**
         * @brief Deletes the task timer.
         *
         * This function deletes the task timer, stopping it first if necessary.
         *
         * @return true if the timer was successfully deleted, false otherwise.
         */
        bool MaintenanceManager::maintenance_deleteTimer()
        {
            bool status = false;
            if (!g_task_timerCreated)
            {
                LOGINFO("Timer has not been created already, cannot delete the Timer.");
                return status;
            }

            LOGINFO("Timer has already been created, delete the Timer.");

            if (timer_delete(timerid) == -1)
            {
                LOGERR("timer_delete() failed to delete the Timer.");
            }
            else
            {
                g_task_timerCreated = false;
                LOGINFO("Timer successfully deleted.");
                status = true;
            }
            return status;
        }

        /**
         * @brief Handles the timer signal.
         *
         * This function is invoked when the task timer expires and processes the timeout accordingly.
         *
         * @param signo The signal number received.
         */
        void MaintenanceManager::timer_handler(int signo)
        {
            if (signo == SIGALRM)
            {
                LOGERR("Timeout reached for %s. Set task to Error...", currentTask.c_str());

                const char *failedTask = nullptr;
                int complete_status = 0;

                for (size_t j = 0; j < (sizeof(task_names_foreground) / sizeof(task_names_foreground[0])); j++)
                {
                    if (currentTask.find(task_names_foreground[j]) != string::npos)
                    {
                        failedTask = task_names_foreground[j].c_str();
                        complete_status = task_complete_status[j];
                        break;
                    }
                }

                if (failedTask && !MaintenanceManager::_instance->m_task_map[failedTask])
                {
                    LOGINFO("Ignoring Error Event for Task: %s", failedTask);
                }
                else if (failedTask)
                {
                    MaintenanceManager::_instance->m_task_map[failedTask] = false;
                    SET_STATUS(MaintenanceManager::_instance->g_task_status, complete_status);
                    MaintenanceManager::_instance->task_thread.notify_one();
                    LOGINFO("Set %s Task to ERROR", failedTask);
                }
            }
            else
            {
                LOGERR("Received %d Signal instead of SIGALRM", signo);
            }
        }

        /**
         * @brief Sets an RFC parameter.
         *
         * This function sets an RFC parameter with a specified value and data type.
         *
         * @param rfc The RFC parameter name.
         * @param value The value to set.
         * @param dataType The type of the value.
         * @return true if the parameter was successfully set, false otherwise.
         */
        bool MaintenanceManager::setRFC(const char *rfc, const char *value, DATA_TYPE dataType)
        {
            LOGINFO("Invoke setRFC...");
            bool result = false;
            WDMP_STATUS status;
            status = setRFCParameter((char *)MAINTENANCE_MANAGER_RFC_CALLER_ID, rfc, value, dataType);

            if (WDMP_SUCCESS == status)
            {
                LOGINFO("Successfuly set the tr181 parameter %s with value %s", rfc, value);
                result = true;
            }
            else
            {
                LOGINFO("Failed setting %s parameter", rfc);
            }
            return result;
        }

        /**
         * @brief Sets the partner ID using the AuthService.
         *
         * This function sets the partner ID by invoking the `setPartnerId` method of the AuthService.
         *
         * @param partnerid The partner ID to set.
         */
        void MaintenanceManager::setPartnerId(string partnerid)
        {
            LOGINFO("Invoke setPartnerId...");
            const char *authservice_callsign = "org.rdk.AuthService.1";
            PluginHost::IShell::state state;
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *thunder_client = nullptr;

            if ((getServiceState(m_service, "org.rdk.AuthService", state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
            {
                thunder_client = getThunderPluginHandle(authservice_callsign);

                if (thunder_client == nullptr)
                {
                    LOGERR("Failed to get plugin handle");
                }
                else
                {
                    JsonObject joGetParams;
                    JsonObject joGetResult;

                    joGetParams["partnerId"] = partnerid;

                    thunder_client->Invoke<JsonObject, JsonObject>(5000, "setPartnerId", joGetParams, joGetResult);
                    string responseJson;
                    joGetResult.ToString(responseJson);
                    LOGINFO("AuthService Response Data: %s", responseJson.c_str());
                    if (joGetResult.HasLabel("success") && joGetResult["success"].Boolean())
                    {
                        LOGINFO("Successfully set the partnerId via Authservice");
                    }
                    else
                    {
                        LOGERR("Failed to set the partnerId through Authservice");
                    }
                }
            }
        }

        /**
         * @brief Subscribes to an internet status event.
         *
         * This function subscribes to an event from the network plugin to listen for internet status changes.
         *
         * @param event The name of the event to subscribe to.
         * @return true if the subscription was successful, false otherwise.
         */
        bool MaintenanceManager::subscribeForInternetStatusEvent(string event)
        {
            int32_t status = Core::ERROR_NONE;
            bool result = false;
            LOGINFO("Attempting to subscribe for %s events", event.c_str());
            const char *network_callsign = "org.rdk.Network.1";
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *thunder_client = nullptr;

            thunder_client = getThunderPluginHandle(network_callsign);
            if (thunder_client == nullptr)
            {
                LOGERR("Failed to get plugin handle");
            }
            else
            {
                status = thunder_client->Subscribe<JsonObject>(5000, event, &MaintenanceManager::internetStatusChangeEventHandler, this);
                if (status == Core::ERROR_NONE)
                {
                    result = true;
                }
            }
            return result;
        }

        /**
         * @brief Handles the internet status change event.
         *
         * This function is called when the internet status changes and starts critical tasks if connected.
         *
         * @param parameters The event parameters.
         */
        void MaintenanceManager::internetStatusChangeEventHandler(const JsonObject &parameters)
        {
            string value;
            int state;

            if (parameters.HasLabel("status") && parameters.HasLabel("state"))
            {
                value = parameters["status"].String();
                state = parameters["state"].Number();

                LOGINFO("Received onInternetStatusChange event: [%s:%d]", value.c_str(), state);
                if (g_listen_to_nwevents)
                {
                    if (state == INTERNET_CONNECTED_STATE)
                    {
                        startCriticalTasks();
                        g_listen_to_nwevents = false;
                    }
                }
            }
        }

        /**
         * @brief Handles the device initialization context update event.
         *
         * This function is called when the device initialization context is updated and sets the context.
         *
         * @param parameters The event parameters.
         */
        void MaintenanceManager::deviceInitializationContextEventHandler(const JsonObject &parameters)
        {
            bool contextSet = false;
            if (g_listen_to_deviceContextUpdate && UNSOLICITED_MAINTENANCE == g_maintenance_type)
            {
                LOGINFO("onDeviceInitializationContextUpdate event is already subscribed and Maintenance Type is Unsolicited Maintenance");
                if (parameters.HasLabel("deviceInitializationContext"))
                {
                    LOGINFO("deviceInitializationContext found");

                    contextSet = setDeviceInitializationContext(parameters);
                    if (contextSet)
                    {
                        LOGINFO("setDeviceInitializationContext() success");
                        g_listen_to_deviceContextUpdate = false;
                        LOGINFO("Notify maintenance execution thread");
                        task_thread.notify_one();
                    }
                    else
                    {
                        LOGINFO("setDeviceInitializationContext() failed");
                    }
                }
                else
                {
                    LOGINFO("deviceInitializationContext not found");
                }
            }
            else
            {
                LOGINFO("onDeviceInitializationContextUpdate event is not being listened already or Maintenance Type is not Unsolicited Maintenance");
            }
        }

        /**
         * @brief Starts critical maintenance tasks.
         *
         * This function invokes Tasks to start critical maintenance tasks.
         */
        void MaintenanceManager::startCriticalTasks()
        {
            LOGINFO("Starting Critical Tasks...");
            int rfc_task_status = -1;
            int xconf_imagecheck_status = -1;

            LOGINFO("Starting Script /lib/rdk/Start_RFC.sh");
            rfc_task_status = system("/lib/rdk/Start_RFC.sh &");
            if (rfc_task_status != 0)
            {
                LOGINFO("Failed to run Start_RFC.sh with %d", WEXITSTATUS(rfc_task_status));
            }

            LOGINFO("Starting Script /lib/rdk/xconfImageCheck.sh");
            xconf_imagecheck_status = system("/lib/rdk/xconfImageCheck.sh &");
            if (xconf_imagecheck_status != 0)
            {
                LOGINFO("Failed to run xconfImageCheck.sh with %d", WEXITSTATUS(xconf_imagecheck_status));
            }
        }

        /**
         * @brief Checks the activation status of the device.
         *
         * This function queries the AuthService to check if the device is activated.
         *
         * @return A string representing the activation status.
         */
        const string MaintenanceManager::checkActivatedStatus()
        {
            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.AuthService.1";
            uint8_t i = 0;
            std::string ret_status("invalid");

            /* check if plugin active */
            PluginHost::IShell::state state = PluginHost::IShell::state::UNAVAILABLE;
            if ((getServiceState(m_service, "org.rdk.AuthService", state) != Core::ERROR_NONE) || (state != PluginHost::IShell::state::ACTIVATED))
            {
                LOGERR("AuthService plugin is not activated.Retrying.. \n");
                // if plugin is not activated we need to retry
                do
                {
                    if ((getServiceState(m_service, "org.rdk.AuthService", state) != Core::ERROR_NONE) || (state != PluginHost::IShell::state::ACTIVATED))
                    {
                        sleep(10);
                        i++;
                        LOGINFO("AuthService retries [%d/4] \n", i);
                    }
                    else
                    {
                        break;
                    }
                } while (i < MAX_ACTIVATION_RETRIES);

                if (state != PluginHost::IShell::state::ACTIVATED)
                {
                    LOGERR("AuthService plugin is Still not active");
                    return ret_status;
                }
                else
                {
                    LOGINFO("AuthService plugin is Now active");
                }
            }
            if (state == PluginHost::IShell::state::ACTIVATED)
            {
                LOGINFO("AuthService is active");
            }

            string token;

            // TODO: use interfaces and remove token
            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr)
            {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t *>(payload.c_str()),
                        token) == Core::ERROR_NONE)
                {
                    LOGINFO("MaintenanceManager got security token");
                }
                else
                {
                    LOGERR("MaintenanceManager failed to get security token");
                }
                security->Release();
            }
            else
            {
                LOGERR("No security agent");
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto thunder_client = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>(callsign.c_str(), "", false, query);
            if (thunder_client != nullptr)
            {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "getActivationStatus", joGetParams, joGetResult);
                LOGINFO("Invoke status : %d", status);
                if (status > 0)
                {
                    LOGERR("%s call failed %d", callsign.c_str(), status);
                    ret_status = "invalid";
                    LOGINFO("Setting Default to [%s]", ret_status.c_str());
                }
                else if (joGetResult.HasLabel("status"))
                {
                    ret_status = joGetResult["status"].String();
                    LOGINFO("Activation Value [%s]", ret_status.c_str());
                }
                else
                {
                    LOGERR("Failed to read the ActivationStatus");
                    ret_status = "invalid";
                }

                return ret_status;
            }

            LOGERR("thunder client failed");
            return ret_status;
        }

        /**
         * @brief Gets the activation status of the device.
         *
         * This function checks the activation status and determines whether to skip firmware checks.
         *
         * @param skipFirmwareCheck A reference to a boolean indicating if firmware checks should be skipped.
         * @return true if the device is activated or in a state to proceed, false otherwise.
         */
        bool MaintenanceManager::getActivatedStatus(bool &skipFirmwareCheck)
        {
            /* activation-connect, activation ready, not-activated - execute all except DIFD
             * activation disconnect - dont run maintenance
             * activated - run normal */
            bool ret_result = false;
            string activationStatus;
            Auth_activation_status_t result;
            const std::unordered_map<std::string, std::function<void()>> act{
                {"activation-connect", [&]()
                 { result = ACTIVATION_CONNECT; }},
                {"activation-ready", [&]()
                 { result = ACTIVATION_READY; }},
                {"not-activated", [&]()
                 { result = NOT_ACTIVATED; }},
                {"activation-disconnect", [&]()
                 { result = ACTIVATION_DISCONNECT; }},
                {"activated", [&]()
                 { result = ACTIVATED; }},
            };

            activationStatus = checkActivatedStatus();
            LOGINFO("activation status : [ %s ]", activationStatus.c_str());
            const auto end = act.end();
            auto search = act.find(activationStatus);
            if (search != end)
            {
                search->second();
            }
            else
            {
                result = INVALID_ACTIVATION;
                LOGERR("result: invalid Activation");
            }

            switch (result)
            {
                case ACTIVATED:
                    ret_result = true;
                    break;
                case ACTIVATION_DISCONNECT:
                    ret_result = false;
                break;
                case NOT_ACTIVATED:
                case ACTIVATION_READY:
                case ACTIVATION_CONNECT:
                    ret_result = true;
                    skipFirmwareCheck = true;
                default:
                    ret_result = true;
            }
            LOGINFO("ret_result: [%s] skipFirmwareCheck:[%s]", (ret_result) ? "true" : "false", (skipFirmwareCheck) ? "true" : "false");
            return ret_result;
        }

        /**
         * @brief Checks the network connection status.
         *
         * This function queries the network plugin to determine if the device is connected to the internet.
         *
         * @return true if the device is connected to the internet, false otherwise.
         */
        bool MaintenanceManager::checkNetwork()
        {
            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.Network.1";
            PluginHost::IShell::state state;

            string token;

            if ((getServiceState(m_service, "org.rdk.Network", state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
            {
                LOGINFO("Network plugin is active");

                if (UNSOLICITED_MAINTENANCE == g_maintenance_type && !g_subscribed_for_nwevents)
                {
                    /* Subscribe for internetConnectionStatusChange event */
                    bool subscribe_status = subscribeForInternetStatusEvent("onInternetStatusChange");
                    if (subscribe_status)
                    {
                        LOGINFO("MaintenanceManager subscribed for onInternetStatusChange event");
                        g_subscribed_for_nwevents = true;
                    }
                    else
                    {
                        LOGERR("Failed to subscribe for onInternetStatusChange event");
                    }
                }
            }
            else
            {
                LOGERR("Network plugin is not active");
                return false;
            }

            // TODO: use interfaces and remove token
            auto security = m_service->QueryInterfaceByCallsign<PluginHost::IAuthenticate>("SecurityAgent");
            if (security != nullptr)
            {
                string payload = "http://localhost";
                if (security->CreateToken(
                        static_cast<uint16_t>(payload.length()),
                        reinterpret_cast<const uint8_t *>(payload.c_str()),
                        token) == Core::ERROR_NONE)
                {
                    LOGINFO("MaintenanceManager got security token");
                }
                else
                {
                    LOGERR("MaintenanceManager failed to get security token");
                }
                security->Release();
            }
            else
            {
                LOGERR("No security agent");
            }

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto thunder_client = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>>(callsign.c_str(), "", false, query);
            if (thunder_client != nullptr)
            {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "isConnectedToInternet", joGetParams, joGetResult);
                if (status > 0)
                {
                    LOGERR("%s call failed %d", callsign.c_str(), status);
                    return false;
                }
                else if (joGetResult.HasLabel("connectedToInternet"))
                {
                    LOGINFO("connectedToInternet status %s", (joGetResult["connectedToInternet"].Boolean()) ? "true" : "false");
                    return joGetResult["connectedToInternet"].Boolean();
                }
                else
                {
                    return false;
                }
            }

            LOGERR("thunder client failed");
            return false;
        }

        /**
         * @brief Checks if the device is online.
         *
         * This function checks the network connection status with retries.
         *
         * @return true if the device is online, false otherwise.
         */
        bool MaintenanceManager::isDeviceOnline()
        {
            bool network_available = false;
            LOGINFO("Checking device has network connectivity\n");
            /* add 4 checks every 30 seconds */
            network_available = checkNetwork();
            if (!network_available)
            {
                int retry_count = 0;
                while (retry_count < MAX_NETWORK_RETRIES)
                {
                    LOGINFO("Network not available. Sleeping for %d seconds", NETWORK_RETRY_INTERVAL);
                    sleep(NETWORK_RETRY_INTERVAL);
                    LOGINFO("Network retries [%d/%d] \n", ++retry_count, MAX_NETWORK_RETRIES);
                    network_available = checkNetwork();
                    if (network_available)
                    {
                        break;
                    }
                }
            }
            return network_available;
        }

        /**
         * @brief Sets the device initialization context.
         *
         * This function sets the device initialization context parameters via RFC.
         *
         * @param response_data The JSON object containing the initialization context.
         * @return true if the context was successfully set, false otherwise.
         */
        bool MaintenanceManager::setDeviceInitializationContext(JsonObject response_data)
        {
            bool setDone = false;
            bool paramEmpty = false;
            JsonObject getInitializationContext = response_data["deviceInitializationContext"].Object();
            for (const string &key : kDeviceInitContextKeyVals)
            {
                // Retrieve deviceInitializationContext Value
                string paramValue = getInitializationContext[key.c_str()].String();

                if (!paramValue.empty())
                {
                    if (strcmp(key.c_str(), "regionalConfigService") == 0)
                    {
                        paramValue = "https://" + paramValue;
                    }
                    LOGINFO("[deviceInitializationContext] %s : %s", key.c_str(), paramValue.c_str());

                    // Retrieve tr181 parameter from m_param_map
                    string rfc_parameter = m_param_map[key];

                    //  Retrieve parameter data type from m_paramType_map
                    DATA_TYPE rfc_dataType = m_paramType_map[key];

                    // Set the RFC values for deviceInitializationContext parameters
                    setRFC(rfc_parameter.c_str(), paramValue.c_str(), rfc_dataType);
                    LOGINFO("deviceInitializationContext parameters set successfully via RFC");

                    if (strcmp(key.c_str(), "partnerId") == 0)
                    {
                        setPartnerId(paramValue);
                    }
                }
                else
                {
                    LOGERR("Not able to fetch %s value from deviceInitializationContext", key.c_str());
                    paramEmpty = true;
                }
            }
            setDone = !paramEmpty;
            return setDone;
        }

        /**
         * @brief Subscribes to the device initialization context update event.
         *
         * This function subscribes to the `onDeviceInitializationContextUpdate` event from the Security Manager.
         *
         * @return true if the subscription was successful, false otherwise.
         */
        bool MaintenanceManager::subscribeToDeviceInitializationEvent()
        {
            int32_t status = Core::ERROR_NONE;
            bool result = false;
            string event = "onDeviceInitializationContextUpdate";
            const char *secMgr_callsign_ver = "org.rdk.SecManager.1";
            WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> *thunder_client = nullptr;

            // subscribe to onDeviceInitializationContextUpdate event
            LOGINFO("Attempting to subscribe for %s events", event.c_str());

            thunder_client = getThunderPluginHandle(secMgr_callsign_ver);
            if (thunder_client == nullptr)
            {
                LOGINFO("Failed to get plugin handle");
            }
            else
            {
                status = thunder_client->Subscribe<JsonObject>(5000, event, &MaintenanceManager::deviceInitializationContextEventHandler, this);
                if (status == Core::ERROR_NONE)
                {
                    result = true;
                }
            }
            g_subscribed_for_deviceContextUpdate = result;
            if (g_subscribed_for_deviceContextUpdate)
            {
                LOGINFO("MaintenanceManager subscribed for %s event", event.c_str());
                return true;
            }
            else
            {
                LOGINFO("Failed to subscribe for %s event", event.c_str());
                return false;
            }
        }

        MaintenanceManager::~MaintenanceManager()
        {
            MaintenanceManager::_instance = nullptr;
        }

        const string MaintenanceManager::Initialize(PluginHost::IShell *service)
        {
            ASSERT(service != nullptr);
            ASSERT(m_service == nullptr);
            ASSERT(timerid != nullptr);
            
            m_service = service;
            m_service->AddRef();

#if defined(ENABLE_WHOAMI)
            subscribeToDeviceInitializationEvent();
#endif

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif

            // Register Signal Handler
            if (signal(SIGALRM, timer_handler) == SIG_ERR)
            {
                LOGERR("Failed to register signal handler");
                return string("Failed to register signal handler");
            }
            LOGINFO("Signal Handler registered for Timer");

            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void MaintenanceManager::Deinitialize(PluginHost::IShell *service)
        {
            if (!maintenance_deleteTimer())
            {
                LOGINFO("Failed to delete timer");
            }
            LOGINFO("Timer Deleted on Deinitialization.");
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            stopMaintenanceTasks();
            DeinitializeIARM();
#endif

            ASSERT(service == m_service);

            m_service->Release();
            m_service = nullptr;
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void MaintenanceManager::InitializeIARM()
        {
            if (Utils::IARM::init())
            {
                IARM_Result_t res;
                // Register for the Maintenance Notification Events
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));

                maintenanceManagerOnBootup();
            }
        }

        void MaintenanceManager::maintenanceManagerOnBootup()
        {
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;

            MaintenanceManager::m_notify_status = MAINTENANCE_IDLE;
            MaintenanceManager::g_epoch_time = "";

            /* to know whether the maintenance is solicited or unsolicited */
            g_maintenance_type = UNSOLICITED_MAINTENANCE;
            LOGINFO("Triggering Maintenance on bootup");

            /* On bootup we check for opt-out value
             * if empty set the value to none */
            string OptOutmode = "NONE";
            OptOutmode = m_setting.getValue("softwareoptout").String();
            if (!checkValidOptOutModes(OptOutmode))
            {
                LOGINFO("OptOut Value is not Set. Setting to NONE \n");
                m_setting.remove("softwareoptout");
                OptOutmode = "NONE";
                m_setting.setValue("softwareoptout", OptOutmode);
            }
            else
            {
                LOGINFO("OptOut Value Found as: %s \n", OptOutmode.c_str());
            }

            MaintenanceManager::g_is_critical_maintenance = "false";
            MaintenanceManager::g_is_reboot_pending = "false";
            MaintenanceManager::g_lastSuccessful_maint_time = "";
            MaintenanceManager::g_task_status = 0;
            MaintenanceManager::m_abort_flag = false;
            MaintenanceManager::g_unsolicited_complete = false;

            /* we post to tell that the maintenance is in idle state at this moment */
            m_statusMutex.lock();
            MaintenanceManager::_instance->onMaintenanceStatusChange(m_notify_status);
            m_statusMutex.unlock();

            m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);
        }

        void MaintenanceManager::_MaintenanceMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (MaintenanceManager::_instance)
            {
                LOGWARN("IARM event Received with %d !", eventId);
                MaintenanceManager::_instance->iarmEventHandler(owner, eventId, data, len);
            }
            else
                LOGWARN("WARNING - cannot handle IARM events without MaintenanceManager plugin instance!");
        }

        void MaintenanceManager::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            m_statusMutex.lock();
            if (!m_abort_flag)
            {
                Maint_notify_status_t notify_status = MAINTENANCE_STARTED;
                IARM_Bus_MaintMGR_EventData_t *module_event_data = (IARM_Bus_MaintMGR_EventData_t *)data;
                IARM_Maint_module_status_t module_status;
                time_t successfulTime;
                string str_successfulTime = "";
                auto task_status_RFC = m_task_map.find(task_names_foreground[0].c_str());
                auto task_status_FWDLD = m_task_map.find(task_names_foreground[1].c_str());
                auto task_status_LOGUPLD = m_task_map.find(task_names_foreground[2].c_str());

                IARM_Bus_MaintMGR_EventId_t event = (IARM_Bus_MaintMGR_EventId_t)eventId;
                LOGINFO("Maintenance Event-ID = %d \n", event);

                if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME))
                {
                    if ((IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE == eventId) && (MAINTENANCE_STARTED == m_notify_status))
                    {
                        module_status = module_event_data->data.maintenance_module_status.status;
                        LOGINFO("MaintMGR Status %d \n", module_status);
                        string status_string = moduleStatusToString(module_status);
                        LOGINFO("MaintMGR Status %s \n", status_string.c_str());
                        switch (module_status)
                        {
                        case MAINT_RFC_COMPLETE:
                            if (task_status_RFC->second != true)
                            {
                                LOGINFO("Ignoring Event RFC_COMPLETE");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, RFC_SUCCESS);
                                SET_STATUS(g_task_status, RFC_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[0].c_str()] = false;
                            }
                            break;
                        case MAINT_FWDOWNLOAD_COMPLETE:
                            if (task_status_FWDLD->second != true)
                            {
                                LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_COMPLETE");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, SWUPDATE_SUCCESS);
                                SET_STATUS(g_task_status, SWUPDATE_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[1].c_str()] = false;
                            }
                            break;
                        case MAINT_LOGUPLOAD_COMPLETE:
                            if (task_status_LOGUPLD->second != true)
                            {
                                LOGINFO("Ignoring Event MAINT_LOGUPLOAD_COMPLETE");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, LOGUPLOAD_SUCCESS);
                                SET_STATUS(g_task_status, LOGUPLOAD_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[2].c_str()] = false;
                            }
                            break;
                        case MAINT_REBOOT_REQUIRED:
                            SET_STATUS(g_task_status, REBOOT_REQUIRED);
                            g_is_reboot_pending = "true";
                            break;
                        case MAINT_CRITICAL_UPDATE:
                            g_is_critical_maintenance = "true";
                            break;
                        case MAINT_FWDOWNLOAD_ABORTED:
                            SET_STATUS(g_task_status, TASK_SKIPPED);
                            /* Set FWDOWNLOAD Task as completed */
                            SET_STATUS(g_task_status, SWUPDATE_COMPLETE);
                            task_thread.notify_one();
                            m_task_map[task_names_foreground[1].c_str()] = false;
                            LOGINFO("FW Download task aborted \n");
                            break;
                        case MAINT_RFC_ERROR:
                            if (task_status_RFC->second != true)
                            {
                                LOGINFO("Ignoring Event RFC_ERROR");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, RFC_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in RFC Task \n");
                                m_task_map[task_names_foreground[0].c_str()] = false;
                            }

                            break;
                        case MAINT_LOGUPLOAD_ERROR:
                            if (task_status_LOGUPLD->second != true)
                            {
                                LOGINFO("Ignoring Event MAINT_LOGUPLOAD_ERROR");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, LOGUPLOAD_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in LOGUPLOAD Task \n");
                                m_task_map[task_names_foreground[2].c_str()] = false;
                            }

                            break;
                        case MAINT_FWDOWNLOAD_ERROR:
                            if (task_status_FWDLD->second != true)
                            {
                                LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_ERROR");
                                break;
                            }
                            else
                            {
                                SET_STATUS(g_task_status, SWUPDATE_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in SWUPDATE Task \n");
                                m_task_map[task_names_foreground[1].c_str()] = false;
                            }
                            break;
                        case MAINT_RFC_INPROGRESS:
                            m_task_map[task_names_foreground[0].c_str()] = true;
                            /* Set false once COMPLETE/ ERROR is received for RFC*/
                            LOGINFO(" RFC already IN PROGRESS -> setting m_task_map of RFC to true \n");
                            break;
                        case MAINT_FWDOWNLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[1].c_str()] = true;
                            /* Set false once COMPLETE/ ERROR is received for FWDOWNLOAD*/
                            LOGINFO(" FWDOWNLOAD already IN PROGRESS -> setting m_task_map of FWDOWNLOAD to true \n");
                            break;
                        case MAINT_LOGUPLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[2].c_str()] = true;
                            /* Set false once COMPLETE/ ERROR is received for LOGUPLOAD*/
                            LOGINFO(" LOGUPLOAD already IN PROGRESS -> setting m_task_map of LOGUPLOAD to true \n");
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        LOGINFO("Ignoring/Unknown Maintenance Status!!");
                        m_statusMutex.unlock();
                        return;
                    }

                    LOGINFO(" BITFIELD Status : %x", g_task_status);
                    /* Send the updated status only if all tasks complete execution
                     * Until that Maintenance is in started state */
                    if ((g_task_status & TASKS_COMPLETED) == TASKS_COMPLETED)
                    {
                        if ((g_task_status & ALL_TASKS_SUCCESS) == ALL_TASKS_SUCCESS)
                        { // all tasks success
                            LOGINFO("Maintenance Successfully Completed!!");
                            notify_status = MAINTENANCE_COMPLETE;
                            /*  we store the time in persistent location */
                            successfulTime = time(nullptr);
                            tm ltime = *localtime(&successfulTime);
                            time_t epoch_time = mktime(&ltime);
                            str_successfulTime = to_string(epoch_time);
                            LOGINFO("last succesful time is :%s", str_successfulTime.c_str());
                            /* Remove any old completion time */
                            m_setting.remove("LastSuccessfulCompletionTime");
                            m_setting.setValue("LastSuccessfulCompletionTime", str_successfulTime);
                        }
                        /* Check other than all success cases which means we have errors */
                        else if ((g_task_status & ALL_TASKS_SUCCESS) != ALL_TASKS_SUCCESS)
                        {
                            if ((g_task_status & MAINTENANCE_TASK_SKIPPED) == MAINTENANCE_TASK_SKIPPED)
                            {
                                LOGINFO("There are Skipped Task. Maintenance Incomplete");
                                notify_status = MAINTENANCE_INCOMPLETE;
                            }
                            else
                            {
                                LOGINFO("Maintenance Ended with Errors");
                                notify_status = MAINTENANCE_ERROR;
                            }
                        }

                        LOGINFO("ENDING MAINTENANCE CYCLE");
                        if (m_thread.joinable())
                        {
                            m_thread.join();
                            LOGINFO("Thread joined successfully\n");
                        }

                        if (g_maintenance_type == UNSOLICITED_MAINTENANCE && !g_unsolicited_complete)
                        {
                            g_unsolicited_complete = true;
                        }
                        MaintenanceManager::_instance->onMaintenanceStatusChange(notify_status);
                    }
                    else
                    {
                        LOGINFO("Tasks are not completed!!!!");
                    }
                }
                else
                {
                    LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
                }
            }
            else
            {
                LOGINFO("Maintenance has been aborted. Hence ignoring the event");
            }
            m_statusMutex.unlock();
        } /* end of iarmEventHandler() */

        void MaintenanceManager::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected())
            {
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_RemoveEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                MaintenanceManager::_instance = nullptr;
            }
        }
#endif /* end of USE_IARMBUS or USE_IARM_BUS */

#ifdef DEBUG
        /**
         * @brief : sampleAPI
         */
        uint32_t MaintenanceManager::sampleAPI(const JsonObject &parameters,
                                               JsonObject &response)
        {
            response["sampleAPI"] = "Success";
            /* Kept for debug purpose/future reference. */
            sendNotify(EVT_ONMAINTMGRSAMPLEEVENT, parameters);
            returnResponse(true);
        }
#endif

        /**
         * @brief This function returns the status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceActivityStatus",
         *                  "params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"status":MAINTENANCE_IDLE,"LastSuccessfulCompletionTime":
                              -1,"isCriticalMaintenance":true,"isRebootPending":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceActivityStatus(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFO("Request for getMaintenanceActivityStatus()");
            bool result = false;
            string isCriticalMaintenance = "false";
            string isRebootPending = "false";
            string LastSuccessfulCompletionTime = "NA"; /* TODO : check max size to hold this */
            string getMaintenanceStatusString = "\0";
            bool b_criticalMaintenace = false;
            bool b_rebootPending = false;

            std::lock_guard<std::mutex> guard(m_callMutex);

            /* Check if we have a critical maintenance */
            if (!g_is_critical_maintenance.empty())
            {
                isCriticalMaintenance = g_is_critical_maintenance;
            }

            if (!g_is_reboot_pending.empty())
            {
                isRebootPending = g_is_reboot_pending;
            }

            /* Get the last SuccessfulCompletion time from Persistant location */
            if (m_setting.contains("LastSuccessfulCompletionTime"))
            {
                LastSuccessfulCompletionTime = m_setting.getValue("LastSuccessfulCompletionTime").String();
            }

            if (!isCriticalMaintenance.compare("true"))
            {
                b_criticalMaintenace = true;
            }

            if (!isRebootPending.compare("true"))
            {
                b_rebootPending = true;
            }

            response["maintenanceStatus"] = notifyStatusToString(m_notify_status);
            if (strcmp("NA", LastSuccessfulCompletionTime.c_str()) == 0)
            {
                response["LastSuccessfulCompletionTime"] = 0; // stoi is not able handle "NA"
            }
            else
            {
                try
                {
                    response["LastSuccessfulCompletionTime"] = stoi(LastSuccessfulCompletionTime.c_str());
                }
                catch (exception &err)
                {
                    // exception caught with stoi -- So making "LastSuccessfulCompletionTime" as 0
                    response["LastSuccessfulCompletionTime"] = 0;
                }
            }
            response["isCriticalMaintenance"] = b_criticalMaintenace;
            response["isRebootPending"] = b_rebootPending;
            result = true;

            returnResponse(result);
        }

        /**
         * @brief This function returns the start time of the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceStartTime","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceStartTime":12345678,"success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceStartTime(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;
            string starttime = "";

            starttime = Utils::cRunScript("/lib/rdk/getMaintenanceStartTime.sh &");
            if (!starttime.empty())
            {
                response["maintenanceStartTime"] = stoi(starttime.c_str());
                result = true;
            }

            returnResponse(result);
        }

        /***
        * @brief	: Used to read file contents into a vector
        * @param1[in]	: Complete file name with path
        * @param2[in]	: Destination vector buffer to be filled with file contents
        * @return	: <bool>; TRUE if operation success; else FALSE.
        */
        bool getFileContent(std::string fileName, std::vector<std::string> &vecOfStrs)
        {
            bool retStatus = false;
            std::ifstream inFile(fileName.c_str(), ios::in);

            if (!inFile.is_open()){
                return retStatus;
            }

            std::string line;
            retStatus = true;
            while (std::getline(inFile, line))
            {
                if (line.size() > 0)
                {
                    vecOfStrs.push_back(line);
                }
            }
            inFile.close();
            return retStatus;
        }

        /**
         * @brief Parses a configuration file to find the value associated with a given key.
         *
         * This function reads the lines of a configuration file to locate a specified key
         * and retrieves its corresponding value.
         *
         * @param filename The name of the configuration file to parse.
         * @param findkey The key to search for in the configuration file.
         * @param value A reference to a string where the found value will be stored.
         * @return true if the key was found and the value was retrieved, false otherwise.
         */
        bool parseConfigFile(const char *filename, string findkey, string &value)
        {
            vector<std::string> lines;
            bool found=false;
            getFileContent(filename,lines);
            for (vector<std::string>::const_iterator i = lines.begin();
                 i != lines.end(); ++i){
                string line = *i;
                size_t eq = line.find_first_of("=");
                if (std::string::npos != eq) {
                    std::string key = line.substr(0, eq);
                    if (key == findkey) {
                        value = line.substr(eq + 1);
                        found=true;
                        break;
                    }
                }
            }

            if(found){
                return true;
            }
            else{
                return false;
            }
        }

        /**
         * @brief This function returns Mode of the maintenance.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceMode","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceMode":"FOREGROUND","optOut":"IGNORE_UPDATE","success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceMode(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;
            string softwareOptOutmode = "NONE";
            if (BACKGROUND_MODE != g_currentMode && FOREGROUND_MODE != g_currentMode)
            {
                LOGERR("Didnt get a valid Mode. Failed\n");
                returnResponse(false);
            }
            else
            {
                response["maintenanceMode"] = g_currentMode;

                if (Utils::fileExists(MAINTENANCE_MGR_RECORD_FILE))
                {
                    if (parseConfigFile(MAINTENANCE_MGR_RECORD_FILE, "softwareoptout", softwareOptOutmode))
                    {
                        /* check if the value is valid */
                        if (!checkValidOptOutModes(softwareOptOutmode))
                        {
                            LOGERR("OptOut Value Corrupted. Failed\n");
                            returnResponse(false);
                        }
                        else
                        {
                            LOGINFO("OptOut Value = %s", softwareOptOutmode.c_str());
                        }
                    }
                    else
                    {
                        LOGERR("OptOut Value Not Found. Failed\n");
                        returnResponse(false);
                    }
                }
                else
                {
                    LOGERR("OptOut Config File Not Found. Failed\n");
                    returnResponse(false);
                }
                response["optOut"] = softwareOptOutmode.c_str();
                result = true;
            }
            returnResponse(result);
        }

        /**
         * @brief This function returns the current status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.setMaintenanceMode",
         *                  "params":{"maintenanceMode":FOREGROUND}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::setMaintenanceMode(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFO("Request for setMaintenanceMode()");
            bool result = false;
            string new_mode = "";
            string old_mode = g_currentMode;
            string bg_flag = "false";
            string new_optout_state = "";
            bool rdkvfwrfc = false;
            // 1 = Foreground and 0 = background
            int mode = 1;

            rdkvfwrfc = readRFC(TR181_RDKVFWUPGRADER);
            /* Label should have maintenance mode and softwareOptout field */
            if (parameters.HasLabel("maintenanceMode") && parameters.HasLabel("optOut"))
            {

                new_mode = parameters["maintenanceMode"].String();

                if (BACKGROUND_MODE != new_mode && FOREGROUND_MODE != new_mode)
                {
                    LOGERR("value of new mode is incorrect, therefore \
                            current mode '%s' not changed.\n",
                           old_mode.c_str());
                    returnResponse(false);
                }

                std::lock_guard<std::mutex> guard(m_callMutex);

                /* check if maintenance is on progress or not */
                /* if in progress restrict the same */
                if (MAINTENANCE_STARTED != m_notify_status)
                {

                    LOGINFO("SetMaintenanceMode new_mode = %s\n", new_mode.c_str());

                    /* remove any older one */
                    m_setting.remove("background_flag");
                    if (BACKGROUND_MODE == new_mode)
                    {
                        bg_flag = "true";
                    }
                    else
                    {
                        /* foreground */
                        bg_flag = "false";
                    }
                    g_currentMode = new_mode;
                    m_setting.setValue("background_flag", bg_flag);
                }
                else
                {
                    /*If firmware rfc is true and IARM bus component present allow to change maintenance mode*/
                    if (rdkvfwrfc == true)
                    {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                        LOGINFO("SetMaintenanceMode new_mode = %s\n", new_mode.c_str());
                        /* Sending IARM Event to application for mode change */
                        (new_mode != BACKGROUND_MODE) ? mode = 1 : mode = 0;
                        LOGINFO("setMaintenanceMode rfc is true and mode:%d\n", mode);
                        IARM_Result_t ret_code = IARM_RESULT_SUCCESS;
                        ret_code = IARM_Bus_BroadcastEvent("RdkvFWupgrader", (IARM_EventId_t)0, (void *)&mode, sizeof(mode));
                        if (ret_code == IARM_RESULT_SUCCESS)
                        {
                            LOGINFO("IARM_Bus_BroadcastEvent is success and value=%d\n", mode);
                            g_currentMode = new_mode;
                            /* remove any older one */
                            m_setting.remove("background_flag");
                            if (BACKGROUND_MODE == new_mode)
                            {
                                bg_flag = "true";
                            }
                            else
                            {
                                /* foreground */
                                bg_flag = "false";
                            }
                            m_setting.setValue("background_flag", bg_flag);
                        }
                        else
                        {
                            LOGINFO("IARM_Bus_BroadcastEvent is fail Mode change not allowed and value=%d\n", mode);
                        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
                    }
                    else
                    {
                        LOGERR("Maintenance is in Progress, Mode change not allowed");
                    }
                    result = true;
                }

                /* OptOut changes here */
                new_optout_state = parameters["optOut"].String();

                LOGINFO("SetMaintenanceMode optOut = %s\n", new_optout_state.c_str());

                /* check if we have a valid state from user */
                if (checkValidOptOutModes(new_optout_state))
                {
                    /* we got a valid state; Now store it in persistant location */
                    m_setting.setValue("softwareoptout", new_optout_state);
                }
                else
                {
                    LOGINFO("Invalid optOut = %s\n", new_optout_state.c_str());
                    returnResponse(false);
                }

                /* Set the result as true */
                result = true;
            }
            else
            {
                /* havent got the correct label */
                LOGERR("SetMaintenanceMode Missing Key Values\n");
            }

            returnResponse(result);
        }

        /**
         * @brief This function starts the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.startMaintenance",
         *                  "params":{}}''
         * @param2[out]:{"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::startMaintenance(const JsonObject &parameters, JsonObject &response)
        {
            LOGINFO("Request for startMaintenance()");
            bool result = false;
            /* check what mode we currently have */
            string current_mode = "";

            LOGINFO("Triggering scheduled maintenance ");
            /* only one maintenance at a time */
            /* Lock so that m_notify_status will not be updated  further */
            m_statusMutex.lock();
            if (MAINTENANCE_STARTED != m_notify_status && g_unsolicited_complete)
            {

                /*reset the status to 0*/
                g_task_status = 0;
                g_maintenance_type = SOLICITED_MAINTENANCE;

                m_abort_flag = false;

                /* isRebootPending will be set to true
                 * irrespective of XConf configuration */
                g_is_reboot_pending = "true";

                /* we set this to false */
                g_is_critical_maintenance = "false";

                /* if there is any active thread, join it before executing the tasks from startMaintenance
                 * especially when device is in offline mode*/
                if (m_thread.joinable())
                {
                    m_thread.join();
                    LOGINFO("Thread joined successfully");
                }

                m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);

                result = true;
            }
            else
            {
                LOGINFO("Already a maintenance is in Progress. Please wait for it to complete !!");
            }
            m_statusMutex.unlock();
            returnResponse(result);
        }

        /*
         * @brief This function stops the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.stopMaintenance",
         *                  "params":{}}''
         * @param2[out]:{"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::stopMaintenance(const JsonObject &parameters, JsonObject &response)
        {
            bool result = false;
            if (readRFC(TR181_STOP_MAINTENANCE))
            {
                result = stopMaintenanceTasks();
            }
            else
            {
                LOGERR("Failed to initiate stopMaintenance, RFC is set as False");
            }
            returnResponse(result);
        }

        /**
         * @brief Stops ongoing maintenance tasks.
         *
         * This function stops current maintenance activities by setting the abort flag
         * and terminating running tasks. It also stops the task timer and changes the maintenance status.
         *
         * @return true if the maintenance tasks were successfully stopped, false otherwise.
         */
        bool MaintenanceManager::stopMaintenanceTasks()
        {
            LOGINFO("Request for stopMaintenance()");
            string codeDLtask;
            int k_ret = EINVAL;
            int i = 0;
            bool task_status[3] = {false};
            bool result = false;

            LOGINFO("Stopping maintenance activities");
            /* run only when the maintenance status is MAINTENANCE_STARTED */
            m_statusMutex.lock();
            if (MAINTENANCE_STARTED == m_notify_status)
            {

                // Set the condition flag m_abort_flag to true
                m_abort_flag = true;

                auto task_status_RFC = m_task_map.find(task_names_foreground[0].c_str());
                auto task_status_FWDLD = m_task_map.find(task_names_foreground[1].c_str());
                auto task_status_LOGUPLD = m_task_map.find(task_names_foreground[2].c_str());

                task_status[0] = task_status_RFC->second;
                task_status[1] = task_status_FWDLD->second;
                task_status[2] = task_status_LOGUPLD->second;

                for (i = 0; i < 3; i++)
                {
                    LOGINFO("task status [%d]  = %s ScriptName %s", i, (task_status[i]) ? "true" : "false", task_names[i].c_str());
                }
                for (i = 0; i < 3; i++)
                {
                    if (task_status[i])
                    {

                        k_ret = abortTask(task_names[i].c_str()); // default signal is SIGABRT

                        if (k_ret == 0)
                        {                                                         // if task(s) was(were) killed successfully ...
                            m_task_map[task_names_foreground[i].c_str()] = false; // set it to false
                        }
                        /* No need to loop again */
                        break;
                    }
                    else
                    {
                        LOGINFO("Task[%d] is false \n", i);
                    }
                }
                result = true;
            }
            else
            {
                LOGERR("Failed to stopMaintenance without starting maintenance");
            }
            if (task_stopTimer())
            {
                LOGINFO("Stopped Timer Successfully..");
            }
            else
            {
                LOGERR("task_stopTimer() did not stop the Timer...");
            }
            task_thread.notify_one();

            if (m_thread.joinable())
            {
                m_thread.join();
                LOGINFO("Thread joined successfully");
            }

            if (UNSOLICITED_MAINTENANCE == g_maintenance_type && !g_unsolicited_complete)
            {
                g_unsolicited_complete = true;
            }

            LOGINFO("Maintenance has been stopped. Hence setting maintenance status to MAINTENANCE_ERROR");
            MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
            m_statusMutex.unlock();

            return result;
        }

        /**
         * @brief Reads an RFC parameter.
         *
         * This function reads an RFC parameter value and checks if it is enabled.
         *
         * @param rfc The RFC parameter name to read.
         * @return true if the RFC parameter is enabled, false otherwise.
         */
        bool MaintenanceManager::readRFC(const char *rfc)
        {
            bool ret = false;
            RFC_ParamData_t param;
            if (rfc == NULL)
            {
                return ret;
            }
            WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("MaintenanceManager"), rfc, &param);
            if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE)
            {
                LOGINFO("rfc read success");
                if (param.type == WDMP_BOOLEAN)
                {
                    LOGINFO("rfc type is boolean");
                    if (strncasecmp(param.value, "true", 4) == 0)
                    {
                        LOGINFO("rfc value=%s", param.value);
                        ret = true;
                    }
                }
            }
            LOGINFO(" %s = %s , call value %d ", rfc, (ret == true) ? "true" : "false", wdmpStatus);
            return ret;
        }

        /**
         * @brief Aborts a task by sending a signal.
         *
         * This function gets the PID of a task by its name and sends a specified signal to terminate it.
         *
         * @param taskname The name of the task to abort.
         * @param sig_to_send The signal to send to the task.
         * @return 0 if the signal was successfully sent, otherwise an error code.
         */
        int MaintenanceManager::abortTask(const char *taskname, int sig_to_send)
        {
            int k_ret = EINVAL;
            pid_t pid_num = getTaskPID(taskname);
            LOGINFO("PID of %s is %d \n", taskname, (int)pid_num);

            if (pid_num != -1)
            {
                /* send the signal to task to terminate */
                k_ret = kill(pid_num, sig_to_send);
                LOGINFO(" %s sent signal %d\n", taskname, sig_to_send);
                if (k_ret == 0)
                {
                    LOGINFO(" %s Terminated\n", taskname);
                }
                else
                {
                    LOGINFO("Failed to terminate with error %s - %d \n", taskname, k_ret);
                }
            }
            else
            {
                LOGINFO("Didnt find PID for %s\n", taskname);
            }
            return k_ret;
        }

        /**
         * @brief Gets the PID of a running task by its name.
         *
         * This function retrieves the PID of a task based on its name by scanning the /proc directory.
         *
         * @param taskname The name of the task to find the PID for.
         * @return The PID of the task if found, otherwise -1.
         */
        pid_t MaintenanceManager::getTaskPID(const char *taskname)
        {
            DIR *dir = opendir(PROC_DIR);
            if (!dir)
            {
                LOGINFO("Failed to open %s", PROC_DIR);
                return -1;
            }

            struct dirent *ent;
            char *endptr;
            char buf[512];

            while ((ent = readdir(dir)) != NULL)
            {
                long lpid = strtol(ent->d_name, &endptr, 10);
                if (*endptr != '\0')
                {
                    continue;
                }

                /* Get the PID */
                snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);

                /* Open the cmdline and read */
                FILE *fp = fopen(buf, "r");
                if (fp)
                {
                    char *arg = 0;
                    size_t size = 0;
                    while (getdelim(&arg, &size, 0, fp) != -1)
                    {
                        printf("%s\n",arg);
                        char *first = strstr(arg, taskname);
                        if (first != NULL)
                        {
                            free(arg);
                            fclose(fp);
                            closedir(dir);
                            return (pid_t)lpid;
                        }
                    }
                    free(arg);
                    fclose(fp);
                }
            }
            closedir(dir);
            return (pid_t)-1;
        }

        /**
         * @brief Notifies about the change in maintenance status.
         *
         * This function updates the maintenance status and sends a notification about the status change.
         *
         * @param status The new maintenance status.
         */
        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status_t status)
        {
            JsonObject params;
            /* we store the updated value as well */
            m_notify_status = status;
            params["maintenanceStatus"] = notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }
    } /* namespace Plugin */
} /* namespace WPEFramework */
