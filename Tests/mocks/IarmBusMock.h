#pragma once

#include <gmock/gmock.h>

class IarmBusMock {
public:
    static IarmBusMock* instance{ nullptr };
    IarmBusMock()
    {
        instance = this;

        // Defaults:
        ON_CALL(*this, IARM_Bus_IsConnected(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [](const char*, int* isRegistered) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }));
        ON_CALL(*this, IARM_Bus_Init(::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
        ON_CALL(*this, IARM_Bus_Connect())
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
        ON_CALL(*this, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
        ON_CALL(*this, IARM_Bus_UnRegisterEventHandler(::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
        ON_CALL(*this, IARM_Bus_Call(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
        ON_CALL(*this, IARM_Bus_RegisterCall(::testing::_, ::testing::_))
            .WillByDefault(::testing::Return(IARM_RESULT_SUCCESS));
    }
    virtual ~IarmBusMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(IARM_Result_t, IARM_Bus_Init, (const char* name));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Connect, ());
    MOCK_METHOD(IARM_Result_t, IARM_Bus_IsConnected, (const char* memberName, int* isRegistered));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_RegisterEventHandler, (const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_UnRegisterEventHandler, (const char* ownerName, IARM_EventId_t eventId));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_Call, (const char* ownerName, const char* methodName, void* arg, size_t argLen));
    MOCK_METHOD(IARM_Result_t, IARM_Bus_RegisterCall, (const char* methodName, IARM_BusCall_t handler));

    static IARM_Result_t _IARM_Bus_Init(const char* name)
    {
        return instance->IARM_Bus_Init(name);
    }

    static IARM_Result_t _IARM_Bus_Connect()
    {
        return instance->IARM_Bus_Connect();
    }

    static IARM_Result_t _IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
    {
        return instance->IARM_Bus_IsConnected(memberName, isRegistered);
    }

    static IARM_Result_t _IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
    {
        return instance->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
    }

    static IARM_Result_t _IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
    {
        return instance->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
    }
    static IARM_Result_t _IARM_Bus_Call(const char* ownerName, const char* methodName, void* arg, size_t argLen)
    {
        return instance->IARM_Bus_Call(ownerName, methodName, arg, argLen);
    }

    static IARM_Result_t _IARM_Bus_RegisterCall(const char* methodName, IARM_BusCall_t handler)
    {
        return instance->IARM_Bus_RegisterCall(methodName, handler);
    }
};

constexpr auto IARM_Bus_Init = &IarmBusMock::_IARM_Bus_Init;
constexpr auto IARM_Bus_Connect = &IarmBusMock::_IARM_Bus_Connect;
constexpr auto IARM_Bus_IsConnected = &IarmBusMock::_IARM_Bus_IsConnected;
constexpr auto IARM_Bus_RegisterEventHandler = &IarmBusMock::_IARM_Bus_RegisterEventHandler;
constexpr auto IARM_Bus_UnRegisterEventHandler = &IarmBusMock::_IARM_Bus_UnRegisterEventHandler;
constexpr auto IARM_Bus_Call = &IarmBusMock::_IARM_Bus_Call;
constexpr auto IARM_Bus_RegisterCall = &IarmBusMock::_IARM_Bus_RegisterCall;
