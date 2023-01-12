#pragma once

#include <gmock/gmock.h>

class FrontPanelIndicatorMock : public device::FrontPanelIndicator::FrontPanelIndicatorImpl {
public:
    virtual ~FrontPanelIndicatorMock() = default;

    MOCK_METHOD(device::FrontPanelIndicator&, getInstance, (const std::string& name), (override));
    MOCK_METHOD(void, setState, (const bool bState), (const, override));
    MOCK_METHOD(const std::string&, getName, (), (const, override));
    MOCK_METHOD(void, setBrightness, (const int& brightness, const bool toPersist), (override));
    MOCK_METHOD(int, getBrightness, (), (override));
    MOCK_METHOD(void, setColor, (const device::FrontPanelIndicator::Color& newColor, bool toPersist), (override));
    MOCK_METHOD(void, setColor, (const uint32_t color, const bool toPersist), (override));
};
