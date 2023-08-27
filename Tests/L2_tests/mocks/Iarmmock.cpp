# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#include "Iarm.h"
#include <string.h>

IARM_Result_t IARM_Bus_Init(const char* name)
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_Connect()
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
{
     *isRegistered = 1;
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
{
    return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_RemoveEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_Call(const char* ownerName, const char* methodName, void* arg, size_t argLen)
{
     printf("SUMA: owner name = %s, method_name = %s\n",ownerName,methodName);
     if(0 == strcmp(ownerName,IARM_BUS_PWRMGR_NAME))
     {
     printf("SUMA: owner matching\n");
        
     }

     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_BroadcastEvent(const char *ownerName, IARM_EventId_t eventId, void *arg, size_t argLen)
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_RegisterCall(const char* methodName, IARM_BusCall_t handler)
{
     return IARM_RESULT_SUCCESS;
}
IARM_Result_t IARM_Bus_Call_with_IPCTimeout(const char *ownerName,  const char *methodName, void *arg, size_t argLen, int timeout)
{
     return IARM_RESULT_SUCCESS;
}
