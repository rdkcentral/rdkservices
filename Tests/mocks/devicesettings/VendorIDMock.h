#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class VendorIDImplMock : public VendorIDImpl {
public:
    virtual ~VendorIDImplMock() = default;

    MOCK_METHOD(const std::string, toString, (), (const, override));

};