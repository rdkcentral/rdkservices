#pragma once

#include <gmock/gmock.h>

#include "system.h"

class SystemMock : public systemServ, public systemFile, public systemPower {

     MOCK_METHOD(int, setMode, (const char* mode, const int duration), (const, override));
     MOCK_METHOD(int, getMode, (char* mode, int* duration), (const, override));
     MOCK_METHOD(bool, isGzEnabledHelper, (bool* enabled), (const, override));
     MOCK_METHOD(bool, setGzEnabled,(bool enabled),(const, override));

     MOCK_METHOD(int, setDevicePowerState, (const char* powerState, const char* standbyReason), (const, override));
     MOCK_METHOD(int, getDevicePowerState,(char* powerState),(const,override));
};