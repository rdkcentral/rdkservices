#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class DeviceTypeMock : public DeviceTypeImpl {
public:
    virtual ~DeviceTypeMock() = default;

    MOCK_METHOD(std::string, toString, (), (const, override));
    

};

