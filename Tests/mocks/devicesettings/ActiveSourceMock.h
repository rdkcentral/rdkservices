#pragma once

#include <gmock/gmock.h>
#include "Operands.h"
class ActiveSourceImplMock : public ActiveSourceImpl {
public:
    virtual ~ActiveSourceImplMock() = default;

    MOCK_METHOD(Op_t, opCode, (), (const, override));
};

