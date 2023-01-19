#pragma once

#include <gmock/gmock.h>

class FrontPanelIndicatorMock : public device::FrontPanelIndicator::FrontPanelIndicatorImpl {
public:
    virtual ~FrontPanelIndicatorMock() = default;

    MOCK_METHOD(device::FrontPanelIndicator&, getInstanceString, (const std::string& name), (override));
    MOCK_METHOD(device::FrontPanelIndicator&, getInstanceInt, (int id), (override));
    MOCK_METHOD(void,setState, (const bool bState), (const, override));
    MOCK_METHOD(std::string,getName, (), (const, override));
    MOCK_METHOD(void,setBrightness, (const int brightness, const bool toPersist), (const, override));
    MOCK_METHOD(int,getBrightness, (), (const, override));
    MOCK_METHOD(void,setColor, (const device::FrontPanelIndicator::Color &newColor, const bool toPersist), (const, override));
    MOCK_METHOD(void,setColorInt, (const uint32_t color, const bool toPersist), (const, override));
    MOCK_METHOD(void,getBrightnessLevels, (int &levels,int &min,int &max), (const, override));
    MOCK_METHOD(int,getColorMode, (), (const, override));
    MOCK_METHOD(std::string,getColorName, (), (const, override));
    MOCK_METHOD(device::List<device::FrontPanelIndicator::Color>,getSupportedColors, (), (const, override));
};
