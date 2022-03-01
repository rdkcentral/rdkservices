#pragma once

typedef int IARM_EventId_t;
typedef enum _IARM_Result_t
{
  IARM_RESULT_SUCCESS,  
  IARM_RESULT_INVALID_PARAM, /*!< Invalid input parameter */
  IARM_RESULT_INVALID_STATE, /*!< Invalid state encountered */
  IARM_RESULT_IPCCORE_FAIL,  /*!< Underlying IPC failure */
  IARM_RESULT_OOM,           /*!< Memory allocation failure */
} IARM_Result_t;
