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
protected:
    static RBusApiImpl* impl;

public:

    RBusApi();
    RBusApi(const RBusApi &obj) = delete;
    static void setImpl(RBusApiImpl* newImpl);
    static rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName);
    static rbusError_t rbusMethod_InvokeAsync(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout);
    static rbusValue_t rbusObject_GetValue(rbusObject_t object, char const* name);
    static char const* rbusValue_GetString(rbusValue_t value, int* len);
    static rbusError_t rbus_close(rbusHandle_t handle);
};

extern rbusError_t (*rbus_open)(rbusHandle_t*,char const* );
extern rbusError_t (*rbusMethod_InvokeAsync)(rbusHandle_t,char const*, rbusObject_t,rbusMethodAsyncRespHandler_t,int);
extern rbusValue_t (*rbusObject_GetValue)(rbusObject_t,char const*);
extern char const* (*rbusValue_GetString)(rbusValue_t,int*);
extern rbusError_t (*rbus_close)(rbusHandle_t);

