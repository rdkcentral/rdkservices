#pragma once

#include <gmock/gmock.h>

class SleepModeMock : public device::SleepModeImpl {
public:
    virtual ~SleepModeMock() = default;
    MOCK_METHOD(List<SleepMode>, getSleepModes, (), (override));
    MOCK_METHOD(onst std::string&, toString, (), (const, override));
};
