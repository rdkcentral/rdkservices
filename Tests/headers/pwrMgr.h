#pragma once

#define PWRMGR_MAX_REBOOT_REASON_LENGTH 100

#define MAX_PWR_STATE_BEF_REBOOR_STR_LEN (32)
#define IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot "GetPowerStateBeforeReboot"
#define IARM_BUS_PWRMGR_API_SetStandbyVideoState "SetStandbyVideoState"
#define IARM_BUS_PWRMGR_API_GetStandbyVideoState "GetStandbyVideoState"

#define IARM_BUS_PWRMGR_API_SetNetworkStandbyMode  "SetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_GetNetworkStandbyMode  "GetNetworkStandbyMode"
#define IARM_BUS_PWRMGR_API_Reboot "performReboot"

typedef struct _IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t {
       bool bStandbyMode;        /*!< Standby mode to set and get*/
} IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t {
    char powerStateBeforeReboot [MAX_PWR_STATE_BEF_REBOOR_STR_LEN];
} IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t {
        unsigned int timeout;        /*!< Timeout for deep sleep in seconds*/
} IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t;

typedef enum _IARM_Bus_PWRMgr_ThermalState_t{
    IARM_BUS_PWRMGR_TEMPERATURE_NORMAL = 0,    /* Temp is within normal operating range */
    IARM_BUS_PWRMGR_TEMPERATURE_HIGH,          /* Temp is high, but just a warning as device can still operate */
    IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL       /* Temp is critical, should trigger a thermal reset */
} IARM_Bus_PWRMgr_ThermalState_t;


typedef struct _PWRMgr_EventData_t {
    union {
        struct _MODE_DATA{
                IARM_Bus_PWRMgr_PowerState_t curState;  /*!< Power manager current power state */
                IARM_Bus_PWRMgr_PowerState_t newState;  /*!< Power manager new power state */
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
     IARM_Bus_PWRMgr_ThermalState_t curLevel;                     /*!<  Current Thermal level */
     float curTemperature;                                                /* !< Current temperature value */
} IARM_Bus_PWRMgr_GetThermalState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetTempThresholds_Param_t{
     float tempHigh;                     /*!< New threshold at which TEMPERATURE_HIGH will be reported  */
     float tempCritical;                  /*!< New threshold at which TEMPERATURE_CRITICAL will be reported  */
} IARM_Bus_PWRMgr_GetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetTempThresholds_Param_t{
     float tempHigh;                     /*!< New threshold at which TEMPERATURE_HIGH will be reported  */
     float tempCritical;                  /*!< New threshold at which TEMPERATURE_CRITICAL will be reported  */
} IARM_Bus_PWRMgr_SetTempThresholds_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t{
     int graceInterval;                     /*!< New over temparature grace interval  */
} IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t;

typedef struct _IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t{
     int graceInterval;                     /*!< New over teamparature grace interval  */
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

#define IARM_BUS_PWRMGR_NAME 					"PWRMgr"  /*!< Power manager IARM bus name */
#define IARM_BUS_PWRMGR_API_SetPowerState 		"SetPowerState" /*!< Sets the powerstate of the device*/
#define IARM_BUS_PWRMGR_API_GetPowerState 		"GetPowerState" /*!< Retrives current  power state of the box*/

typedef struct _IARM_Bus_PWRMgr_SetPowerState_Param_t {
	IARM_Bus_PWRMgr_PowerState_t newState;        /*!< [in] New powerstate to be set */
        int keyCode;                                  /*!< [in] Key code for the last key Pressed */
} IARM_Bus_PWRMgr_SetPowerState_Param_t;

typedef struct _IARM_Bus_PWRMgr_GetPowerState_Param_t {
	IARM_Bus_PWRMgr_PowerState_t curState;        /*!< Current powerstate of the box*/
} IARM_Bus_PWRMgr_GetPowerState_Param_t;

#define IARM_BUS_PWRMGR_API_GetThermalState            "GetThermalState" /*!< Retrieves current thermal level of the box*/
#define IARM_BUS_PWRMGR_API_GetTemperatureThresholds   "GetTemperatureThresholds" /*!< Gets the thermal threshold  for the device*/
#define IARM_BUS_PWRMGR_API_SetTemperatureThresholds   "SetTemperatureThresholds" /*!< Sets the thermal threshold  for the device*/
#define IARM_BUS_PWRMGR_API_GetOvertempGraceInterval   "GetOvertempGraceInterval" /*!< Gets the over temparature grace interval for the device*/
#define IARM_BUS_PWRMGR_API_SetOvertempGraceInterval   "SetOvertempGraceInterval" /*!< Sets the over temparature grace interval for the device*/