#pragma once

#include "libIARM.h"
#include "libIBusDaemon.h"

#define PWRMGR_MAX_REBOOT_REASON_LENGTH 100

#define MAX_PWR_STATE_BEF_REBOOR_STR_LEN (32)
#define IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot "GetPowerStateBeforeReboot"
#define IARM_BUS_PWRMGR_API_SetStandbyVideoState "SetStandbyVideoState"
#define IARM_BUS_PWRMGR_API_GetStandbyVideoState "GetStandbyVideoState"

#define IARM_BUS_PWRMGR_API_SetNetworkStandbyMode  "SetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_GetNetworkStandbyMode  "GetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_Reboot "performReboot"

typedef struct _IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t {
       bool bStandbyMode;
} IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t {
    char powerStateBeforeReboot [MAX_PWR_STATE_BEF_REBOOR_STR_LEN];
} IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t {
        unsigned int timeout;
} IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t;

typedef enum _IARM_Bus_PWRMgr_ThermalState_t{
    IARM_BUS_PWRMGR_TEMPERATURE_NORMAL = 0,
    IARM_BUS_PWRMGR_TEMPERATURE_HIGH,
    IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL
} IARM_Bus_PWRMgr_ThermalState_t;

typedef struct _PWRMgr_EventData_t {
    union {
        struct _MODE_DATA{
                IARM_Bus_PWRMgr_PowerState_t curState;
                IARM_Bus_PWRMgr_PowerState_t newState;
            #ifdef ENABLE_DEEP_SLEEP
                uint32_t deep_sleep_timeout;
            #endif
            #ifdef ENABLE_LLAMA_PLATCO_SKY_XIONE
                bool nwStandbyMode;
            #endif
        } state;
        #ifdef ENABLE_THERMAL_PROTECTION
        struct _THERM_DATA{
                //IARM_Bus_PWRMgr_ThermalState_t curLevel;
                //IARM_Bus_PWRMgr_ThermalState_t newLevel;
                float curTemperature;
        } therm;
        #endif
        bool bNetworkStandbyMode;
        int32_t reset_sequence_progress;
     } data;
}IARM_Bus_PWRMgr_EventData_t;

typedef struct _IARM_Bus_PWRMgr_GetThermalState_Param_t{
     IARM_Bus_PWRMgr_ThermalState_t curLevel;
     float curTemperature;
} IARM_Bus_PWRMgr_GetThermalState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetTempThresholds_Param_t{
     float tempHigh;
     float tempCritical;
} IARM_Bus_PWRMgr_GetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetTempThresholds_Param_t{
     float tempHigh;
     float tempCritical;
} IARM_Bus_PWRMgr_SetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t{
     int graceInterval;
} IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t{
     int graceInterval;
} IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t;

typedef struct _IARM_Bus_CommonAPI_SysModeChange_Param_t{
    IARM_Bus_Daemon_SysMode_t oldMode;
    IARM_Bus_Daemon_SysMode_t newMode;
}IARM_Bus_CommonAPI_SysModeChange_Param_t;

typedef enum _PWRMgr_EventId_t {
    IARM_BUS_PWRMGR_EVENT_MODECHANGED = 0,
    IARM_BUS_PWRMGR_EVENT_DEEPSLEEP_TIMEOUT,
    IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE,
    IARM_BUS_PWRMGR_EVENT_REBOOTING,
#ifdef ENABLE_THERMAL_PROTECTION
    IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED,
#endif // ENABLE_THERMAL_PROTECTION
    IARM_BUS_PWRMGR_EVENT_WAREHOUSEOPS_STATUSCHANGED,
    IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED,
    IARM_BUS_PWRMGR_EVENT_MAX,
} IARM_Bus_PWRMgr_EventId_t;

typedef struct _IARM_Bus_PWRMgr_RebootParam_t{
     char reboot_reason_custom[PWRMGR_MAX_REBOOT_REASON_LENGTH];
     char reboot_reason_other[PWRMGR_MAX_REBOOT_REASON_LENGTH];
     char requestor[PWRMGR_MAX_REBOOT_REASON_LENGTH];
} IARM_Bus_PWRMgr_RebootParam_t;

#define IARM_BUS_PWRMGR_NAME 					"PWRMgr"
#define IARM_BUS_PWRMGR_API_SetPowerState 		"SetPowerState"
#define IARM_BUS_PWRMGR_API_GetPowerState 		"GetPowerState"

typedef struct _IARM_Bus_PWRMgr_SetPowerState_Param_t {
	IARM_Bus_PWRMgr_PowerState_t newState;
        int keyCode;
} IARM_Bus_PWRMgr_SetPowerState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerState_Param_t {
	IARM_Bus_PWRMgr_PowerState_t curState;
} IARM_Bus_PWRMgr_GetPowerState_Param_t;

#define IARM_BUS_PWRMGR_API_GetThermalState            "GetThermalState"
#define IARM_BUS_PWRMGR_API_GetTemperatureThresholds   "GetTemperatureThresholds"
#define IARM_BUS_PWRMGR_API_SetTemperatureThresholds   "SetTemperatureThresholds"
#define IARM_BUS_PWRMGR_API_GetOvertempGraceInterval   "GetOvertempGraceInterval"
#define IARM_BUS_PWRMGR_API_SetOvertempGraceInterval   "SetOvertempGraceInterval"