/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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

/**
* @file libIARM.h
*
* @brief IARM-Bus IARM core library API.
*
* This API defines the core constants for IARM
*
* @par Document
* Document reference.
*
* @par Open Issues (in no particular order)
* -# None
*
* @par Assumptions
* -# None
*
* @par Abbreviations
* - BE:       ig-Endian.
* - cb:       allback function (suffix).
* - DS:      Device Settings.
* - FPD:     Front-Panel Display.
* - HAL:     Hardware Abstraction Layer.
* - LE:      Little-Endian.
* - LS:      Least Significant.
* - MBZ:     Must be zero.
* - MS:      Most Significant.
* - RDK:     Reference Design Kit.
* - _t:      Type (suffix).
*
* @par Implementation Notes
* -# None
*
*/

/** @defgroup IARM_BUS IARM_BUS
*    @ingroup IARM_BUS
*
*  IARM-Bus is a platform agnostic Inter-process communication (IPC) interface. It allows
*  applications to communicate with each other by sending Events or invoking Remote
*  Procedure Calls. The common programming APIs offered by the RDK IARM-Bus interface is
*  independent of the operating system or the underlying IPC mechanism.
*
*  Two applications connected to the same instance of IARM-Bus are able to exchange events
*  or RPC calls. On a typical system, only one instance of IARM-Bus instance is needed. If
*  desired, it is possible to have multiple IARM-Bus instances. However, applications
*  connected to different buses will not be able to communicate with each other.
*/

/** @addtogroup IARM_BUS_IARM_CORE_API IARM-Core library.
*  @ingroup IARM_BUS
*
*  Described herein are the constants declarations that are part of the
*  IARM Core library.
*
*  @{
*/

 
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

/* End of IARM_BUS_IARM_CORE_API doxygen group */
/**
 * @}
 */
