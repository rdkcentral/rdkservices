#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class CECBytesImplMock : public CECBytesImpl {
public:
    virtual ~CECBytesImplMock() = default;

    MOCK_METHOD(const std::string, toString, (), (const, override));
    
};


