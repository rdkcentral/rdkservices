/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
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
**/

#include "Iarm.h"
#include <gmock/gmock.h>

IarmBusImpl* IarmBus::impl = nullptr;

IarmBus::IarmBus() {}

void IarmBus::setImpl(IarmBusImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

IARM_Result_t IarmBus::IARM_Bus_Init(const char* name)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_Init(name);
}

IARM_Result_t IarmBus::IARM_Bus_Connect()
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_Connect();
}

IARM_Result_t IarmBus::IARM_Bus_IsConnected(const char* memberName, int* isRegistered)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_IsConnected(memberName, isRegistered);
}

IARM_Result_t IarmBus::IARM_Bus_RegisterEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_RegisterEventHandler(ownerName, eventId, handler);
}

IARM_Result_t IarmBus::IARM_Bus_UnRegisterEventHandler(const char* ownerName, IARM_EventId_t eventId)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_UnRegisterEventHandler(ownerName, eventId);
}

IARM_Result_t IarmBus::IARM_Bus_RemoveEventHandler(const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_RemoveEventHandler(ownerName, eventId, handler);
}

IARM_Result_t IarmBus::IARM_Bus_Call(const char* ownerName, const char* methodName, void* arg, size_t argLen)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_Call(ownerName, methodName, arg, argLen);
}

IARM_Result_t IarmBus::IARM_Bus_BroadcastEvent(const char *ownerName, IARM_EventId_t eventId, void *arg, size_t argLen)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_BroadcastEvent(ownerName, eventId, arg, argLen);
}


IARM_Result_t IarmBus::IARM_Bus_RegisterCall(const char* methodName, IARM_BusCall_t handler)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_RegisterCall(methodName, handler);
}

IARM_Result_t IarmBus::IARM_Bus_Call_with_IPCTimeout(const char *ownerName,  const char *methodName, void *arg, size_t argLen, int timeout)
{
    EXPECT_NE(impl, nullptr);
    return impl->IARM_Bus_Call_with_IPCTimeout(ownerName, methodName, arg, argLen, timeout);
}

IARM_Result_t (*IARM_Bus_Init)(const char*) = &IarmBus::IARM_Bus_Init;
IARM_Result_t (*IARM_Bus_Connect)() = &IarmBus::IARM_Bus_Connect;
IARM_Result_t (*IARM_Bus_IsConnected)(const char*,int*) = &IarmBus::IARM_Bus_IsConnected;
IARM_Result_t (*IARM_Bus_RegisterEventHandler)(const char*,IARM_EventId_t,IARM_EventHandler_t) = &IarmBus::IARM_Bus_RegisterEventHandler;
IARM_Result_t (*IARM_Bus_UnRegisterEventHandler)(const char*,IARM_EventId_t) = &IarmBus::IARM_Bus_UnRegisterEventHandler;
IARM_Result_t (*IARM_Bus_RemoveEventHandler)(const char*,IARM_EventId_t,IARM_EventHandler_t) = &IarmBus::IARM_Bus_RemoveEventHandler;
IARM_Result_t (*IARM_Bus_Call)(const char*,const char*,void*,size_t) = &IarmBus::IARM_Bus_Call;
IARM_Result_t (*IARM_Bus_BroadcastEvent)(const char *ownerName, IARM_EventId_t eventId, void *arg, size_t argLen) = &IarmBus::IARM_Bus_BroadcastEvent;
IARM_Result_t (*IARM_Bus_RegisterCall)(const char*,IARM_BusCall_t) = &IarmBus::IARM_Bus_RegisterCall;
IARM_Result_t (*IARM_Bus_Call_with_IPCTimeout)(const char*,const char*,void*,size_t,int) = &IarmBus::IARM_Bus_Call_with_IPCTimeout;
