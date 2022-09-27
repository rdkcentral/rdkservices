#pragma once

typedef enum
{
    T2ERROR_SUCCESS,
    T2ERROR_FAILURE,
    T2ERROR_INVALID_PROFILE,
    T2ERROR_PROFILE_NOT_FOUND,
    T2ERROR_PROFILE_NOT_SET,
    T2ERROR_MAX_PROFILES_REACHED,
    T2ERROR_MEMALLOC_FAILED,
    T2ERROR_INVALID_ARGS,
    T2ERROR_INTERNAL_ERROR
}T2ERROR;

class TelemetryApiImpl {
public:
    virtual ~TelemetryApiImpl() = default;

    virtual void t2_init(char *component) = 0;
    virtual T2ERROR t2_event_s(char* marker, char* value) = 0;
};

class TelemetryApi {
public:
    TelemetryApiImpl* impl;

    static TelemetryApi& getInstance()
    {
        static TelemetryApi instance;
        return instance;
    }

    static void t2_init(char *component)
    {
        if (nullptr != getInstance().impl)
            getInstance().impl->t2_init(component);
        else
            fprintf(stderr, "TelemetryApi impl is null\n");
    }
    
    static T2ERROR t2_event_s(char* marker, char* value)
    {
        return getInstance().impl->t2_event_s(marker, value);
    }
};

constexpr auto t2_init = &TelemetryApi::t2_init;
constexpr auto t2_event_s = &TelemetryApi::t2_event_s;
