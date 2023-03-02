#pragma once

#include <gmock/gmock.h>
#include "Operands.h"

class MessageEncoderMock : public MessageEncoderImpl{
public:
    virtual ~MessageEncoderMock() = default;

    MOCK_METHOD(CECFrame&, encode, (const DataBlock &m), (const, override));
};
