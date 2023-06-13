#pragma once

#include <gmock/gmock.h>

#include "RBus.h"

class RBusApiImplMock : public RBusApiImpl {
public:
    virtual ~RBusApiImplMock() = default;

    MOCK_METHOD(rbusError_t, rbus_open, (rbusHandle_t* handle, char const* componentName), (override));
    MOCK_METHOD(rbusError_t, rbusMethod_InvokeAsync, (rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout), (override));
    MOCK_METHOD(rbusValue_t, rbusObject_GetValue, (rbusObject_t object, char const* name), (override));
    MOCK_METHOD(char const*, rbusValue_GetString, (rbusValue_t value, int* len), (override));
    MOCK_METHOD(rbusError_t, rbus_close, (rbusHandle_t handle) , (override));
};
