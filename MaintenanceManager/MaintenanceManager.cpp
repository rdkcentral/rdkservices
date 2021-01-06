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

const short WPEFramework::Plugin::MaintenanceManager::API_VERSION_NUMBER_MAJOR = 2;
const short WPEFramework::Plugin::MaintenanceManager::API_VERSION_NUMBER_MINOR = 0;

#define MAX_REBOOT_DELAY 86400 /* 24Hr = 86400 sec */
#define TR181_FW_DELAY_REBOOT "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"
#define TR181_AUTOREBOOT_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"

string notifyStatusToString(Maint_notify_status &status)
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

string moduleStatusToString(IARM_Maint_module_status &status)
{
    string ret_status="";
    switch(status){
        case MAINT_DCM_COMPLETE:
            ret_status="MAINTENANCE_DCM_ERROR";
            break;
        case MAINT_DCM_ERROR:
            ret_status="MAINTENANCE_DCM_COMPLETE";
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
        SERVICE_REGISTRATION(MaintenanceManager,MaintenanceManager::API_VERSION_NUMBER_MAJOR,MaintenanceManager::API_VERSION_NUMBER_MINOR);

        MaintenanceManager* MaintenanceManager::_instance = nullptr;
        Maint_notify_status MaintenanceManager::g_notify_status=MAINTENANCE_IDLE;

        std::string MaintenanceManager::g_currentMode = "";
        IARM_Bus_MaintMGR_EventData_t* MaintenanceManager::g_maintenance_data = nullptr;
        cSettings MaintenanceManager::m_setting(MAITNENANCE_MGR_RECORD_FILE);
        string MaintenanceManager::g_is_critical_maintenance="";
        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager()
            :AbstractPlugin(MaintenanceManager::API_VERSION_NUMBER_MAJOR)
        {
            MaintenanceManager::_instance = this;
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;
            /**
             * @brief Invoking Plugin API register to WPEFRAMEWORK.
             */
#ifdef DEBUG
            registerMethod("sampleMaintenanceManagerAPI", &MaintenanceManager::sampleAPI, this);
#endif /* DEBUG */
            registerMethod("GetMaintenanceActivityStatus", &MaintenanceManager::GetMaintenanceActivityStatus,this);
            registerMethod("GetMaintenanceStartTime", &MaintenanceManager::GetMaintenanceStartTime,this);
            registerMethod("SetMaintenanceMode", &MaintenanceManager::SetMaintenanceMode,this);
            registerMethod("StartMaintenance", &MaintenanceManager::StartMaintenance,this);
  // version 2 APIs
            registerMethod("fireFirmwarePendingReboot", &MaintenanceManager::fireFirmwarePendingReboot, this, {2});
            registerMethod("setFirmwareRebootDelay", &MaintenanceManager::setFirmwareRebootDelay, this, {2});
            registerMethod("setFirmwareAutoReboot", &MaintenanceManager::setFirmwareAutoReboot, this, {2});
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
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_DCM_EVENT_UPDATE,_MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_RFC_EVENT_UPDATE,_MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_LOGUPLOAD_EVENT_UPDATE,_MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_PING_TELEMETRY_EVENT_UPDATE,_MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_DIFWDOWNLOAD_EVENT_UPDATE,_MaintenanceMgrEventHandler));
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_CRITICAL_UPDATE,_MaintenanceMgrEventHandler));
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
            Maint_notify_status m_notify_status;

            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status module_status;
            LOGINFO("Event-ID = %d \n",eventId);

            if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME)) {
                if (eventId == IARM_BUS_DCM_NEW_START_TIME_EVENT) {
                    /* we got a new start time from DCM script */
                    unsigned int hour = module_event_data->data.start_time.hr;
                    unsigned int minutes = module_event_data->data.start_time.min;
                    LOGINFO("MaintMGR Hour = %d, Min = %d \n", hour, minutes);
                    LOGINFO("Event-ID = %d \n",eventId);

                    /* Store it in a Global structure */
                    g_maintenance_data->data.start_time.hr = hour;
                    g_maintenance_data->data.start_time.min = minutes;
                    return;
                }
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_CRITICAL_UPDATE ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                    g_is_critical_maintenance="true";
                }
                    /* handle notification from DCMScript.sh */
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_DCM_EVENT_UPDATE ) {
                    /* we got a new notification */
                    module_status = module_event_data->data.maintenance_module_status.status;
                }
                    /* handle nofification from rfcbase.sh */
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_RFC_EVENT_UPDATE ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                }
                    /* handle nofitication from UploadSTBLogs.sh */
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_LOGUPLOAD_EVENT_UPDATE ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                }
                    /* handle notification from Ping_Telemetry.sh */
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_PING_TELEMETRY_EVENT_UPDATE ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                }
                    /* handle notification from DeviceInitiatedFWDownload.sh */
                else if ( eventId == IARM_BUS_MAINTENANCEMGR_DIFWDOWNLOAD_EVENT_UPDATE ) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                }
                LOGINFO("MaintMGR Status %s \n",moduleStatusToString(module_status));
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
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_DCM_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_RFC_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_LOGUPLOAD_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_PING_TELEMETRY_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_DIFWDOWNLOAD_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_CRITICAL_UPDATE));

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

        uint32_t MaintenanceManager::GetMaintenanceActivityStatus(const JsonObject& parameters,
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
        uint32_t MaintenanceManager::GetMaintenanceStartTime (const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    int hour = g_maintenance_data->data.start_time.hr;
                    int minutes = g_maintenance_data->data.start_time.min;
                    string start_time="";
                    string old_time="";

                    /*  check the global structure */
                    if ( hour == -1 && minutes == -1 ) {
                        /* we set a default time say 0300 Hrs until
                         * we receive a valid one */
                        hour = 3;
                        minutes = 0;
                    }

                    string new_time_hr=to_string(hour);
                    string new_time_min=to_string(minutes);

                    if ( (hour <= 23 && hour >= 0 ) && ( minutes >= 0 && minutes <=59 )) {
                        /*we may need to prefix the 0 as 01-09 */
                        if ( !(hour/10 ) ) {
                            old_time=to_string(hour);
                            new_time_hr=(2 - old_time.length(), '0') + old_time;
                        }

                        if ( !(minutes/10) ) {
                            old_time=to_string(minutes);
                            new_time_min= (2 - old_time.length(), '0') + old_time;
                        }
                        result=true;
                    }
                    start_time= new_time_hr + ":" + new_time_min;

                    response["time"] = start_time.c_str();
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

        uint32_t MaintenanceManager::SetMaintenanceMode(const JsonObject& parameters,
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

        uint32_t MaintenanceManager::StartMaintenance(const JsonObject& parameters,
                JsonObject& response)
                {
                    bool result = false;
                    Maint_notify_status notify_status = MAINTENANCE_IDLE;
                    /* check what mode we currently have */
                    string current_mode="";

                    /* check if there is any critical maintenance happening */
                    if (!(g_is_critical_maintenance.compare("true"))){
                        /* then run in FG itself */
                        g_currentMode=FOREGROUND_MODE;
                        LOGINFO("Critical Maintenance is True. Running in Foreground \n");
                    }
                    current_mode=g_currentMode;

                    if( FOREGROUND_MODE == current_mode )
                    {
                        /* notify that we started the maintenance*/
                        onMaintenanceStatusChange(MAINTENANCE_STARTED);
                        /* we start the DCMScript.sh first as a part of Maintenance */
                        if (Utils::fileExists("/lib/rdk/StartDCM.sh")) {
                            string dcmscript_command("sh /lib/rdk/StartDCM.sh &");
                            LOGINFO("executing %s\n", dcmscript_command.c_str());
                            Utils::cRunScript(dcmscript_command.c_str());
                            result=true;
                        }
                        else {
                            LOGINFO("StartDCM.sh is not present \n");
                        }

                        if (Utils::fileExists("/lib/rdk/RFCbase.sh")) {
                            string rfcscript_command("sh /lib/rdk/RFCbase.sh &");
                            LOGINFO("executing %s\n", rfcscript_command.c_str());
                            Utils::cRunScript(rfcscript_command.c_str());
                            result=true;
                        }
                        else {
                            LOGINFO("RFCbase.sh is not present \n");
                        }
                    }
                    returnResponse(result);
                }

        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status status) {
            JsonObject params;
            /* we store the updated value as well */
            g_notify_status=status;
            params["onMaintenanceStatusChange"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }

        /*
         * @brief This function delays the reboot in seconds.
         * This will internally sets the tr181 fwDelayReboot parameter.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.setFirmwareRebootDelay",
         *                  "params":{"delaySeconds": int seconds}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::setFirmwareRebootDelay(const JsonObject& parameters,
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
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.setFirmwareAutoReboot",
         *                  "params":{"enable": bool }}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::setFirmwareAutoReboot(const JsonObject& parameters,
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
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.2.fireFirmwarePendingReboot",
         *                  "params":{}}
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":true}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::fireFirmwarePendingReboot(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            int seconds = 120; /* 2 Minutes to Reboot */

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

        void MaintenanceManager::onFirmwarePendingReboot(int seconds)
        {
            JsonObject params;
            params["fireFirmwarePendingReboot"] = seconds;
            LOGINFO("Notifying onFirmwarePendingReboot received \n");
            sendNotify(EVT_ONFWPENDINGREBOOT, params);
        }

   } /* namespace Plugin */
} /* namespace WPEFramework */
