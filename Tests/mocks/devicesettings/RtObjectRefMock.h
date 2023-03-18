#pragma once

#include <gmock/gmock.h>

class rtObjectRefMock : public rtObjectRefImpl {
public:
    virtual ~rtObjectRefMock() = default;

    MOCK_METHOD(rtError, send, (const char* messageName), (override));
    MOCK_METHOD(rtError, send, (const char* messageName, const char* method, rtFunctionCallback* callback), (override));
    MOCK_METHOD(rtError, send, (const char* messageName, const rtValue& arg1), (override));
    MOCK_METHOD(rtError, send, (const char* messageName, rtObjectRef& base), (override));


};
