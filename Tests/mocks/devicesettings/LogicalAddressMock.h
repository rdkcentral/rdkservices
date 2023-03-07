#pragma once
#include <gmock/gmock.h>
#include "Operands.h"

class LogicalAddressImplMock : public LogicalAddressImpl {
public:
    virtual ~LogicalAddressImplMock() = default;

    MOCK_METHOD(int, getType, (), (const, override));
};
