#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class FrontPanelTextDisplayImplMock : public device::FrontPanelTextDisplayImpl {
public:
    virtual ~FrontPanelTextDisplayImplMock() = default;

    MOCK_METHOD(int,getTextBrightness, (), (const, override));
    MOCK_METHOD(void,setTextBrightness, (const int brightness), (const, override));
    MOCK_METHOD(int,getCurrentTimeFormat, (), (const, override));
    MOCK_METHOD(void,setTimeFormat, (const int iTimeFormat), (const, override));
    MOCK_METHOD(void,setText, (const std::string text), (const, override));
    MOCK_METHOD(void,setMode, (const int mode), (const, override));

    //FrontPanelIndicator mock functions, as TextDisplay extends front panel indicator.
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

