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

#include "MaintenanceManager.h"
#include "utils.h"

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#endif /* USE_IARMBUS || USE_IARM_BUS */

#if defined(HAS_API_SYSTEM) && defined(HAS_API_POWERSTATE)
#include "powerstate.h"
#endif /* HAS_API_SYSTEM && HAS_API_POWERSTATE */

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

string notifyStatusToString(Maint_notify_status_t &status)
{
    string ret_status="";
    switch(status){
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

string moduleStatusToString(IARM_Maint_module_status_t &status)
{
    string ret_status="";
    switch(status){
        case MAINT_DCM_COMPLETE:
            ret_status="MAINTENANCE_DCM_COMPLETE";
            break;
        case MAINT_DCM_ERROR:
            ret_status="MAINTENANCE_DCM_ERROR";
            break;
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
        case MAINT_PINGTELEMETRY_COMPLETE:
            ret_status="MAINTENANCE_PINGTELEMETRY_COMPLETE";
            break;
        case MAINT_FWDOWNLOAD_COMPLETE:
            ret_status="MAINTENANCE_FWDONLOAD_COMPLETE";
            break;
        case MAINT_FWDOWNLOAD_ERROR:
            ret_status="MAINTENANCE_FWDONLOAD_ERROR";
            break;
        case MAINT_REBOOT_REQUIRED:
            ret_status="MAINTENANCE_REBOOT_REQUIRED";
            break;
        case MAINT_FWDOWNLOAD_ABORTED:
            ret_status="MAINTENANCE_FWDOWNLOAD_ABORTED";
            break;
        default:
            ret_status="MAINTENANCE_EMPTY";
    }
    return ret_status;
}
/**
 * @brief WPEFramework class for Maintenance Manager
 */
namespace WPEFramework {
    namespace Plugin {
        //Prototypes
        SERVICE_REGISTRATION(MaintenanceManager,API_VERSION_NUMBER_MAJOR,API_VERSION_NUMBER_MINOR);
        /* Globale time Structure */
        //sTime MaintenanceManager::g_start_time;
        MaintenanceManager* MaintenanceManager::_instance = nullptr;
        Maint_notify_status_t MaintenanceManager::g_notify_status=MAINTENANCE_IDLE;
        string MaintenanceManager::g_epoch_time;
        std::string MaintenanceManager::g_currentMode = "";
        cSettings MaintenanceManager::m_setting(MAITNENANCE_MGR_RECORD_FILE);
        string MaintenanceManager::g_is_critical_maintenance="";
        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager()
            :AbstractPlugin()
        {
            MaintenanceManager::_instance = this;
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;

            // default 0300Hrs UTC
            MaintenanceManager::g_epoch_time = "10800";

             LOGINFO("DBG: At init executing");
            /* we call dcmscript to get the new start time */
            if (Utils::fileExists("/lib/rdk/StartDCM.sh")) {
//                string dcmscript_command("sh /lib/rdk/StartDCM.sh &");
//                LOGINFO("DBG: Executing %s\n", dcmscript_command.c_str());
//                Utils::cRunScript(dcmscript_command.c_str());
                  int32_t sysResult=E_NOK;
                  sysResult = system("/lib/rdk/StartDCM.sh &");
            }
            else {
                LOGINFO("DBG: Unable to find StartDCM.sh \n");
            }
            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            registerMethod("sampleMaintenanceManagerAPI", &MaintenanceManager::sampleAPI, this);
#endif /* DEBUG */
            registerMethod("getMaintenanceActivityStatus", &MaintenanceManager::getMaintenanceActivityStatus,this);
            registerMethod("getMaintenanceStartTime", &MaintenanceManager::getMaintenanceStartTime,this);
            registerMethod("setMaintenanceMode", &MaintenanceManager::setMaintenanceMode,this);
            registerMethod("startMaintenance", &MaintenanceManager::startMaintenance,this);
      }


        MaintenanceManager::~MaintenanceManager()
        {
            MaintenanceManager::_instance = nullptr;
        }

        const string MaintenanceManager::Initialize(PluginHost::IShell*)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            InitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void MaintenanceManager::Deinitialize(PluginHost::IShell*)
        {
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
            DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */
        }

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
        void MaintenanceManager::InitializeIARM()
        {
            if (Utils::IARM::init()) {
                LOGINFO();
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_Connect());
                // Register for the Maintenance Notification Events
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                //Register for setMaintenanceStartTime
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT,_MaintenanceMgrEventHandler));
           }
        }

        void MaintenanceManager::_MaintenanceMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (MaintenanceManager::_instance){
                LOGWARN("IARM event Received with %d !", eventId);
                MaintenanceManager::_instance->iarmEventHandler(owner, eventId, data, len);
            }
            else
                LOGWARN("WARNING - cannot handle IARM events without MaintenanceManager plugin instance!");
        }

        void MaintenanceManager::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            Maint_notify_status_t m_notify_status;
            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status_t module_status;
            LOGINFO("Event-ID = %d \n",eventId);
            IARM_Bus_MaintMGR_EventId_t event = (IARM_Bus_MaintMGR_EventId_t)eventId;
            LOGINFO("event= %d \n",event);

            if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME)) {
                if ( IARM_BUS_DCM_NEW_START_TIME_EVENT == eventId ) {
                    /* we got a new start time from DCM script */
                    string l_time(module_event_data->data.startTimeUpdate.start_time);
                    LOGINFO("MaintMGR Start Time %s \n", l_time.c_str());
                    /* Store it in a Global structure */
                    g_epoch_time=l_time;
                }
                else if ( IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE == eventId ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                    LOGINFO("MaintMGR Status %d \n",module_status);
                    string status_string=moduleStatusToString(module_status);
                    LOGINFO("MaintMGR Status %s \n", status_string.c_str());
                    if ( MAINT_CRITICAL_UPDATE == module_status ){
                        g_is_critical_maintenance="true";
                    }
                    else if ( (MAINT_RFC_ERROR == module_status) ||
                            ( MAINT_DCM_ERROR == module_status) )
                    {
                        MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
                    }
                    //TODO handle all other case scenarios.
                }
                else{
                    LOGINFO("Unknown Maintenance Status!!");
                }
                /* Logic to strore the status as bit fields about each module */
            }
            else {
                LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
            }
        }
        void MaintenanceManager::DeinitializeIARM()
        {
            if (Utils::IARM::isConnected()){
                IARM_Result_t res;
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT));
                MaintenanceManager::_instance = nullptr;
                IARM_CHECK(IARM_Bus_Disconnect());
                IARM_CHECK(IARM_Bus_Term());
            }
        }
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

#ifdef DEBUG
        /**
         * @brief : sampleAPI
         */
        uint32_t MaintenanceManager::sampleAPI(const JsonObject& parameters,
                JsonObject& response)
        {
            response["sampleAPI"] = "Success";
            /* Kept for debug purpose/future reference. */
            sendNotify(EVT_ONMAINTMGRSAMPLEEVENT, parameters);
            returnResponse(true);
        }
#endif /* DEBUG */

        /*
         * @brief This function returns the status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.GetMaintenanceActivityStatus",
         *                  "params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"status":MAINTENANCE_IDLE,"LastSuccessfulCompletionTime":
                              -1,"isCriticalMaintenance":true,"isRebootPending":true,}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::getMaintenanceActivityStatus(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    string isCriticalMaintenance = "false";
                    bool isRebootPending = false;
                    string LastSuccessfulCompletionTime = "NA"; /* TODO : check max size to hold this */
                    string getMaintenanceStatusString = "\0";

                    /* Check if we have a critical maintenance */
                    if (!g_is_critical_maintenance.empty()){
                        isCriticalMaintenance=g_is_critical_maintenance;
                    }

                    /* Get the last SuccessfulCompletion time from Persistant location */
                    if (m_setting.contains("LastSuccessfulCompletionTime")){
                        LastSuccessfulCompletionTime=m_setting.getValue("LastSuccessfulCompletionTime").String();
                    }

                    response["status"] = notifyStatusToString(g_notify_status);
                    response["LastSuccessfulCompletionTime"] = LastSuccessfulCompletionTime;
                    response["isCriticalMaintenance"] = isCriticalMaintenance;
                    response["isRebootPending"] = isRebootPending;
                    result = true;

                    returnResponse(result);
                }
        /*
         * @brief This function returns the start time of the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.GetMaintenanceStartTime","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"time":03:45","success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceStartTime (const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    if(!g_epoch_time.empty()) {
                        response["time"] = g_epoch_time.c_str();
                        result=true;
                    }
                    returnResponse(result);
                }

        /*
         * @brief This function returns the current status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.SetMaintenanceMode",
         *                  "params":{"Mode":FOREGROUND}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::setMaintenanceMode(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    string new_mode = "";
                    string old_mode = g_currentMode;
                    string abort_flag = "false";

                    /*  we set the default value to FG */
                    if ( parameters.HasLabel("maintenanceMode") ){
                        /* Get the value */
                        new_mode = parameters["maintenanceMode"].String();

                        LOGINFO("SetMaintenanceMode new_mode = %s\n",new_mode.c_str());
                        if ( BACKGROUND_MODE != new_mode && FOREGROUND_MODE != new_mode )  {
                            LOGERR("value of new mode is incorrect, therefore \
                                    current mode '%s' not changed.\n", old_mode.c_str());
                            returnResponse(false);
                        }
                        if ( BACKGROUND_MODE == new_mode ) {
                            g_currentMode = new_mode;
                            abort_flag="true";
                            m_setting.setValue("abort_flag",abort_flag);
                        }
                        else {
                            /* foreground */
                            g_currentMode =new_mode;
                            m_setting.remove("abort_flag");
                        }
                        result = true;
                    }
                    else {
                          /* havent got the correct label */
                        LOGERR("SetMaintenanceMode Missing Key Values\n");
                        populateResponseWithError(SysSrv_MissingKeyValues,response);
                    }

                    returnResponse(result);

                }

        /*
         * @brief This function starts the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.StartMaintenance",
         *                  "params":{}}''
         * @param2[out]:{"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::startMaintenance(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    int32_t exec_status=E_NOK;
                    Maint_notify_status_t notify_status = MAINTENANCE_IDLE;
                    /* check what mode we currently have */
                    string current_mode="";

                    /* check if there is any critical maintenance happening */
                    if (!(g_is_critical_maintenance.compare("true"))){
                        /* then run in FG itself */
                        g_currentMode=FOREGROUND_MODE;
                        LOGINFO("Critical Maintenance is True. Running in Foreground forcefully \n");
                    }

                    current_mode=g_currentMode;

                    /* notify that we started the maintenance*/
                    MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
                    if (Utils::fileExists("/lib/rdk/RFCbase.sh")) {
                        LOGINFO("executing RFCbase.sh\n");
                        exec_status=system("/lib/rdk/RFCbase.sh &");
                        if (exec_status != E_OK) {
                            LOGINFO("Failed to execute RFCbase.sh\n");
                        }
                    }
                    else {
                        LOGINFO("RFCbase.sh is not present \n");
                    }
                    if (Utils::fileExists("/lib/rdk/dca_utility.sh")) {
                        exec_status=system("sh /lib/rdk/dca_utility.sh 0 0 &");
                        LOGINFO("executing DCA UTILITY");
                        if (exec_status != E_OK) {
                            LOGINFO("Failed to execute DCA utility\n");
                        }
                    }

                    if ( BACKGROUND_MODE != current_mode ) {
                        if (Utils::fileExists("/lib/rdk/deviceInitiatedFWDnld.sh")) {
                            LOGWARN("Checking for new Firmware\n");
                            exec_status=system("/lib/rdk/deviceInitiatedFWDnld.sh 0 4 >> /opt/logs/swupdate.log &");
                            if (exec_status != E_OK) {
                                LOGINFO("Failed to execute deviceInitiatedFWDnld.sh\n");
                            }
                        }
                        else {
                            LOGINFO("deviceInitiatedFWDnld is not present \n");
                        }
                    }
                    else {
                        LOGINFO("SKipping deviceInitiatedFWDnld due to Background Mode \n");
                    }
                    result=true;
                    returnResponse(result);
                }

        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status_t status) {
            JsonObject params;
            /* we store the updated value as well */
            g_notify_status=status;
            params["onMaintenanceStatusChange"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }

   } /* namespace Plugin */
} /* namespace WPEFramework */
