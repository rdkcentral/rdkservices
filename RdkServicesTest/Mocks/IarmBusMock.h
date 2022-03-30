#pragma once

#include <gmock/gmock.h>

#include "libIBus.h"

class IarmBusImplMock : public IarmBusImpl {
public:
    virtual ~IarmBusImplMock() = default;

    MOCK_METHOD(IARM_Result_t, IARM_Bus_Init, (const char* name), (override));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Connect, (), (override));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_IsConnected, (const char* memberName, int* isRegistered), (override));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_RegisterEventHandler, (const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler), (override));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_UnRegisterEventHandler, (const char* ownerName, IARM_EventId_t eventId), (override));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Call, (const char *ownerName, const char *methodName, void *arg, size_t argLen), (override));
};
