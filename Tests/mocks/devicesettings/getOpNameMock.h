#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class getOpNamesImplMock : public getOpNamesImpl {
public:
    virtual ~getOpNamesImplMock() = default;

    MOCK_METHOD(const char*, GetOpName, (Op_t op), (const, override));
};