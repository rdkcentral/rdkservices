#pragma once

#define IARM_BUS_NAME "com.comcast.rdk.iarm.bus"
#define IARM_MAX_NAME_LEN 64

typedef int IARM_EventId_t;

typedef enum _IARM_Result_t
{
  IARM_RESULT_SUCCESS,  
  IARM_RESULT_INVALID_PARAM,
  IARM_RESULT_INVALID_STATE,
  IARM_RESULT_IPCCORE_FAIL,
  IARM_RESULT_OOM,

} IARM_Result_t;

#define IARM_METHOD_IPC_TIMEOUT_DEFAULT    (-1)
#define IARM_METHOD_IPC_TIMEOUT_INFINITE   ((int) 0x7fffffff)
