#pragma once

typedef enum _rbusError
{
    RBUS_ERROR_SUCCESS,
    RBUS_ERROR_NOT_INITIALIZED,
    RBUS_ERROR_BUS_ERROR,
} rbusError_t;

struct _rbusHandle
{
};

typedef struct _rbusHandle* rbusHandle_t;

 struct _rbusObject
{
};
typedef struct _rbusObject* rbusObject_t;

struct _rbusValue
{
};
typedef struct _rbusValue* rbusValue_t;

typedef void (*rbusMethodAsyncRespHandler_t)(rbusHandle_t handle, char const* methodName, rbusError_t error, rbusObject_t params);


class RBusApiImpl {
public:
    virtual ~RBusApiImpl() = default;

    virtual rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName) = 0;
    virtual rbusError_t rbusMethod_InvokeAsync(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) = 0;
    virtual rbusValue_t rbusObject_GetValue(rbusObject_t object, char const* name) = 0;
    virtual char const* rbusValue_GetString(rbusValue_t value, int* len) = 0;
    virtual rbusError_t rbus_close(rbusHandle_t handle) = 0;
};

class RBusApi {
public:
    static RBusApi& getInstance()
    {
        static RBusApi instance;
        return instance;
    }

    RBusApiImpl* impl;

    static rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName)
    {
        return getInstance().impl->rbus_open(handle, componentName);
    }

    static rbusError_t rbusMethod_InvokeAsync(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout)
    {
        return getInstance().impl->rbusMethod_InvokeAsync(handle, methodName, inParams, callback, timeout);
    }

    static rbusValue_t rbusObject_GetValue(rbusObject_t object, char const* name)
    {
        return getInstance().impl->rbusObject_GetValue(object, name);
    }

    static char const* rbusValue_GetString(rbusValue_t value, int* len)
    {
        return getInstance().impl->rbusValue_GetString(value, len);
    }

    static rbusError_t rbus_close(rbusHandle_t handle)
    {
        return getInstance().impl->rbus_close(handle);
    }
};

constexpr auto rbus_open = &RBusApi::rbus_open;
constexpr auto rbusMethod_InvokeAsync = &RBusApi::rbusMethod_InvokeAsync;
constexpr auto rbusObject_GetValue = &RBusApi::rbusObject_GetValue;
constexpr auto rbusValue_GetString = &RBusApi::rbusValue_GetString;
constexpr auto rbus_close = &RBusApi::rbus_close;
