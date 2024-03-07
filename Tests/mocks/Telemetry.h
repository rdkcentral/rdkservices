#pragma once

typedef enum {
    T2ERROR_SUCCESS,
    T2ERROR_FAILURE,
    T2ERROR_INVALID_PROFILE,
    T2ERROR_PROFILE_NOT_FOUND,
    T2ERROR_PROFILE_NOT_SET,
    T2ERROR_MAX_PROFILES_REACHED,
    T2ERROR_MEMALLOC_FAILED,
    T2ERROR_INVALID_ARGS,
    T2ERROR_INTERNAL_ERROR
} T2ERROR;

class TelemetryApiImpl {
public:
    virtual ~TelemetryApiImpl() = default;

    virtual void t2_init(char* component) = 0;
    virtual T2ERROR t2_event_s(char* marker, char* value) = 0;
};

class TelemetryApi {
protected:
    static TelemetryApiImpl* impl;
public:
    TelemetryApi();
    TelemetryApi(const TelemetryApi &obj) = delete;
    static void setImpl(TelemetryApiImpl* newImpl);
    static void t2_init(char* component);
    static T2ERROR t2_event_s(char* marker, char* value);
};

extern void (*t2_init)(char*);
extern  T2ERROR (*t2_event_s)(char*,char*);

