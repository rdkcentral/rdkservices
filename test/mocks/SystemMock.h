#pragma once

#include <gmock/gmock.h>

#include "system.h"

class SystemMock : public systemServ {

     
     MOCK_METHOD(bool, isGzEnabledHelper, (bool* enabled), (override));
     MOCK_METHOD(bool, setGzEnabled,(bool enabled),(override));

     MOCK_METHOD(int, setDevicePowerState, (const char* powerState, const char* standbyReason), ( override));
     MOCK_METHOD(int, getDevicePowerState,(char* powerState),(override));
};