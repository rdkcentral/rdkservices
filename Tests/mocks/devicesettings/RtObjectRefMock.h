#pragma once

#include <gmock/gmock.h>

class rtObjectRefMock : public rtObjectRefImpl {
public:
    virtual ~rtObjectRefMock() = default;

    MOCK_METHOD(rtError, send, (const char* messageName, rtObjectBase& result), (override));


};
