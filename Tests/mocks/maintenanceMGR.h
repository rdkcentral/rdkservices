/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
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
  */
 
 
 #ifndef _IARM_BUS_MAINTENANCEMGR_H
 #define _IARM_BUS_MAINTENANCEMGR_H
 
 #define IARM_BUS_MAINTENANCE_MGR_NAME "MaintenanceMGR"
 
 typedef enum {
  IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE=0, /* Event status as data */
  IARM_BUS_DCM_NEW_START_TIME_EVENT, /* Payload as Time */
 }IARM_Bus_MaintMGR_EventId_t;
 
 /* Notification to rdkservice over IARM */
 typedef enum {
  MAINT_DCM_COMPLETE=0,
  MAINT_DCM_ERROR, // 1
  MAINT_RFC_COMPLETE, // 2
  MAINT_RFC_ERROR, // 3
  MAINT_LOGUPLOAD_COMPLETE, // 4
  MAINT_LOGUPLOAD_ERROR, // 5
  MAINT_PINGTELEMETRY_COMPLETE, // 6
  MAINT_PINGTELEMETRY_ERROR, // 7
  MAINT_FWDOWNLOAD_COMPLETE, //8
  MAINT_FWDOWNLOAD_ERROR, //9
  MAINT_FWDOWNLOAD_ABORTED, //10
  MAINT_CRITICAL_UPDATE, // 11
  MAINT_REBOOT_REQUIRED, //12
  MAINT_DCM_INPROGRESS, //13
  MAINT_RFC_INPROGRESS, //14
  MAINT_FWDOWNLOAD_INPROGRESS, //15
  MAINT_LOGUPLOAD_INPROGRESS, //16
  MAINT_STATUS_EMPTY //17
 } IARM_Maint_module_status_t;
 
 #define MAX_TIME_LEN 32
 
 /* Event Data for holding the start time and module status */
 typedef struct {
  union{
  struct _DCM_DATA{
  char start_time[MAX_TIME_LEN];
  }startTimeUpdate;
  struct _MAINT_STATUS_UPDATE{
  IARM_Maint_module_status_t status;
  }maintenance_module_status;
  } data;
 }IARM_Bus_MaintMGR_EventData_t;
 
 
 #endif