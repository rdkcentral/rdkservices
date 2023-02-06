#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class OSDNameImplMock : public OSDNameImpl {
public:
    virtual ~OSDNameImplMock() = default;

    MOCK_METHOD(const std::string, toString, (), (const, override));
    
};