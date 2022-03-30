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
    virtual IARM_Result_t IARM_Bus_Call(const char *ownerName, const char *methodName, void *arg, size_t argLen) = 0;
};

class IarmBus {
public:
    static IarmBus& getInstance()
    {
        static IarmBus instance;
        return instance;
    }

    IarmBusImpl* impl;

    static IARM_Result_t IARM_Bus_Init(const char* name)
    {
        return getInstance().impl->IARM_Bus_Init(name);
    }

    static IARM_Result_t IARM_Bus_Connect()
    {
        return getInstance().impl->IARM_Bus_Connect();
    }

    static IARM_Result_t IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
    {
        return getInstance().impl->IARM_Bus_IsConnected(memberName, isRegistered);
    }

    static IARM_Result_t IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
    {
        return getInstance().impl->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
    }

    static IARM_Result_t IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
    {
        return getInstance().impl->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
    }
    static IARM_Result_t IARM_Bus_Call(const char *ownerName, const char *methodName, void *arg, size_t argLen)
    {
        return getInstance().impl->IARM_Bus_Call(ownerName, methodName, arg, argLen);
    }

};

constexpr auto IARM_Bus_Init = &IarmBus::IARM_Bus_Init;
constexpr auto IARM_Bus_Connect = &IarmBus::IARM_Bus_Connect;
constexpr auto IARM_Bus_IsConnected = &IarmBus::IARM_Bus_IsConnected;
constexpr auto IARM_Bus_RegisterEventHandler = &IarmBus::IARM_Bus_RegisterEventHandler;
constexpr auto IARM_Bus_UnRegisterEventHandler = &IarmBus::IARM_Bus_UnRegisterEventHandler;
constexpr auto IARM_Bus_Call = &IarmBus::IARM_Bus_Call;
