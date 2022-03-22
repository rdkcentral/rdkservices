#pragma once

#include "libIARM.h"

typedef void (*IARM_EventHandler_t)(const char* owner, IARM_EventId_t eventId, void* data, size_t len);

class IarmBusImpl {
public:
    virtual ~IarmBusImpl() = default;

    virtual IARM_Result_t IARM_Bus_Init(const char* name) = 0;
    virtual IARM_Result_t IARM_Bus_Connect() = 0;
    virtual IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered) = 0;
    virtual IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) = 0;
    virtual IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId) = 0;
};

extern IarmBusImpl* gIarmBusImpl;

IARM_Result_t IARM_Bus_Init(const char* name)
{
    return gIarmBusImpl->IARM_Bus_Init(name);
}

IARM_Result_t IARM_Bus_Connect()
{
    return gIarmBusImpl->IARM_Bus_Connect();
}

IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
{
    return gIarmBusImpl->IARM_Bus_IsConnected(memberName, isRegistered);
}

IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
    return gIarmBusImpl->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
}

IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
{
    return gIarmBusImpl->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
}
