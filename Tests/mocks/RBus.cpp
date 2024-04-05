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

#include "RBus.h"
#include <gmock/gmock.h>

RBusApiImpl* RBusApi::impl = nullptr;

RBusApi::RBusApi() {}

void RBusApi::setImpl(RBusApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

rbusError_t RBusApi::rbus_open(rbusHandle_t* handle, char const* componentName)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_open(handle, componentName);
}

rbusError_t RBusApi::rbusMethod_InvokeAsync(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbusMethod_InvokeAsync(handle, methodName, inParams, callback, timeout);
}

rbusValue_t RBusApi::rbusObject_GetValue(rbusObject_t object, char const* name)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbusObject_GetValue(object, name);
}

char const* RBusApi::rbusValue_GetString(rbusValue_t value, int* len)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbusValue_GetString(value, len);
}

rbusError_t RBusApi::rbus_close(rbusHandle_t handle)
{
    EXPECT_NE(impl, nullptr);
    return impl->rbus_close(handle);
}
rbusError_t (*rbus_open)(rbusHandle_t*,char const* ) = &RBusApi::rbus_open;
rbusError_t (*rbusMethod_InvokeAsync)(rbusHandle_t,char const*, rbusObject_t,rbusMethodAsyncRespHandler_t,int)= &RBusApi::rbusMethod_InvokeAsync;
rbusValue_t (*rbusObject_GetValue)(rbusObject_t,char const*) = &RBusApi::rbusObject_GetValue;
char const* (*rbusValue_GetString)(rbusValue_t,int*) = &RBusApi::rbusValue_GetString;
rbusError_t (*rbus_close)(rbusHandle_t) = &RBusApi::rbus_close;
