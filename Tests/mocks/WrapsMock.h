#pragma once

#include <gmock/gmock.h>

#include "Wraps.h"

class WrapsImplMock : public WrapsImpl {
public:
    virtual ~WrapsImplMock() = default;

    MOCK_METHOD(int, system, (const char* command), (override));
    MOCK_METHOD(FILE*, popen, (const char* command, const char* type), (override));
    MOCK_METHOD(void, syslog, (int pri, const char* fmt, va_list args), (override));
    MOCK_METHOD(FILE*, v_secure_popen, (const char *direction, const char *command, va_list args), (override));
    MOCK_METHOD(int, v_secure_pclose, (FILE *file), (override));
    MOCK_METHOD(int, v_secure_pclose, (FILE *file), (override));
};
