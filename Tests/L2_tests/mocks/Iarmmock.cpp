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
