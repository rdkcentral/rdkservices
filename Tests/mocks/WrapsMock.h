#pragma once

#include <gmock/gmock.h>

#include "Wraps.h"

class WrapsImplMock : public WrapsImpl {
public:
    virtual ~WrapsImplMock() = default;

    MOCK_METHOD(int, system, (const char* command), (override));
    MOCK_METHOD(FILE*, popen, (const char* command, const char* type), (override));
};
