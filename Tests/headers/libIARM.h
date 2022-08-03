#pragma once

#define IARM_BUS_NAME "com.comcast.rdk.iarm.bus"
#define IARM_MAX_NAME_LEN 64
#define IARM_BUS_COMMON_API_SysModeChange "SysModeChange"

typedef int IARM_EventId_t;

typedef enum _IARM_Result_t
{
  IARM_RESULT_SUCCESS,  
  IARM_RESULT_INVALID_PARAM,
  IARM_RESULT_INVALID_STATE,
  IARM_RESULT_IPCCORE_FAIL,
  IARM_RESULT_OOM,

} IARM_Result_t;
typedef IARM_Result_t (*IARM_BusCall_t) (void *arg);

void _powerEventHandler(const char*, IARM_EventId_t, void*, size_t);
#define IARM_METHOD_IPC_TIMEOUT_DEFAULT    (-1)
#define IARM_METHOD_IPC_TIMEOUT_INFINITE   ((int) 0x7fffffff)

typedef enum _IARM_Bus_Daemon_PowerState_t {
    IARM_BUS_PWRMGR_POWERSTATE_OFF, /*!< Used for both IARM Bus Daemon Power pre change and Power manager Power state OFF */
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY, /*!< Used for both IARM Bus Daemon Power pre change and Power manager Power state STANDBY */
    IARM_BUS_PWRMGR_POWERSTATE_ON,  /*!< Used for both IARM Bus Daemon Power pre change and Power manager Power state ON */
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP, /*!< Used for both IARM Bus Daemon Power pre change and Power manager Power state set/get */
    IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP   /*!< Used for both IARM Bus Daemon Power pre change and Power manager Power state set/get */
} IARM_Bus_PowerState_t;

typedef IARM_Bus_PowerState_t IARM_Bus_PWRMgr_PowerState_t;

typedef enum _IARM_Bus_Daemon_SysMode_t{
    IARM_BUS_SYS_MODE_NORMAL,
    IARM_BUS_SYS_MODE_EAS,
    IARM_BUS_SYS_MODE_WAREHOUSE
} IARM_Bus_Daemon_SysMode_t;
