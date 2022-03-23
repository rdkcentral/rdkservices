#pragma once

#include <gmock/gmock.h>

#include "hdmiIn.hpp"

class HdmiInputImplMock : public device::HdmiInputImpl {
public:
    virtual ~HdmiInputImplMock() = default;

    MOCK_METHOD(uint8_t, getNumberOfInputs, (), (const, override));
    MOCK_METHOD(bool, isPortConnected, (int8_t Port), (const, override));
    MOCK_METHOD(std::string, getCurrentVideoMode, (), (const, override));
};
