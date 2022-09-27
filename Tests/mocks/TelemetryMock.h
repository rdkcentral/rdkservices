#pragma once

#include <gmock/gmock.h>

#include "telemetry_busmessage_sender.h"

class TelemetryApiImplMock : public TelemetryApiImpl {
public:
    virtual ~TelemetryApiImplMock() = default;

    MOCK_METHOD(void, t2_init, (char *component), (override));
    MOCK_METHOD(T2ERROR, t2_event_s, (char* marker, char* value), (override));
};
