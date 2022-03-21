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
* @file libIBus.h
*
* @brief RDK IARM-Bus API Declarations.
*
* @defgroup IARMBUS_API IARM Bus API
* @ingroup IARMBUS
* Application should use the APIs declared in this file to access
* services provided by IARM-Bus. Basically services provided by
* these APIs include:
* <br> 1) Library Initialization and termination.
* <br> 2) Connection to IARM-Bus.
* <br> 3) Send and Receive Events.
* <br> 4) Declared and Invoke RPC Methods.
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

/** @defgroup IARMBUS IARM Bus
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
*
* @par Capabilities
* <ol>
* <li> Invoke methods in other processes via Remote Procedure Call (RPC).
* <li> Send interprocess messages.
* <li> Manage shared memory and exclusive access to resources.
* <li> Register for event notification.
* <li> Publish event notification to registered listeners.
* </ol>
*/

#ifndef _LIB_IARM_BUS_H
#define _LIB_IARM_BUS_H



#ifdef __cplusplus
extern "C" 
{
#endif

#include "libIARM.h"

typedef IARM_Result_t (*IARM_BusCall_t) (void *arg);

typedef void (*IARM_EventHandler_t)(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

IARM_Result_t IARM_Bus_Init(const char *name)
{
IARM_Result_t retCode = IARM_RESULT_SUCCESS;
return retCode;
}

IARM_Result_t IARM_Bus_Connect(void)
{
    IARM_Result_t retCode = IARM_RESULT_SUCCESS;

    return retCode;
}	

IARM_Result_t IARM_Bus_IsConnected(const char *memberName, int *isRegistered)
{
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
	return IARM_RESULT_SUCCESS;
}
 
IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId)
{
	return IARM_RESULT_SUCCESS;
}


/* End of IARM_BUS_IARM_CORE_API doxygen group */
/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif

