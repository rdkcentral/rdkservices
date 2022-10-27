#pragma once

#include <gmock/gmock.h>

#include "RBus.h"
//#include "rbus.h"

class RBusApiImplMock : public RBusApiImpl {
public:
    virtual ~RBusApiImplMock() = default;

    MOCK_METHOD(rbusError_t, rbus_open, (rbusHandle_t* handle, char const* componentName), (override));
    MOCK_METHOD(rbusError_t, rbus_setBoolean, (rbusHandle_t handle, char const* paramName, bool paramVal), (override));
    MOCK_METHOD(rbusError_t, rbus_close, (rbusHandle_t handle) , (override));
};
