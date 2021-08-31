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
#define SERVER_DETAILS  "127.0.0.1:9998"


#define PROC_DIR "/proc"
#define TR181_AUTOREBOOT_ENABLE "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"
#define TR181_STOP_MAINTENANCE  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.StopMaintenance.Enable"

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

bool checkValidOptOutModes(string OptoutModes){
    vector<string> modes{
        "ENFORCE_OPTOUT",
        "BYPASS_OPTOUT",
        "IGNORE_UPDATE",
        "NONE"
    };

    if ( find( modes.begin(), modes.end(), OptoutModes) != modes.end() ){
        return true;
    }
    else {
        return false;
    }
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
namespace WPEFramework {
    namespace Plugin {
        //Prototypes
        SERVICE_REGISTRATION(MaintenanceManager,API_VERSION_NUMBER_MAJOR,API_VERSION_NUMBER_MINOR);
        /* Global time variable */
        MaintenanceManager* MaintenanceManager::_instance = nullptr;

        cSettings MaintenanceManager::m_setting(MAINTENANCE_MGR_RECORD_FILE);
        //TODO  this need to moved to a seperate class and vector based.

        string task_names_foreground[]={
            "/lib/rdk/RFCbase.sh",
            "/lib/rdk/swupdate_utility.sh >> /opt/logs/swupdate.log",
            "/lib/rdk/Start_uploadSTBLogs.sh"
        };

        string script_names[]={
            "DCMscript_maintaince.sh",
            "RFCbase.sh",
            "deviceInitiatedFWDnld.sh",
            "uploadSTBLogs.sh"
        };

        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager()
            :AbstractPlugin()
        {
            MaintenanceManager::_instance = this;

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
            registerMethod("stopMaintenance", &MaintenanceManager::stopMaintenance,this);
            registerMethod("getMaintenanceMode", &MaintenanceManager::getMaintenanceMode,this);


            MaintenanceManager::m_task_map["/lib/rdk/StartDCM_maintaince.sh"]=false;
            MaintenanceManager::m_task_map[task_names_foreground[0].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[1].c_str()]=false;
            MaintenanceManager::m_task_map[task_names_foreground[2].c_str()]=false;


         }

        void MaintenanceManager::task_execution_thread(){
            int task_count=3;
            int i=0;
            string cmd="";

            LOGINFO("Reboot_Pending :%s",g_is_reboot_pending.c_str());

#if defined (SKY_BUILD)
            bool internetConnectStatus = true;
#else
            bool internetConnectStatus = isDeviceOnline();
#endif

            MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
            /*  In an unsolicited maintenance we make sure only after
             *  after DCM task activities are started.
             */

            /* we add the task in a loop */
            std::unique_lock<std::mutex> lck(m_callMutex);
            if (UNSOLICITED_MAINTENANCE == g_maintenance_type && internetConnectStatus){
                LOGINFO("---------------UNSOLICITED_MAINTENANCE--------------");
                for ( i=0;i< task_count ;i++ ){
                    task_thread.wait(lck);
                    cmd=task_names_foreground[i].c_str();
                    cmd+=" &";
                    cmd+="\0";
                    m_task_map[task_names_foreground[i].c_str()]=true;
                    LOGINFO("Starting Script (USM) :  %s \n", cmd.c_str());
                    if (!m_abort_flag){
                        system(cmd.c_str());
                    }
                }
            }
            /* Here in Solicited we start with RFC so no
             * need to wait for any DCM events */
            else if( SOLICITED_MAINTENANCE == g_maintenance_type && internetConnectStatus){
                    LOGINFO("=============SOLICITED_MAINTENANCE===============");
                    cmd=task_names_foreground[0].c_str();
                    cmd+=" &";
                    cmd+="\0";
                    m_task_map[task_names_foreground[0].c_str()]=true;
                    LOGINFO("Starting Script (SM) :  %s \n", cmd.c_str());
                    system(cmd.c_str());
                    cmd="";
                    for (i=1;i<task_count;i++){
                        task_thread.wait(lck);
                        cmd=task_names_foreground[i].c_str();
                        cmd+=" &";
                        cmd+="\0";
                        m_task_map[task_names_foreground[i].c_str()]=true;
                        LOGINFO("Starting Script (SM) :  %s \n", cmd.c_str());
                        if (!m_abort_flag){
                            system(cmd.c_str());
                        }
                    }
            }

            m_abort_flag=false;
            LOGINFO("Worker Thread Completed");
            if (false == internetConnectStatus) {
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
                LOGINFO("Maintenance completed as it is offline mode");
            }
        }

        bool MaintenanceManager::isDeviceOnline()
        {
            LOGINFO("Checking device has network connectivity\n");

            if (false == Utils::isPluginActivated("org.rdk.Network")) {
               sleep(30);
               if (false == Utils::isPluginActivated("org.rdk.Network")) {
                   LOGINFO("Network plugin is not activated and considered as offline\n");
                   return false;
               }
            }

            JsonObject joGetParams;
            JsonObject joGetResult;
            std::string callsign = "org.rdk.Network.1";
	    std::string token;
            Utils::SecurityToken::getSecurityToken(token);

            string query = "token=" + token;
            Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), _T(SERVER_DETAILS));
            auto thunder_client = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >(callsign.c_str(), "");
            if (thunder_client != nullptr) {
                uint32_t status = thunder_client->Invoke<JsonObject, JsonObject>(5000, "isConnectedToInternet", joGetParams, joGetResult);
                if (status > 0) {
                    LOGINFO("%s call failed %d", callsign.c_str(), status);
                    return false;
                } else if (joGetResult.HasLabel("connectedToInternet")) {
                    LOGINFO("connectedToInternet status %d", joGetResult["connectedToInternet"].Boolean());
                    return joGetResult["connectedToInternet"].Boolean();
                } else {
                    return false;
                }
	    }
            LOGINFO("thunder client failed");
            return false;
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
                IARM_Result_t res;
                // Register for the Maintenance Notification Events
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                //Register for setMaintenanceStartTime
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT,_MaintenanceMgrEventHandler));

                maintenanceManagerOnBootup();
            }
        }

        void MaintenanceManager::maintenanceManagerOnBootup() {
            /* on boot up we set these things */
            MaintenanceManager::g_currentMode = FOREGROUND_MODE;

            MaintenanceManager::m_notify_status=MAINTENANCE_IDLE;
            MaintenanceManager::g_epoch_time="";

            /* to know the maintenance is solicited or unsolicited */
            g_maintenance_type=UNSOLICITED_MAINTENANCE;

            /* On bootup we check for opt-out value
             * if empty set the value to none */
            string OptOutmode = "NONE";
            OptOutmode = m_setting.getValue("softwareoptout").String();
            if(!checkValidOptOutModes(OptOutmode)){
                LOGINFO("OptOut Value is not Set. Setting to NONE \n");
                m_setting.remove("softwareoptout");
                OptOutmode = "NONE";
                m_setting.setValue("softwareoptout",OptOutmode);
            }
            else {
                LOGINFO("OptOut Value Found as: %s \n", OptOutmode.c_str());
            }

            MaintenanceManager::g_is_critical_maintenance="false";
            MaintenanceManager::g_is_reboot_pending="false";
            MaintenanceManager::g_lastSuccessful_maint_time="";
            MaintenanceManager::g_task_status=0;
            MaintenanceManager::m_abort_flag=false;

            /* we post just to tell that we are in idle at this moment */
            MaintenanceManager::_instance->onMaintenanceStatusChange(m_notify_status);

            int32_t exec_status=E_NOK;

            /* we call dcmscript to get the new start time */
            if (Utils::fileExists("/lib/rdk/StartDCM_maintaince.sh")) {
                exec_status = system("/lib/rdk/StartDCM_maintaince.sh &");
                if ( E_OK == exec_status ){
                    LOGINFO("DBG:Succesfully executed StartDCM_maintaince.sh \n");
                    m_task_map["/lib/rdk/StartDCM_maintaince.sh"]=true;
                }
                else {
                    LOGINFO("DBG:Failed to execute StartDCM_maintaince.sh !! \n");
                }
            }
            else {
                LOGINFO("DBG: Unable to find StartDCM_maintaince.sh \n");
            }

            /* we moved every thing to a thread */
            /* only when dcm is getting a DCM_SUCCESS/DCM_ERROR we say
             * Maintenance is started until then we say MAITENANCE_IDLE */
            if(m_thread.joinable()){
                     m_thread.join();
             }

            m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);
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
            Maint_notify_status_t notify_status=MAINTENANCE_STARTED;
            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status_t module_status;
            time_t successfulTime;
            string str_successfulTime="";
            auto task_status_DCM=m_task_map.find("/lib/rdk/StartDCM_maintaince.sh");
            auto task_status_RFC=m_task_map.find(task_names_foreground[0].c_str());
            auto task_status_FWDLD=m_task_map.find(task_names_foreground[1].c_str());
            auto task_status_LOGUPLD=m_task_map.find(task_names_foreground[2].c_str());

            IARM_Bus_MaintMGR_EventId_t event = (IARM_Bus_MaintMGR_EventId_t)eventId;
            LOGINFO("Maintenance Event-ID = %d \n",event);

            if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME)) {
                if ( IARM_BUS_DCM_NEW_START_TIME_EVENT == eventId ) {
                    /* we got a new start time from DCM script */
                    string l_time(module_event_data->data.startTimeUpdate.start_time);
                    LOGINFO("DCM_NEW_START_TIME_EVENT Start Time %s \n", l_time.c_str());
                    /* Store it in a Global structure */
                    g_epoch_time=l_time;
                }
                else if (( IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE == eventId ) &&
                    ( MAINTENANCE_STARTED == m_notify_status )) {
                    module_status = module_event_data->data.maintenance_module_status.status;
                    LOGINFO("MaintMGR Status %d \n",module_status);
                    string status_string=moduleStatusToString(module_status);
                    LOGINFO("MaintMGR Status %s \n", status_string.c_str());
                    switch (module_status) {
                        case MAINT_RFC_COMPLETE :
                            if(task_status_RFC->second != true) {
                                 LOGINFO("Ignoring Event RFC_COMPLETE");
                                 return;
                            }
                            else {
                                 SET_STATUS(g_task_status,RFC_SUCCESS);
                                 SET_STATUS(g_task_status,RFC_COMPLETE);
                                 task_thread.notify_one();
                                 m_task_map[task_names_foreground[0].c_str()]=false;
                            }
                            break;
                        case MAINT_DCM_COMPLETE :
                            if(task_status_DCM->second != true) {
                                 LOGINFO("Ignoring Event DCM_COMPLETE");
                                 return;
                            }
                            else {
                                SET_STATUS(g_task_status,DCM_SUCCESS);
                                SET_STATUS(g_task_status,DCM_COMPLETE);
                                task_thread.notify_one();
                                m_task_map["/lib/rdk/StartDCM_maintaince.sh"]=false;
                            }
                            break;
                        case MAINT_FWDOWNLOAD_COMPLETE :
                            if(task_status_FWDLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_COMPLETE");
                                 return;
                            }
                            else {
                                SET_STATUS(g_task_status,DIFD_SUCCESS);
                                SET_STATUS(g_task_status,DIFD_COMPLETE);
                                task_thread.notify_one();
                                m_task_map[task_names_foreground[1].c_str()]=false;
                            }
                            break;
                       case MAINT_LOGUPLOAD_COMPLETE :
                            if(task_status_LOGUPLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_LOGUPLOAD_COMPLETE");
                                 return;
                            }
                            else {
                                SET_STATUS(g_task_status,LOGUPLOAD_SUCCESS);
                                SET_STATUS(g_task_status,LOGUPLOAD_COMPLETE);
                                m_task_map[task_names_foreground[2].c_str()]=false;
                            }

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
                            /* we say FW update task complete */
                            SET_STATUS(g_task_status,DIFD_COMPLETE);
                            task_thread.notify_one();
                            m_task_map[task_names_foreground[1].c_str()]=false;
                            LOGINFO("FW Download task aborted \n");
                            break;
                        case MAINT_DCM_ERROR:
                            if(task_status_DCM->second != true) {
                                 LOGINFO("Ignoring Event DCM_ERROR");
                            }
                            else {
                                SET_STATUS(g_task_status,DCM_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in DCM script task \n");
                                m_task_map["/lib/rdk/StartDCM_maintaince.sh"]=false;
                            }
                            break;
                        case MAINT_RFC_ERROR:
                            if(task_status_RFC->second != true) {
                                 LOGINFO("Ignoring Event RFC_ERROR");
                                 return;
                            }
                            else {
                                 SET_STATUS(g_task_status,RFC_COMPLETE);
                                 task_thread.notify_one();
                                 LOGINFO("Error encountered in RFC script task \n");
                                 m_task_map[task_names_foreground[0].c_str()]=false;
                            }

                            break;
                        case MAINT_LOGUPLOAD_ERROR:
                            if(task_status_LOGUPLD->second != true) {
                                  LOGINFO("Ignoring Event MAINT_LOGUPLOAD_ERROR");
                                  return;
                            }
                            else {
                                SET_STATUS(g_task_status,LOGUPLOAD_COMPLETE);
                                LOGINFO("Error encountered in LOGUPLOAD script task \n");
                                m_task_map[task_names_foreground[2].c_str()]=false;
                            }

                            break;
                       case MAINT_FWDOWNLOAD_ERROR:
                            if(task_status_FWDLD->second != true) {
                                 LOGINFO("Ignoring Event MAINT_FWDOWNLOAD_ERROR");
                                 return;
                            }
                            else {
                                SET_STATUS(g_task_status,DIFD_COMPLETE);
                                task_thread.notify_one();
                                LOGINFO("Error encountered in SWUPDATE script task \n");
                                m_task_map[task_names_foreground[1].c_str()]=false;
                            }
                            break;
                       case MAINT_DCM_INPROGRESS:
                            m_task_map["/lib/rdk/StartDCM_maintaince.sh"]=true;
                            /*will be set to false once COMEPLETE/ERROR received for DCM*/
                            LOGINFO(" DCM already IN PROGRESS -> setting m_task_map of DCM to true \n");
                            break;
                       case MAINT_RFC_INPROGRESS:
                            m_task_map[task_names_foreground[0].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for RFC*/
                            LOGINFO(" RFC already IN PROGRESS -> setting m_task_map of RFC to true \n");
                            break;
                       case MAINT_FWDOWNLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[1].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for FWDOWNLOAD*/
                            LOGINFO(" FWDOWNLOAD already IN PROGRESS -> setting m_task_map of FWDOWNLOAD to true \n");
                            break;
                       case MAINT_LOGUPLOAD_INPROGRESS:
                            m_task_map[task_names_foreground[2].c_str()]=true;
                            /*will be set to false once COMEPLETE/ERROR received for LOGUPLOAD*/
                            LOGINFO(" LOGUPLOAD already IN PROGRESS -> setting m_task_map of LOGUPLOAD to true \n");
                            break;
                    }
                }
                else{
                    LOGINFO("Ignoring/Unknown Maintenance Status!!");
                    return;
                }

                LOGINFO(" BITFIELD Status : %x",g_task_status);
                /* Send the updated status only if all task completes execution
                 * until that we say maintenance started */
                if ( (g_task_status & TASKS_COMPLETED ) == TASKS_COMPLETED ){
                    if ( (g_task_status & ALL_TASKS_SUCCESS) == ALL_TASKS_SUCCESS ){ // all tasks success
                        LOGINFO("DBG:Maintenance Successfully Completed!!");
                        notify_status=MAINTENANCE_COMPLETE;
                        /*  we store the time in persistant location */
                        successfulTime=time(nullptr);
                        tm ltime=*localtime(&successfulTime);
                        time_t epoch_time=mktime(&ltime);
                        str_successfulTime=to_string(epoch_time);
                        LOGINFO("last succesful time is :%s", str_successfulTime.c_str());
                        /* Remove any old completion time */
                        m_setting.remove("LastSuccessfulCompletionTime");
                        m_setting.setValue("LastSuccessfulCompletionTime",str_successfulTime);

                    }
                    /* Check other than all success case which means we have errors */
                    else if ((g_task_status & ALL_TASKS_SUCCESS)!= ALL_TASKS_SUCCESS) {
                        if ((g_task_status & MAINTENANCE_TASK_SKIPPED ) == MAINTENANCE_TASK_SKIPPED ){
                            LOGINFO("DBG:There are Skipped Task. Maintenance Incomplete");
                            notify_status=MAINTENANCE_INCOMPLETE;
                        }
                        else {
                            LOGINFO("DBG:Maintenance Ended with Errors");
                            notify_status=MAINTENANCE_ERROR;
                        }

                    }

                    LOGINFO("ENDING MAINTENANCE CYCLE");
                    if(m_thread.joinable()){
                        m_thread.join();
                    }

                    MaintenanceManager::_instance->onMaintenanceStatusChange(notify_status);
                }
                else {
                    LOGINFO("Tasks are not completed!!!!");

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
            }

            if(m_thread.joinable()){
                m_thread.join();
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
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceActivityStatus",
         *                  "params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"status":MAINTENANCE_IDLE,"LastSuccessfulCompletionTime":
                              -1,"isCriticalMaintenance":true,"isRebootPending":true}}
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
                    bool b_criticalMaintenace=false;
                    bool b_rebootPending=false;

                    std::lock_guard<std::mutex> guard(m_callMutex);

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

                    if (!isCriticalMaintenance.compare("true")){
                        b_criticalMaintenace=true;
                    }

                    if(!isRebootPending.compare("true")){
                        b_rebootPending=true;
                    }

                    response["maintenanceStatus"] = notifyStatusToString(m_notify_status);
                    if(strcmp("NA",LastSuccessfulCompletionTime.c_str())==0)
                    {
                       response["LastSuccessfulCompletionTime"] = 0;  // stoi is not able handle "NA"
                    }
                    else
                    {
                       try{
                               response["LastSuccessfulCompletionTime"] = stoi(LastSuccessfulCompletionTime.c_str());
                          }
                       catch(exception &err)
                          {
                              //exception caught with stoi -- So making "LastSuccessfulCompletionTime" as 0
                              response["LastSuccessfulCompletionTime"] = 0;
                          }
                    }
                    response["isCriticalMaintenance"] = b_criticalMaintenace;
                    response["isRebootPending"] = b_rebootPending;
                    result = true;

                    returnResponse(result);
                }
        /*
         * @brief This function returns the start time of the maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceStartTime","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceStartTime":12345678,"success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceStartTime (const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string starttime="";
            unsigned long int start_time=0;

            starttime = Utils::cRunScript("/lib/rdk/getMaintenanceStartTime.sh &");
            if (!starttime.empty()){
                  response["maintenanceStartTime"]=stoi(starttime.c_str());
                  result=true;
            }

            returnResponse(result);
        }

        /*
         * @brief This function returns Mode of the maintenance.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.getMaintenanceMode","params":{}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"maintenanceMode":"FOREGROUND","optOut":"IGNORE_UPDATE","success":true}}
         * @return: Core::<StatusCode>
         */
        uint32_t MaintenanceManager::getMaintenanceMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string softwareOptOutmode = "NONE";
            if ( BACKGROUND_MODE != g_currentMode && FOREGROUND_MODE != g_currentMode )  {
                LOGERR("Didnt get a valid Mode. Failed\n");
                returnResponse(false);
            }
            else {
                response["maintenanceMode"] = g_currentMode;

                if ( Utils::fileExists(MAINTENANCE_MGR_RECORD_FILE) ){
                    if ( parseConfigFile(MAINTENANCE_MGR_RECORD_FILE,"softwareoptout",softwareOptOutmode)){
                        /* check if the value is valid */
                        if(!checkValidOptOutModes(softwareOptOutmode)){
                            LOGERR("OptOut Value Corrupted. Failed\n");
                            returnResponse(false);
                        }
                        else {
                            LOGINFO("OptOut Value = %s",softwareOptOutmode.c_str());
                        }
                    }
                    else {
                        LOGERR("OptOut Value Not Found. Failed\n");
                        returnResponse(false);
                    }
                }
                else {
                    LOGERR("OptOut Config File Not Found. Failed\n");
                    returnResponse(false);
                }
                response["optOut"] = softwareOptOutmode.c_str();
                result = true;
            }
            returnResponse(result);
        }


        /*
         * @brief This function returns the current status of the current
         * or previous maintenance activity.
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.setMaintenanceMode",
         *                  "params":{"maintenanceMode":FOREGROUND}}''
         * @param2[out]: {"jsonrpc":"2.0","id":3,"result":{"success":<bool>}}
         * @return: Core::<StatusCode>
         */

        uint32_t MaintenanceManager::setMaintenanceMode(const JsonObject& parameters,
                JsonObject& response)
        {
            bool result = false;
            string new_mode = "";
            string old_mode = g_currentMode;
            string bg_flag = "false";
            string new_optout_state = "";

            /* Label should have maintenance mode and softwareOptout field */
            if ( parameters.HasLabel("maintenanceMode") && parameters.HasLabel("optOut") ){

                new_mode = parameters["maintenanceMode"].String();

                std::lock_guard<std::mutex> guard(m_callMutex);

                /* check if maintenance is on progress or not */
                /* if in progress restrict the same */
                if ( MAINTENANCE_STARTED != m_notify_status ){

                    LOGINFO("SetMaintenanceMode new_mode = %s\n",new_mode.c_str());

                    if ( BACKGROUND_MODE != new_mode && FOREGROUND_MODE != new_mode )  {
                        LOGERR("value of new mode is incorrect, therefore \
                                current mode '%s' not changed.\n", old_mode.c_str());
                        returnResponse(false);
                    }
                    /* remove any older one */
                    m_setting.remove("background_flag");
                    if ( BACKGROUND_MODE == new_mode ) {
                        bg_flag = "true";
                    }
                    else {
                        /* foreground */
                        bg_flag = "false";
                    }
                    g_currentMode = new_mode;
                    m_setting.setValue("background_flag", bg_flag);
                }
                else{
                    LOGERR("Maintenance is in Progress, Mode change not allowed");
                    result =true;
                }

                /* OptOut changes here */
                new_optout_state = parameters["optOut"].String();

                LOGINFO("SetMaintenanceMode optOut = %s\n",new_optout_state.c_str());

                /* check if we have a valid state from user */
                if(checkValidOptOutModes(new_optout_state)){
                    /* we got a valid state; Now store it in persistant location */
                    m_setting.setValue("softwareoptout",new_optout_state);
                }
                else{
                    LOGINFO("Invalid optOut = %s\n",new_optout_state.c_str());
                    returnResponse(false);
                }

                /* Set the result as true */
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
         * @param1[in]: {"jsonrpc":"2.0","id":"3","method":"org.rdk.MaintenanceManager.1.startMaintenance",
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

                    /* only one maintenance at a time */
                    /* Lock so that m_notify_status will not be updated  further */
                    m_statusMutex.lock();
                    if ( MAINTENANCE_STARTED != m_notify_status  ){

                        /*reset the status to 0*/
                        g_task_status=0;
                        g_maintenance_type=SOLICITED_MAINTENANCE;

                        m_abort_flag=false;

                        /* we dont touch the dcm so
                         * we say DCM is success and complete */
                        SET_STATUS(g_task_status,DCM_SUCCESS);
                        SET_STATUS(g_task_status,DCM_COMPLETE);

                        /* isRebootPending will be set to true
                         * irrespective of XConf configuration */
                        g_is_reboot_pending="true";

                        /* we set this to false */
                        g_is_critical_maintenance="false";

                        if(m_thread.joinable()){
                               m_thread.join();
                        }

                        m_thread = std::thread(&MaintenanceManager::task_execution_thread, _instance);

                        result=true;
                    }
                    else {
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

        uint32_t MaintenanceManager::stopMaintenance(const JsonObject& parameters,
                JsonObject& response){

            pid_t pid_num=-1;

            int k_ret=EINVAL;
            int i=0,record=-1;
            int32_t exec_status=E_NOK;

            bool task_status[4]={false};
            bool result=false;
            bool task_incomplete=false;

            /* only based on RFC */
            if( checkAbortFlag() ){

                /* run only when the maintenance status is MAINTENANCE_STARTED */
                m_statusMutex.lock();
                if ( MAINTENANCE_STARTED == m_notify_status  ){

                    // Set the condition flag m_abort_flag to true
                    m_abort_flag = true;

                    auto task_status_DCM=m_task_map.find("/lib/rdk/StartDCM_maintaince.sh");
                    auto task_status_RFC=m_task_map.find(task_names_foreground[0].c_str());
                    auto task_status_FWDLD=m_task_map.find(task_names_foreground[1].c_str());
                    auto task_status_LOGUPLD=m_task_map.find(task_names_foreground[2].c_str());

                    task_status[0] = task_status_DCM->second;
                    task_status[1] = task_status_RFC->second;
                    task_status[2] = task_status_FWDLD->second;
                    task_status[3] = task_status_LOGUPLD->second;

                    for (i=0;i<4;i++)
                        LOGINFO("task status [%d]  = %s ScriptName %s",i,(task_status[i])? "true":"false",script_names[i].c_str());
                    for (i=0;i<4;i++){
                        if(task_status[i]){
                            record = i;
                            LOGINFO("Checking the Task PID\n");
                            pid_num=getTaskPID(script_names[i].c_str());
                            LOGINFO("PID of script_name [%d] = %s is %d \n", i,script_names[i].c_str(),pid_num);
                            if( pid_num != -1){
                                /* send the signal to task to terminate */
                                k_ret=kill(pid_num,SIGABRT);
                                if (k_ret == 0){
                                    LOGINFO(" %s Termimated\n",script_names[i].c_str());
                                    /*this means we killed the task currently running */
                                    task_incomplete = true;
                                }
                                else{
                                    LOGINFO("Failed to terminate with error %d \n",script_names[i].c_str(),k_ret);
                                }
                            }
                            else {
                                LOGINFO("Didnt find PID for %s\n",script_names[i].c_str());
                            }

                            /* No need to loop again */
                            break;
                        }
                        else{
                            LOGINFO("Task[%d] is false \n",i);
                        }
                    }

                    /* if we still didnt get the pid but we still know which task is running */
                    if ( !task_incomplete ){

                        char cmd[128] = {'\0'};
                        if (Utils::fileExists("/lib/rdk/maintenanceTrapEventNotifier.sh")){
                            /* send the arg to the trap notifier */
                            snprintf(cmd, 127, "/lib/rdk/maintenanceTrapEventNotifier.sh %i &", record);
                            exec_status=system(cmd);
                            if ( E_OK == exec_status ){
                                LOGINFO("DBG:Succesfully executed maintenanceTrapEventNotifier.sh \n");
                            }
                            else{
                                LOGERR("Failed to execute maintenanceTrapEventNotifier.sh \n");
                            }
                        }
                        else {
                            LOGINFO("Failed to locate maintenanceTrapEventNotifier.sh \n");
                        }
                    }
                    result=true;
                }
                else {
                    LOGERR("Failed to stopMaintenance without starting maintenance \n");
                }
                m_statusMutex.unlock();
            }
            else {
                LOGERR("Failed to initiate stopMaintenance, RFC is set as False \n");
            }
            returnResponse(result);
        }

        bool MaintenanceManager::checkAbortFlag(){
            bool ret=false;
            RFC_ParamData_t param;
            WDMP_STATUS wdmpStatus = getRFCParameter(const_cast<char *>("MaintenanceManager"),TR181_STOP_MAINTENANCE, &param);
            if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE){
                if( param.type == WDMP_BOOLEAN ){
                    if(strncasecmp(param.value,"true",4) == 0 ){
                        ret=true;
                    }
                }
            }
            LOGINFO(" StopMaintenance.Enable = %s , call value %d ", (ret == true)?"true":"false", wdmpStatus);
            return ret;
        }

        /* Helper function to find the Script/Task PID*/
        pid_t MaintenanceManager::getTaskPID(const char* taskname){

            DIR* dir=opendir(PROC_DIR);
            struct dirent* ent;
            char* endptr;
            char buf[512];
            char *ch =0;

            while((ent = readdir(dir)) != NULL) {
                long lpid = strtol(ent->d_name, &endptr, 10);
                if (*endptr != '\0') {
                    continue;
                }
                /* Get the PID */
                snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);

                /* Open the cmdline and read */
                FILE* fp = fopen(buf, "r");
                if (fp) {
                    char *arg = 0;
                    size_t size = 0;
                    while(getdelim(&arg, &size, 0,fp) != -1){
                        printf("%s\n",arg);
                        char* first = strstr(arg, taskname);
                        if (first != NULL){
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
            return -1;
        }

        void MaintenanceManager::onMaintenanceStatusChange(Maint_notify_status_t status) {
            JsonObject params;
            /* we store the updated value as well */
            m_statusMutex.lock();
            m_notify_status=status;
            m_statusMutex.unlock();
            params["maintenanceStatus"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }

   } /* namespace Plugin */
} /* namespace WPEFramework */
