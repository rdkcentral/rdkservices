#pragma once

#include <gmock/gmock.h>

class SleepModeMock : public device::SleepModeImpl {
public:
    virtual ~SleepModeMock() = default;

    MOCK_METHOD(device::SleepMode&, getInstanceById, (int id), (override));
    MOCK_METHOD(device::SleepMode&, getInstanceByName, (const std::string &name), (override));
    MOCK_METHOD(device::List<device::SleepMode>, getSleepModes, (), (override));
    MOCK_METHOD(const std::string&, toString, (), (const, override));
};
