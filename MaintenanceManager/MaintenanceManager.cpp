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
#include <list>
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

string notifyStatusToString(MAINTENANCE_STATUS &status)
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
            ret_status="MAINTENANCE_UNKOWN_EVENT";
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
        MaintenanceManager* MaintenanceManager::_instance = nullptr;
        cSettings MaintenanceManager::m_setting(MAINTENANCE_MGR_RECORD_FILE);

        /**
         * Register MaintenanceManager module as wpeframework plugin
         */
        MaintenanceManager::MaintenanceManager():AbstractPlugin(){
            MaintenanceManager::_instance = this;
            maintenanceInProgress = false;
            stopMaintenanceFlag = false;
            maintenanceType = UNSOLICITED_MAINTENANCE;
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
#ifndef DEBUG
            registerMethod("stopMaintenance", &MaintenanceManager::stopMaintenance,this);
#endif
        }

        MaintenanceManager::~MaintenanceManager()
        {
            MaintenanceManager::_instance = nullptr;
        }

        const string MaintenanceManager::Initialize(PluginHost::IShell*)
        {
        	std::lock_guard<std::mutex> guard(apiMutex);
            runMaintenance(UNSOLICITED_MAINTENANCE);
            /* On Success; return empty to indicate no error text. */
            return (string());
        }

        void MaintenanceManager::Deinitialize(PluginHost::IShell*)
        {
        }

        bool MaintenanceManager::InitializeIARM()
        {
        	IARM_Result_t res;
            if (Utils::IARM::init()) {
                // Register for the Maintenance Notification Events
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _MaintenanceMgrEventHandler));
                //Register for setMaintenanceStartTime
                IARM_CHECK(IARM_Bus_RegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT,_MaintenanceMgrEventHandler));
                LOGINFO("DBG:Succesfully registered with IARM \n");
                return true;
            }
            else
            {
                LOGINFO("DBG:Failed to register with IARM \n");
                return false;
            }
        }
        void MaintenanceManager::DeinitializeIARM()
        {
        	IARM_Result_t res;
            if (Utils::IARM::isConnected()){
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE));
                IARM_CHECK(IARM_Bus_UnRegisterEventHandler(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT));
            }
        }

        MAINTENANCE_STATUS MaintenanceManager::getMaintenanceCompletionStatus(){
            for (auto taskIT = taskList.begin(); taskIT != taskList.end(); taskIT++) {
                if((*taskIT)->getStatus() == TASK_INCOMPLETE)
                    return MAINTENANCE_INCOMPLETE;
                else if((*taskIT)->getStatus() != TASK_COMPLETE)
                    return MAINTENANCE_ERROR;
            }
            return MAINTENANCE_COMPLETE;
        }

        void MaintenanceManager::task_execution_thread(){
            LOGINFO("INSIDE task execution thread");

            if(!InitializeIARM())
            {
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
                return;
            }
            for (auto taskIT = taskList.begin(); !stopMaintenanceFlag && taskIT != taskList.end(); taskIT++) {
                std::unique_lock<std::mutex> lck(taskThreadMutex);
                if((*taskIT)->startTask())
                {
                	LOGINFO("Waiting for Task: %s completion", (*taskIT)->getTaskName().c_str());
                    taskThreadCV.wait(lck);
                    LOGINFO("Task :%s execution complete", (*taskIT)->getTaskName().c_str());
                }
                else
                {
                    LOGINFO("Task :%s skipped", (*taskIT)->getTaskName().c_str());
                }
            }
            if(stopMaintenanceFlag)
            	LOGINFO("stopMaintenanceFlag is set - existing from running maintenance tasks\n");
            MAINTENANCE_STATUS maintStatus = getMaintenanceCompletionStatus();

            if (maintStatus == MAINTENANCE_COMPLETE){ // all tasks success

                LOGINFO("DBG:Maintenance Successfully Completed!!");
                /*  we store the time in persistant location */
                time_t successfulTime=time(nullptr);
                tm ltime=*localtime(&successfulTime);
                time_t epoch_time=mktime(&ltime);

                LOGINFO("last succesful time is :%s", to_string(epoch_time).c_str());
                std::unique_lock<std::mutex> lck(apiMutex);
                lck.lock();
                /* Remove any old completion time */
                m_setting.remove("LastSuccessfulCompletionTime");
                m_setting.setValue("LastSuccessfulCompletionTime",to_string(epoch_time).c_str());
                lck.unlock();
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_COMPLETE);
            }
            else if(maintStatus == MAINTENANCE_ERROR){/* Check other than all success case which means we have errors */
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_ERROR);
            }
            else{
                MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_INCOMPLETE);
            }
            DeinitializeIARM();
            taskList.clear();
            std::unique_lock<std::mutex> lck(apiMutex);
            maintenanceInProgress = false;
            stopMaintenanceFlag = false;
            LOGINFO("exiting task execution thread");
        }
        void MaintenanceManager::initMaintenanceTasks(Maintenance_Type_t maintType)
        {
            if(maintType == UNSOLICITED_MAINTENANCE)
            {
            	LOGINFO("Starting UNSOLICITED_MAINTENANCE\n");

            	taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("START_TIME", "/lib/rdk/StartDCM_maintaince.sh")));
                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("RFC", "/lib/rdk/RFCbase.sh")));
                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("FW_UPDATE", "/lib/rdk/swupdate_utility.sh >> /opt/logs/swupdate.log")));
                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("LOG_UPLOAD", "/lib/rdk/Start_uploadSTBLogs.sh")));
            }
            else
            {
            	LOGINFO("Starting SOLICITED_MAINTENANCE\n");

                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("RFC", "/lib/rdk/RFCbase.sh")));
                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("FW_UPDATE", "/lib/rdk/swupdate_utility.sh >> /opt/logs/swupdate.log")));
                taskList.push_back(std::shared_ptr<MaintenanceTask>(new MaintenanceTask("LOG_UPLOAD", "/lib/rdk/Start_uploadSTBLogs.sh")));
            }
        }

        void MaintenanceManager::runMaintenance(Maintenance_Type_t maintType) {

            maintenanceInProgress = true;
            MaintenanceManager::_instance->onMaintenanceStatusChange(MAINTENANCE_STARTED);
            initMaintenanceTasks(maintType);
            /* on boot up we set these things */
            MaintenanceManager::maintenanceMode = FOREGROUND_MODE;
            MaintenanceManager::isCriticalMaintenance=false;
            if(maintType == UNSOLICITED_MAINTENANCE)
                MaintenanceManager::isRebootPending=false;
            else
                MaintenanceManager::isRebootPending=true;

            taskThread = std::thread(&MaintenanceManager::task_execution_thread, _instance);
            taskThread.detach();
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
            std::lock_guard<std::mutex> guard(apiMutex);
            LOGINFO("inside startMaintenance");
            /* only one maintenance at a time */
            if (!maintenanceInProgress){

                runMaintenance(SOLICITED_MAINTENANCE);
                returnResponse(true);
            }
            else {
                LOGINFO("previous maintenance in Progress. Please wait for it to complete !!");
            }
            returnResponse(false);
        }

        void MaintenanceManager::_MaintenanceMgrEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            if (MaintenanceManager::_instance){
                MaintenanceManager::_instance->iarmEventHandler(owner, eventId, data, len);
            }
            else
                LOGWARN("WARNING - cannot handle IARM events without MaintenanceManager plugin instance!");
        }

        std::shared_ptr<MaintenanceTask> MaintenanceManager::findMaintenanceTask(const char *taskName)
        {
        	for(auto taskIT = taskList.begin(); taskIT != taskList.end(); taskIT++){
        		if(!(*taskIT)->getTaskName().compare(taskName))
        		{
        			return *taskIT;
        		}
        	}
   			LOGINFO("Task %s not found\n", taskName);
   			return NULL;
        }

        void MaintenanceManager::iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
        {
            IARM_Bus_MaintMGR_EventData_t *module_event_data=(IARM_Bus_MaintMGR_EventData_t*)data;
            IARM_Maint_module_status_t module_status;
            std::shared_ptr<MaintenanceTask> task = NULL;
            IARM_Bus_MaintMGR_EventId_t event = (IARM_Bus_MaintMGR_EventId_t)eventId;

            if (!strcmp(owner, IARM_BUS_MAINTENANCE_MGR_NAME)) {
                if ( IARM_BUS_DCM_NEW_START_TIME_EVENT == eventId ) {
                    /* we got a new start time from DCM script */
                    string l_time(module_event_data->data.startTimeUpdate.start_time);
                    LOGINFO("IARM event Received : IARM_BUS_DCM_NEW_START_TIME_EVENT - %s", l_time.c_str());

                    std::lock_guard<std::mutex> lck(apiMutex);
                    task = findMaintenanceTask("START_TIME");
                    if(task != NULL && task->getStatus() == TASK_STARTED)
                    {
                    	/* Store locally */
                    	maintenanceStartTimeInEpoch=l_time;
                    }
                }
                else if ( IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE == eventId ) {
                	module_status = module_event_data->data.maintenance_module_status.status;
                    string status_string=moduleStatusToString(module_status);
                    LOGINFO("IARM event Received : %s", status_string.c_str());
                    switch (module_status) {
                        case MAINT_RFC_COMPLETE :
                        	task = findMaintenanceTask("RFC");
                            if(task != NULL && task->setStatus(TASK_COMPLETE))
                                taskThreadCV.notify_one();
                            break;
                        case MAINT_RFC_ERROR:
                        	task = findMaintenanceTask("RFC");
                            if(task != NULL && task->setStatus(TASK_ERROR))
                                taskThreadCV.notify_one();
                            break;

                        case MAINT_DCM_COMPLETE :
                        	task = findMaintenanceTask("START_TIME");
                            if(task != NULL && task->setStatus(TASK_COMPLETE))
                                taskThreadCV.notify_one();
                            break;
                        case MAINT_DCM_ERROR:
                        	task = findMaintenanceTask("START_TIME");
                            if(task != NULL && task->setStatus(TASK_ERROR))
                                taskThreadCV.notify_one();
                            break;

                        case MAINT_FWDOWNLOAD_COMPLETE:
                        	task = findMaintenanceTask("FW_UPDATE");
                            if(task != NULL && task->setStatus(TASK_COMPLETE))
                                taskThreadCV.notify_one();
                            break;
                        case MAINT_FWDOWNLOAD_ABORTED:
                        	task = findMaintenanceTask("FW_UPDATE");
                            if(task != NULL && task->setStatus(TASK_SKIPPED))
                                taskThreadCV.notify_one();
                            break;
                        case MAINT_FWDOWNLOAD_ERROR:
                        	task = findMaintenanceTask("FW_UPDATE");
                            if(task != NULL && task->setStatus(TASK_ERROR))
                                taskThreadCV.notify_one();
                            break;

                        case MAINT_LOGUPLOAD_COMPLETE :
                        	task = findMaintenanceTask("LOG_UPLOAD");
                            if(task != NULL && task->setStatus(TASK_COMPLETE))
                                taskThreadCV.notify_one();
                            break;
                        case MAINT_LOGUPLOAD_ERROR:
                        	task = findMaintenanceTask("LOG_UPLOAD");
                            if(task != NULL && task->setStatus(TASK_ERROR))
                                taskThreadCV.notify_one();
                            break;

                        case MAINT_REBOOT_REQUIRED :
                        	task = findMaintenanceTask("FW_UPDATE");
                        	if(task != NULL && task->getStatus() == TASK_STARTED)
                        	{
                        		LOGINFO("Updating isRebootPending to true\n");
                        		isRebootPending=true;
                        	}
                            break;
                        case MAINT_CRITICAL_UPDATE:
                        	task = findMaintenanceTask("FW_UPDATE");
                        	if(task != NULL && task->getStatus() == TASK_STARTED)
                        	{
                        		LOGINFO("Updating isCriticalMaintenance to true\n");
                        		isCriticalMaintenance=true;
                        	}
                            break;
                    }
                }
                else{
                    LOGINFO("Unknown Maintenance Status!!");
                }
            }
            else {
                LOGWARN("Ignoring unexpected event - owner: %s, eventId: %d!!", owner, eventId);
            }
        }

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
                JsonObject& response){

				string lastSuccessfulCompletionTime = "NA"; /* TODO : check max size to hold this */
				string getMaintenanceStatusString = "\0";

				std::lock_guard<std::mutex> guard(apiMutex);
				/* Get the last SuccessfulCompletion time from Persistant location */
				if (m_setting.contains("LastSuccessfulCompletionTime")){
					lastSuccessfulCompletionTime=m_setting.getValue("LastSuccessfulCompletionTime").String();
				}
				if(strcmp("NA",lastSuccessfulCompletionTime.c_str())==0)
				{
					response["LastSuccessfulCompletionTime"] = 0;  // stoi is not able handle "NA"
				}
				else
				{
					try{
						response["LastSuccessfulCompletionTime"] = stoi(lastSuccessfulCompletionTime.c_str());
					}
					catch(exception &err)
					{
						//exception caught with stoi -- So making "LastSuccessfulCompletionTime" as 0
						response["LastSuccessfulCompletionTime"] = 0;
					}
				}

				response["maintenanceStatus"] = notifyStatusToString(maintenanceStatus);
				response["isCriticalMaintenance"] = isCriticalMaintenance;
				response["isRebootPending"] = isRebootPending;

				returnResponse(true);
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
            std::lock_guard<std::mutex> guard(apiMutex);
            if(!maintenanceStartTimeInEpoch.empty()) {

                response["maintenanceStartTime"] = stoi(maintenanceStartTimeInEpoch.c_str());
                result=true;
            }
            else {
                string startTime = Utils::cRunScript("/lib/rdk/getMaintenanceStartTime.sh &");
                if (!startTime.empty()){
                    response["maintenanceStartTime"]=stoi(startTime.c_str());
                    result=true;
                }
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
            std::lock_guard<std::mutex> guard(apiMutex);
            /*  we set the default value to FG */
            if ( parameters.HasLabel("maintenanceMode") ){
                /* Get the value */
                new_mode = parameters["maintenanceMode"].String();
                LOGINFO("SetMaintenanceMode new_mode = %s\n",new_mode.c_str());
                if ( BACKGROUND_MODE != new_mode && FOREGROUND_MODE != new_mode )  {
                    LOGERR("value of new mode is incorrect, therefore \
                            current mode '%s' not changed.\n", maintenanceMode.c_str());
                    returnResponse(false);
                }
                 /* check if maintenance is on progress or not */
                /* if in progress restrict the same */
                if ( !maintenanceInProgress ){
                    maintenanceMode = new_mode;
                    if ( BACKGROUND_MODE == new_mode ) {
                        m_setting.setValue("background_flag","true");
                    }
                    else {
                        m_setting.remove("background_flag");
                        m_setting.setValue("background_flag","false");
                    }
                    result = true;
                }
                else{
                    LOGERR("Maintenance is in Progress, Mode change not allowed");
                    result =false;
                }
            }
            else {
                /* havent got the correct label */
                LOGERR("SetMaintenanceMode Missing Key Values\n");
                populateResponseWithError(SysSrv_MissingKeyValues,response);
            }

            returnResponse(result);
        }

        void MaintenanceManager::onMaintenanceStatusChange(MAINTENANCE_STATUS status) {
            JsonObject params;
            /* we store the updated value as well */
            maintenanceStatus=status;
            params["maintenanceStatus"]=notifyStatusToString(status);
            sendNotify(EVT_ONMAINTENANCSTATUSCHANGE, params);
        }


#ifndef DEBUG
        /**
         * @brief : sampleAPI
         */
//        uint32_t MaintenanceManager::sampleAPI(const JsonObject& parameters,
//                JsonObject& response)
//        {
//            response["sampleAPI"] = "Success";
//            /* Kept for debug purpose/future reference. */
//            sendNotify(EVT_ONMAINTMGRSAMPLEEVENT, parameters);
//            returnResponse(true);
//        }

        uint32_t MaintenanceManager::stopMaintenance(const JsonObject& parameters,
                JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(apiMutex);
            /* only one maintenance at a time */
            if (!maintenanceInProgress){

                LOGINFO("Maintenance is not in progress, nothing to stop\n");
                returnResponse(false);
            }
            else {
            	stopMaintenanceFlag = true;
            	taskThreadCV.notify_one();
                LOGINFO("stopMaintenanceFlag set to true, waiting for task execution thread to complete\n");
            }
            returnResponse(true);
        }
#endif /* DEBUG */

   } /* namespace Plugin */
} /* namespace WPEFramework */
