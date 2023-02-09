#pragma once

#include <gmock/gmock.h>

#include "compositeIn.hpp"

class CompositeInputImplMock : public device::CompositeInputImpl {
public:
    virtual ~CompositeInputImplMock() = default;

    MOCK_METHOD(uint8_t, getNumberOfInputs, (), (const, override));
    MOCK_METHOD(bool, isPortConnected, (int8_t Port), (const, override));
    MOCK_METHOD(void, selectPort, (int8_t Port), (const, override));
	MOCK_METHOD(void, scaleVideo, (int32_t x, int32_t y, int32_t width, int32_t height), (const, override));
   
};
