#pragma once

#include <gmock/gmock.h>

class FrontPanelIndicatorMock : public device::FrontPanelIndicator::FrontPanelIndicatorImpl {
public:
    virtual ~FrontPanelIndicatorMock() = default;

    MOCK_METHOD(device::FrontPanelIndicator&, getInstanceString, (const std::string& name), (override));
    MOCK_METHOD(device::FrontPanelIndicator&, getInstanceInt, (int id), (override));
    MOCK_METHOD(void,getBrightnessLevels, (int &levels,int &min,int &max), (const, override));
    MOCK_METHOD(int,getColorMode, (), (const, override));
    MOCK_METHOD(void,setBrightness, (const int brightness, bool persist), (const, override));
    MOCK_METHOD(void,setBrightness, (const int brightness), (const, override));
    MOCK_METHOD(int,getBrightness, (), (const, override));
    MOCK_METHOD(std::string,getName, (), (const, override));
    MOCK_METHOD(std::string,getColorName, (), (const, override));
    MOCK_METHOD(void,setState, (const bool &enable), (const, override));
    MOCK_METHOD(void,setColor, (device::FrontPanelIndicator::Color color), (const, override));
    MOCK_METHOD(void,setColor, (device::FrontPanelIndicator::Color color, bool persist), (const, override));
    MOCK_METHOD(device::List<device::FrontPanelIndicator::Color>,getSupportedColors, (), (const, override));
 
};
