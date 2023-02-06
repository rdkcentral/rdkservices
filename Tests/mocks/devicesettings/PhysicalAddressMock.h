#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class PhysicalAddressImplMock : public PhysicalAddressImpl {
public:
    virtual ~PhysicalAddressImplMock() = default;

    MOCK_METHOD(int, toInt, (), (const, override));
    MOCK_METHOD(int, getType, (), (const, override));

};

