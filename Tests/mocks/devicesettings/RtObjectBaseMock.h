#pragma once

#include <gmock/gmock.h>

class rtObjectBaseMock : public rtObjectBaseImpl {
public:
    virtual ~rtObjectBaseMock() = default;

    MOCK_METHOD(rtError, set, (const char* name, const char* value), (const, override));
    MOCK_METHOD(rtError, set, (const char* name, const rtValue& value), (const, override));
    MOCK_METHOD(rtString, get, (const char* name), (const, override));
    MOCK_METHOD(rtError, sendReturns, (const char* messageName, rtString& result), (const, override));


};
