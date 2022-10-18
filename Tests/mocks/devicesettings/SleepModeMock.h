#pragma once

#include <gmock/gmock.h>

#include "sleepMode.hpp"

class SleepModeMock : public device::SleepModeImpl {
public:
    virtual ~SleepModeMock() = default;
    MOCK_METHOD(device::List<device::SleepMode>, getSleepModes, (), (override));
    MOCK_METHOD(const std::string&, toString, (), (const, override));
};
