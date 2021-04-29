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
        case MAINT_PINGTELEMETRY_ERROR:
            ret_status="MAINTENANCE_PINGTELEMETRY_ERROR";
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
namespace WPEFramework {
    namespace Plugin {
        //Prototypes
        SERVICE_REGISTRATION(MaintenanceManager,API_VERSION_NUMBER_MAJOR,API_VERSION_NUMBER_MINOR);
        /* Global time variable */
        MaintenanceManager* MaintenanceManager::_instance = nullptr;
        Maint_notify_status_t MaintenanceManager::g_notify_status=MAINTENANCE_IDLE;
        string MaintenanceManager::g_epoch_time;
        std::string MaintenanceManager::g_currentMode = "";
        string MaintenanceManager::g_is_critical_maintenance="";
        string MaintenanceManager::g_is_reboot_pending="";
        string MaintenanceManager::g_lastSuccessful_maint_time="";
        uint8_t MaintenanceManager::g_task_status=0;
        cSettings MaintenanceManager::m_setting(MAINTENANCE_MGR_RECORD_FILE);
        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager()
            :AbstractPlugin()
        {
            MaintenanceManager::_instance = this;
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;

            MaintenanceManager::g_epoch_time="";

            bool skip_task=false;

            int32_t exec_status=E_NOK;

            /* we call dcmscript to get the new start time */
            if (Utils::fileExists("/lib/rdk/StartDCM_maintaince.sh")) {
                  exec_status = system("/lib/rdk/StartDCM_maintaince.sh &");
                  if ( E_OK == exec_status ){
                      LOGINFO("DBG:Succesfully executed StartDCM_maintaince.sh \n");
                  }
                  else {
                      LOGINFO("DBG:Failed to execute StartDCM_maintaince.sh !! \n");
                  }
            }
            else {
                LOGINFO("DBG: Unable to find StartDCM_maintaince.sh \n");
            }

            /* We set the bit to say we have started the maintenance */
            SET_STATUS(g_task_status,TASKS_STARTED);

            /* we run all the critical maintenance activities */
            if (Utils::fileExists("/lib/rdk/RFCbase.sh")) {
                LOGINFO("executing RFCbase.sh\n");
                exec_status=system("/lib/rdk/RFCbase.sh &");
                if (exec_status == E_OK) {
                    LOGINFO("RFCbase execution Success\n");
                }
                else {
                    LOGINFO("Failed to execute RFCbase.sh\n");
                    skip_task=true;
                }
            }
            else {
                LOGINFO("RFCbase.sh is not present \n");
                skip_task=true;
            }

            if (Utils::fileExists("/lib/rdk/Start_uploadSTBLogs.sh")) {
                exec_status=system("sh /lib/rdk/Start_uploadSTBLogs.sh &");
                if (exec_status == E_OK) {
                    LOGINFO("Successfully executed lib/rdk/Start_uploadSTBLogs.sh \n");
                }
                else{
                    LOGINFO( "Failed to execute /lib/rdk/Start_uploadSTBLogs.sh \n");
                    skip_task=true;
                }
            }
            else {
                LOGINFO("Start_uploadSTBLogs.sh is not present \n");
                skip_task=true;
            }
            if (Utils::fileExists("/lib/rdk/deviceInitiatedFWDnld.sh")) {
                LOGWARN("Checking for new Firmware\n");
                exec_status=system("/lib/rdk/deviceInitiatedFWDnld.sh 0 1 >> /opt/logs/swupdate.log &");
                if (exec_status == E_OK) {
                    LOGINFO("Successfully executed DIFD\n");
                }
                else{
                    LOGINFO( "Failed to execute DIFD \n" );
                    skip_task=true;
                }
            }
            else {
                LOGINFO("deviceInitiatedFWDnld is not present \n");
                skip_task=true;
            }

            if (Utils::fileExists("/lib/rdk/ping-telemetry.sh")) {
                exec_status=system("sh /lib/rdk/ping-telemetry.sh &");
                if (exec_status == E_OK) {
                    LOGINFO("Successfully executed lib/rdk/ping-telemetry.sh \n");
                }
                else{
                    LOGINFO( "Failed to execute /lib/rdk/ping-telemetry.sh \n");
                    skip_task=true;
                }
            }
            else {
                LOGINFO("ping-telemetry.sh is not present \n");
                skip_task=true;
            }


            if (skip_task){
                /* we set the corresponding flag to set
                 * which means one of then skipped */
                SET_STATUS(g_task_status,TASK_SKIPPED);
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
            Maint_notify_status_t m_notify_status=MAINTENANCE_STARTED;
            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status_t module_status;
            bool task_error=false;
            time_t successfulTime;
            string str_successfulTime="";

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
                    switch (module_status) {
                        case MAINT_RFC_COMPLETE :
                            SET_STATUS(g_task_status,RFC_SUCCESS);
                            break;
                        case MAINT_DCM_COMPLETE :
                            SET_STATUS(g_task_status,DCM_SUCCESS);
                            break;
                        case MAINT_FWDOWNLOAD_COMPLETE :
                            SET_STATUS(g_task_status,DIFD_SUCCESS);
                            break;
                        case MAINT_PINGTELEMETRY_COMPLETE:
                            SET_STATUS(g_task_status,PING_TELEMETRY_SUCCESS);
                            break;
                        case MAINT_LOGUPLOAD_COMPLETE :
                            SET_STATUS(g_task_status,LOGUPLOAD_SUCCESS);
                            break;
                        case MAINT_REBOOT_REQUIRED :
                            SET_STATUS(g_task_status,REBOOT_REQUIRED);
                            g_is_reboot_pending="true";
                            break;
                        case MAINT_CRITICAL_UPDATE:
                            g_is_critical_maintenance="true";
                            break;
                        case MAINT_FWDOWNLOAD_ABORTED:
                            SET_STATUS(g_task_status,TASK_SKIPPED);
                            task_error=true;
                            break;
                        case MAINT_DCM_ERROR:
                        case MAINT_RFC_ERROR:
                        case MAINT_LOGUPLOAD_ERROR:
                        case MAINT_PINGTELEMETRY_ERROR:
                        case MAINT_FWDOWNLOAD_ERROR:
                            LOGINFO("Error encountered in one of the task \n");
                            task_error=true;
                            break;
                    }
                    LOGINFO(" BITFIELD Status : %x",g_task_status);
                }
                else{
                    LOGINFO("Unknown Maintenance Status!!");
                }

                /* Send the updated status only if all task started */

                if ( (g_task_status & 0x80 ) == 0x80 ){
                    if ( (g_task_status & 0x1F) == 0x1F ){
                        LOGINFO("DBG:Maintenance Successfully Completed!!");
                        m_notify_status=MAINTENANCE_COMPLETE;
                        successfulTime=time(nullptr);
                        str_successfulTime=to_string(successfulTime);
                        /* Remove any old completion time */
                        m_setting.remove("LastSuccessfulCompletionTime");
                        m_setting.setValue("LastSuccessfulCompletionTime",str_successfulTime.c_str());
                        MaintenanceManager::_instance->onMaintenanceStatusChange(m_notify_status);
                    }
                    /* we send only updated notification if one of the
                     * task returned with error or DIFD encountered abort */
                    if ( task_error) {
                        if ((g_task_status & 0x40 ) == 0x40 ){
                            LOGINFO("DBG:There are Skipped Task. Incomplete");
                            m_notify_status=MAINTENANCE_INCOMPLETE;
                        }
                        else {
                            LOGINFO("DBG:There are Errors");
                            m_notify_status=MAINTENANCE_ERROR;
                        }

                        MaintenanceManager::_instance->onMaintenanceStatusChange(m_notify_status);
                    }
                }
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
                    string isRebootPending = "false";
                    string LastSuccessfulCompletionTime = "NA"; /* TODO : check max size to hold this */
                    string getMaintenanceStatusString = "\0";

                    /* Check if we have a critical maintenance */
                    if (!g_is_critical_maintenance.empty()){
                        isCriticalMaintenance=g_is_critical_maintenance;
                    }
                    if (!g_is_reboot_pending.empty()){
                        isRebootPending=g_is_reboot_pending;
                    }
                    /* Get the last SuccessfulCompletion time from Persistant location */
                    if (m_setting.contains("LastSuccessfulCompletionTime")){
                        LastSuccessfulCompletionTime=m_setting.getValue("LastSuccessfulCompletionTime").String();
                    }

                    response["maintenanceStatus"] = notifyStatusToString(g_notify_status);
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
                response["maintenanceStartTime"] = g_epoch_time.c_str();
                result=true;
            }
            else {
                /* use default time 0300Hrs */
                time_t start_time=10800;
                time_t start_epoch;
                time_t tEpoc = time(nullptr);
                long int sec;
                tm ltime = *localtime(&tEpoc);
                tm lmidnight = ltime;
                lmidnight.tm_hour = 0;
                lmidnight.tm_min  = 0;
                lmidnight.tm_sec  = 0;
                time_t midnightEpoc = mktime(&lmidnight);
                /*we add the offset and start time */
                LOGINFO("midnightEpoc =%ld \n", midnightEpoc);
                start_epoch=midnightEpoc+start_time+14400;
                LOGINFO("start_epoch = %ld \n",start_epoch);
                /*check if it is the same day or not */
                sec=start_epoch-tEpoc;
                LOGINFO("sec = %ld",sec);
                if (sec < 0) {
                    int secInDay = 23*60*60 + 59*60 + 60;
                    sec = start_epoch + secInDay;
                }
                else{
                    sec = start_epoch;
                }
                response["maintenanceStartTime"] = to_string(sec);
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
                            abort_flag="false";
                            m_setting.setValue("abort_flag",abort_flag);
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
                    bool skip_task=false;
                    string abort_flag="";

                    /*reset the status to 0*/
                    g_task_status=0;

                    /* check if there is any critical maintenance happening */
                    if (!(g_is_critical_maintenance.compare("true"))){
                        /* then run in FG itself */
                        g_currentMode=FOREGROUND_MODE;
                        LOGINFO("Critical Maintenance is True. Running in Foreground forcefully \n");
                    }

                    current_mode=g_currentMode;

                    /* notify that we started the maintenance */
                    MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
                    /* We set the bit to say we have started the maintenance */
                    SET_STATUS(g_task_status,TASKS_STARTED);
                    if (Utils::fileExists("/lib/rdk/RFCbase.sh")) {
                        LOGINFO("executing RFCbase.sh\n");
                        exec_status=system("/lib/rdk/RFCbase.sh &");
                        if (exec_status == E_OK) {
                            LOGINFO("RFCbase execution Success\n");
                        }
                        else {
                            LOGINFO("Failed to execute RFCbase.sh\n");
                            skip_task=true;
                        }
                    }
                    else {
                        LOGINFO("RFCbase.sh is not present \n");
                        skip_task=true;
                    }

                    if (Utils::fileExists("/lib/rdk/Start_uploadSTBLogs.sh")) {
                        exec_status=system("sh /lib/rdk/Start_uploadSTBLogs.sh &");
                        if (exec_status == E_OK) {
                            LOGINFO("Successfully executed lib/rdk/Start_uploadSTBLogs.sh \n");
                        }
                        else{
                            LOGINFO( "Failed to execute /lib/rdk/Start_uploadSTBLogs.sh \n");
                            skip_task=true;
                        }
                    }
                    else {
                        LOGINFO("Start_uploadSTBLogs.sh is not present \n");
                        skip_task=true;
                    }

                    if (Utils::fileExists("/lib/rdk/ping-telemetry.sh")) {
                        exec_status=system("sh /lib/rdk/ping-telemetry.sh &");
                        if (exec_status == E_OK) {
                            LOGINFO("Successfully executed lib/rdk/ping-telemetry.sh \n");
                        }
                        else{
                            LOGINFO( "Failed to execute /lib/rdk/ping-telemetry.sh \n");
                            skip_task=true;
                        }
                    }
                    else {
                        LOGINFO("ping-telemetry.sh is not present \n");
                        skip_task=true;
                    }

                    /* check the abort flag is set or not*/
                    abort_flag=m_setting.getValue("abort_flag").String();

                    LOGINFO("DBG: Abort_flag = %s\n", abort_flag.c_str());
                    if ( !(abort_flag.compare("false")) || ( abort_flag.empty() ) ) {
                        if (Utils::fileExists("/lib/rdk/deviceInitiatedFWDnld.sh")) {
                            LOGWARN("Checking for new Firmware\n");
                            exec_status=system("/lib/rdk/deviceInitiatedFWDnld.sh 0 4 >> /opt/logs/swupdate.log &");
                            if (exec_status == E_OK) {
                                LOGINFO("Successfully executed DIFD\n");
                            }
                            else{
                                LOGINFO( "Failed to execute DIFD \n" );
                                skip_task=true;
                            }
                        }
                        else {
                            LOGINFO("deviceInitiatedFWDnld is not present \n");
                            skip_task=true;
                        }
                    }
                    else {
                        LOGINFO("SKipping deviceInitiatedFWDnld due to Background Mode \n");
                        skip_task=true;
                    }

                    if (skip_task){
                        /* we set the corresponding flag to set
                         * which means one of then skipped */
                        SET_STATUS(g_task_status,TASK_SKIPPED);
                    }
                    result=true;
                    returnResponse(result);
                }

        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status_t status) {
            JsonObject params;
            /* we store the updated value as well */
            g_notify_status=status;
            params["maintenanceStatus"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }

   } /* namespace Plugin */
} /* namespace WPEFramework */
