#ifndef _LIB_IARM_H
#define _LIB_IARM_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/types.h>


#define IARM_BUS_NAME "com.comcast.rdk.iarm.bus" /*!< Well-known Bus Name */
#define IARM_MAX_NAME_LEN 64 /*!< Maximum string length of names in IARM, including the null terminator */

typedef int IARM_EventId_t;

typedef enum _IARM_Result_t
{
  IARM_RESULT_SUCCESS,
  IARM_RESULT_INVALID_PARAM, /*!< Invalid input parameter */
  IARM_RESULT_INVALID_STATE, /*!< Invalid state encountered */
  IARM_RESULT_IPCCORE_FAIL,  /*!< Underlying IPC failure */
  IARM_RESULT_OOM,           /*!< Memory allocation failure */

} IARM_Result_t;

#define IARM_METHOD_IPC_TIMEOUT_DEFAULT    (-1)
#define IARM_METHOD_IPC_TIMEOUT_INFINITE   ((int) 0x7fffffff)

#ifdef __cplusplus
}
#endif
#endif

