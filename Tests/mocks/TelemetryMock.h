#pragma once

#include <gmock/gmock.h>

class TelemetryApiMock {
public:
    static TelemetryApiMock* instance{ nullptr };
    TelemetryApiMock()
    {
        instance = this;
    }
    virtual ~TelemetryApiMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(void, t2_init, (char* component));
    MOCK_METHOD(T2ERROR, t2_event_s, (char* marker, char* value));

    static void _t2_init(char* component)
    {
        instance->t2_init(component);
    }

    static T2ERROR _t2_event_s(char* marker, char* value)
    {
        return instance->t2_event_s(marker, value);
    }
};

constexpr auto t2_init = &TelemetryApiMock::_t2_init;
constexpr auto t2_event_s = &TelemetryApiMock::_t2_event_s;
