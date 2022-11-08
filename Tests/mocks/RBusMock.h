#pragma once

#include <gmock/gmock.h>

class RBusApiMock {
public:
    static RBusApiMock* instance{ nullptr };
    RBusApiMock()
    {
        instance = this;
    }
    virtual ~RBusApiMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(rbusError_t, rbus_open, (rbusHandle_t * handle, char const* componentName));
    MOCK_METHOD(rbusError_t, rbus_setBoolean, (rbusHandle_t handle, char const* paramName, bool paramVal));
    MOCK_METHOD(rbusError_t, rbus_close, (rbusHandle_t handle));

    static rbusError_t _rbus_open(rbusHandle_t* handle, char const* componentName)
    {
        return instance->rbus_open(handle, componentName);
    }

    static rbusError_t _rbus_setBoolean(rbusHandle_t handle, char const* paramName, bool paramVal)
    {
        return instance->rbus_setBoolean(handle, paramName, paramVal);
    }

    static rbusError_t _rbus_close(rbusHandle_t handle)
    {
        return instance->rbus_close(handle);
    }
};

constexpr auto rbus_open = &RBusApiMock::_rbus_open;
constexpr auto rbus_setBoolean = &RBusApiMock::_rbus_setBoolean;
constexpr auto rbus_close = &RBusApiMock::_rbus_close;
