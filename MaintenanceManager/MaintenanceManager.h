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
#define EVT_ONFWPENDINGREBOOT               "onFirmwarePendingReboot" /* Auto Reboot notifier */
#define EVT_ONMAINTENANCSTATUSCHANGE        "onMaintenanceStatusChange" /* Maintenance Status change */
/* we have a persistant file to hold the record */
#define MAITNENANCE_MGR_RECORD_FILE          "/opt/mw/maintenance_mgr_record.conf"

typedef enum {
    MAINTENANCE_IDLE,
    MAINTENANCE_STARTED,
    MAINTENANCE_ERROR,
    MAINTENANCE_COMPLETE,
    MAINTENANCE_INCOMPLETE
} Maint_notify_status;

#define FOREGROUND_MODE "FOREGROUND"
#define BACKGROUND_MODE "BACKGROUND"

#if 0
typedef enum {
    IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE=0,
    IARM_BUS_DCM_NEW_START_TIME_EVENT,
}IARM_Bus_MaintMGR_EventId_t;

/* Notification to rdkservice over IARM */
typedef enum {
    MAINT_DCM_COMPLETE,
    MAINT_DCM_ERROR,
    MAINT_RFC_COMPLETE,
    MAINT_RFC_ERROR,
    MAINT_LOGUPLOAD_COMPLETE,
    MAINT_LOGUPLOAD_ERROR,
    MAINT_PINGTELEMETRY_COMPLETE,
    MAINT_PINGTELEMETRY_ERROR,
    MAINT_FWDOWNLOAD_COMPLETE,
    MAINT_FWDOWNLOAD_ERROR,
    MAINT_FWDOWNLOAD_ABORTED,
    MAINT_REBOOT_REQUIRED,
    MAINT_STATUS_EMPTY
} IARM_Maint_module_status;

/* Event Data for holding the start time and module status */
typedef struct {
    union{
        struct _DCM_DATA{
            int hr; /* Start time in Hours */
            int min; /* Start time in Minutes*/
        }start_time;
        struct _MAINT_STATUS_UPDATE{
            IARM_Maint_module_status status;
        }maintenance_module_status;
    } data;
}IARM_Bus_MaintMGR_EventData_t;

#endif

namespace WPEFramework {
    namespace Plugin {

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

                static string g_currentMode;
                static string g_is_critical_maintenance;
                static IARM_Bus_MaintMGR_EventData_t *g_maintenance_data;
                static Maint_notify_status g_notify_status;
                static cSettings m_setting;

                static void iarmEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
                static void _MaintenanceMgrEventHandler(const char *owner,IARM_EventId_t eventId, void *data, size_t len);
                Utils::ThreadRAII run_DCMScript_thread;
                Utils::ThreadRAII run_RFCBase_thread;
                // We do not allow this plugin to be copied !!
                MaintenanceManager(const MaintenanceManager&) = delete;
                MaintenanceManager& operator=(const MaintenanceManager&) = delete;
            public:
                MaintenanceManager();
                virtual ~MaintenanceManager();

                static const short API_VERSION_NUMBER_MAJOR;
                static const short API_VERSION_NUMBER_MINOR;

                static MaintenanceManager* _instance;
                virtual const string Initialize(PluginHost::IShell* service) override;
                virtual void Deinitialize(PluginHost::IShell* service) override;
                static int runScript(const std::string& script,
                        const std::string& args, string *output = NULL,
                        string *error = NULL, int timeout = 30000);

#if defined(USE_IARMBUS) || defined(USE_IARM_BUS)
                void InitializeIARM();
                void DeinitializeIARM();
#endif /* defined(USE_IARMBUS) || defined(USE_IARM_BUS) */

                /* Events : Begin */
                void onFirmwarePendingReboot(int seconds); /* Event handler for Pending Reboot */
                void onMaintenanceStatusChange(Maint_notify_status status);
                /* Events : End */

                /* Methods : Begin */
#ifdef DEBUG
                uint32_t sampleAPI(const JsonObject& parameters, JsonObject& response);
#endif /* DEBUG */

                /* TODO: Stub implementation; Decide whether needed or not since setProperty
                   and getProperty functionalities are XRE/RTRemote dependent. */
                uint32_t fireFirmwarePendingReboot(const JsonObject& parameters, JsonObject& response);
                uint32_t setFirmwareRebootDelay(const JsonObject& parameters, JsonObject& response);
                uint32_t setFirmwareAutoReboot(const JsonObject& parameters, JsonObject& response);
                uint32_t GetMaintenanceActivityStatus(const JsonObject& parameters, JsonObject& response);
                uint32_t GetMaintenanceStartTime(const JsonObject& parameters, JsonObject& response);
                uint32_t SetMaintenanceMode(const JsonObject& parameters, JsonObject& response);
                uint32_t StartMaintenance(const JsonObject& parameters, JsonObject& response);
        }; /* end of MaintenanceManager service class */
    } /* end of plugin */
} /* end of wpeframework */

#endif //MAINTENANCEMANAGER_H
