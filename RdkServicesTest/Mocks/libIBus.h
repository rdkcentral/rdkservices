#pragma once

#include "audiocapturemgr_iarm.h"
#include "libIARM.h"

typedef IARM_Result_t (*IARM_BusCall_t) (void *arg);

typedef void (*IARM_EventHandler_t)(const char *owner, IARM_EventId_t eventId, void *data, size_t len);

IARM_Result_t IARM_Bus_Init(const char *name) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Term(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Connect(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Disconnect(void) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_GetContext(void **context) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_BroadcastEvent(const char *ownerName, IARM_EventId_t eventId, void *data, size_t len) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_IsConnected(const char *memberName, int *isRegistered) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RegisterEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char *ownerName, IARM_EventId_t eventId) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RemoveEventHandler(const char *ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RegisterCall(const char *methodName, IARM_BusCall_t handler) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Call(const char *ownerName, const char *methodName, void *arg, size_t argLen) {
    if ((strcmp(ownerName, IARMBUS_AUDIOCAPTUREMGR_NAME) == 0)) {
        auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
        param->result = IARM_RESULT_SUCCESS;
    }

    return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_Call_with_IPCTimeout(const char *ownerName, const char *methodName, void *arg, size_t argLen, int timeout) { return IARM_RESULT_SUCCESS; }

IARM_Result_t IARM_Bus_RegisterEvent(IARM_EventId_t maxEventId) { return IARM_RESULT_SUCCESS; }

void IARM_Bus_WritePIDFile(const char *path) {}
