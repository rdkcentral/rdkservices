#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class LogicalAddressImplMock : public LogicalAddressImpl {
public:
    virtual ~LogicalAddressImplMock() = default;

    MOCK_METHOD(int, toInt, (), (const, override));
    MOCK_METHOD(int, getType, (), (const, override));
    MOCK_METHOD(std::string, toString, (), (const, override));
    
};


