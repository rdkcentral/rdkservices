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

extern rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName);
extern rbusError_t rbus_setBoolean(rbusHandle_t handle, char const* paramName, bool paramVal);
extern rbusError_t rbus_close(rbusHandle_t handle);
