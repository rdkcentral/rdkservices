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

#ifndef MAINTENANCEMANAGER_H
#define MAINTENANCEMANAGER_H

#include <stdint.h>
#include <thread>

#include "Module.h"
#include "tracing/Logging.h"
#include "utils.h"
#include "AbstractPlugin.h"
#include "SystemServicesHelper.h"
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
#include "libIARM.h"
#include "libIBus.h"
#include "irMgr.h"
#include "libIBusDaemon.h"
#include "pwrMgr.h"
#include "maintenanceMGR.h" /* IARM INTERFACE HELPER */
#endif /* USE_IARMBUS || USE_IARM_BUS */

#include "sysMgr.h"
#include "cTimer.h"
#include "rfcapi.h"
#include "cSettings.h"

/* MaintenanceManager Services Triggered Events. */
#define EVT_ONMAINTMGRSAMPLEEVENT           "onSampleEvent"
#define EVT_ONMAINTENANCSTATUSCHANGE        "onMaintenanceStatusChange" /* Maintenance Status change */
/* we have a persistant file to hold the record */
#define MAINTENANCE_MGR_RECORD_FILE          "/opt/maintenance_mgr_record.conf"

typedef enum {
    MAINTENANCE_IDLE,
    MAINTENANCE_STARTED,
    MAINTENANCE_ERROR,
    MAINTENANCE_INCOMPLETE,
    MAINTENANCE_COMPLETE
} MAINTENANCE_STATUS;

typedef enum {
    TASK_IDLE,
    TASK_STARTED,
    TASK_ERROR,
    TASK_SKIPPED,
	TASK_INCOMPLETE,
    TASK_COMPLETE
}TASK_STATUS;

typedef enum{
    SOLICITED_MAINTENANCE,
    UNSOLICITED_MAINTENANCE
}Maintenance_Type_t;

#define FOREGROUND_MODE "FOREGROUND"
#define BACKGROUND_MODE "BACKGROUND"

namespace WPEFramework {
    namespace Plugin {

		class MaintenanceTask{
			private:
				std::mutex statusMutex;
				std::string taskName;
				std::string taskScript;
				TASK_STATUS taskStatus = TASK_IDLE; // represents task status part of current maintenance activity
				bool taskExecStatus = false; // represents whether this task has been executed part of current maintenance activity

			public:
				MaintenanceTask(const std::string &name, const std::string &script): taskName(name), taskScript(script){
					LOGINFO("Constructor: %s\n",taskName.c_str());
					taskStatus = TASK_IDLE;
				}
				~MaintenanceTask(){
					LOGINFO("Destructor: %s\n",taskName.c_str());
				}
//				MaintenanceTask(const MaintenanceTask&) = delete;
//				MaintenanceTask& operator=(const MaintenanceTask&) = delete;
				bool startTask()
				{
					//WARNING:::This method shouldn't block as this mutex is used in setStatus also. setStaus is called from IARM on event which should never block
					std::lock_guard<std::mutex> guard(statusMutex);
					LOGINFO("Starting %s\n",taskName.c_str());
					if(!taskExecStatus && taskStatus != TASK_STARTED)
					{
						string cmd=taskScript.append(" &\0");
						int ret = system(cmd.c_str());
						LOGINFO("System command execution status - %d\n", ret);
						if(ret != 0) //TODO: Confirm if system return 0 always on success
						{
							LOGINFO("Failed to start %s\n", taskName.c_str());
							taskExecStatus = true;
							taskStatus = TASK_SKIPPED;
							return false;
						}
					}
					else
					{
						LOGINFO("Task %s is already running\n",taskName.c_str());
					}
					taskExecStatus = true;
					taskStatus = TASK_STARTED;
					return true;
				}
				bool setStatus(TASK_STATUS status){
					std::lock_guard<std::mutex> guard(statusMutex);
					LOGINFO("Set status %d for Task :%s \n",status, taskName.c_str());
					if(taskExecStatus) //update taskStatus only if already started by MM
					{
						taskStatus = status;
						return true;
					}
					else if(status == TASK_ERROR || status == TASK_SKIPPED || status == TASK_COMPLETE)
					{
						taskStatus = TASK_IDLE;
						return false;
					}
					else
						taskStatus = status;
					return false;
				}
				TASK_STATUS getStatus(){
					std::lock_guard<std::mutex> guard(statusMutex);
					return taskStatus;
				}
				string getTaskName(){
					return taskName;
				}
		};
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

        class MaintenanceManager : public AbstractPlugin {
            private:

                typedef Core::JSON::String JString;
                typedef Core::JSON::ArrayType<JString> JStringArray;
                typedef Core::JSON::Boolean JBool;

                string maintenanceMode;
                bool isCriticalMaintenance;
                bool isRebootPending;
                MAINTENANCE_STATUS maintenanceStatus;
                string maintenanceStartTimeInEpoch;

                static cSettings m_setting;
                std::mutex  apiMutex;
                std::mutex  taskThreadMutex;
                std::condition_variable taskThreadCV;
                std::thread taskThread;
                Maintenance_Type_t maintenanceType;
                bool maintenanceInProgress;
                bool stopMaintenanceFlag;
                std::list<std::shared_ptr<MaintenanceTask>> taskList;

                IARM_Bus_MaintMGR_EventData_t *g_maintenance_data;
                void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
                static void _MaintenanceMgrEventHandler(const char *owner,IARM_EventId_t eventId, void *data, size_t len);
                // We do not allow this plugin to be copied !!
                MaintenanceManager(const MaintenanceManager&) = delete;
                MaintenanceManager& operator=(const MaintenanceManager&) = delete;
                void task_execution_thread();
                void initMaintenanceTasks(Maintenance_Type_t maintType);
                void runMaintenance(Maintenance_Type_t maintType);
                std::shared_ptr<MaintenanceTask> findMaintenanceTask(const char *taskName);
                MAINTENANCE_STATUS getMaintenanceCompletionStatus();
#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                bool InitializeIARM();
                void DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */


            public:
                MaintenanceManager();
                virtual ~MaintenanceManager();

                static MaintenanceManager* _instance;
                virtual const string Initialize(PluginHost::IShell* service) override;
                virtual void Deinitialize(PluginHost::IShell* service) override;
                /* Events : Begin */
                void onMaintenanceStatusChange(MAINTENANCE_STATUS status);
                /* Events : End */

                /* Methods : Begin */
#ifdef DEBUG
                uint32_t sampleAPI(const JsonObject& parameters, JsonObject& response);
#endif /* DEBUG */
                uint32_t getMaintenanceActivityStatus(const JsonObject& parameters, JsonObject& response);
                uint32_t getMaintenanceStartTime(const JsonObject& parameters, JsonObject& response);
                uint32_t setMaintenanceMode(const JsonObject& parameters, JsonObject& response);
                uint32_t startMaintenance(const JsonObject& parameters, JsonObject& response);
#ifndef DEBUG
                uint32_t stopMaintenance(const JsonObject& parameters, JsonObject& response);
#endif
        }; /* end of MaintenanceManager service class */
    } /* end of plugin */
} /* end of wpeframework */

#endif //MAINTENANCEMANAGER_H
