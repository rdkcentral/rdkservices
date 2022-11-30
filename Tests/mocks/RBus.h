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

class RBusApiImpl {
public:
    virtual ~RBusApiImpl() = default;

    virtual rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName) = 0;
    virtual rbusError_t rbus_setBoolean(rbusHandle_t handle, char const* paramName, bool paramVal) = 0;
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

    static rbusError_t rbus_setBoolean(rbusHandle_t handle, char const* paramName, bool paramVal)
    {
        return getInstance().impl->rbus_setBoolean(handle, paramName, paramVal);
    }

    static rbusError_t rbus_close(rbusHandle_t handle)
    {
        return getInstance().impl->rbus_close(handle);
    }
};

constexpr auto rbus_open = &RBusApi::rbus_open;
constexpr auto rbus_setBoolean = &RBusApi::rbus_setBoolean;
constexpr auto rbus_close = &RBusApi::rbus_close;
