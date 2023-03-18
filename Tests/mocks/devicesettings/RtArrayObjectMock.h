#pragma once

#include <gmock/gmock.h>

class rtArrayObjectMock : public rtArrayObjectImpl {
public:
    virtual ~rtArrayObjectMock() = default;

    MOCK_METHOD(void, pushBack, (const char* v), (const, override));
    MOCK_METHOD(void, pushBack, (rtValue v), (const, override));


};
