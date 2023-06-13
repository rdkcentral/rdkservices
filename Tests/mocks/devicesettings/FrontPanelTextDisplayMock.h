#pragma once

#include <gmock/gmock.h>

class FrontPanelTextDisplayMock : public device::FrontPanelTextDisplayImpl {
public:
    virtual ~FrontPanelTextDisplayMock() = default;
    MOCK_METHOD(device::FrontPanelTextDisplay&, getInstanceByName, (const std::string& name), (override));
    MOCK_METHOD(device::FrontPanelTextDisplay&, getInstanceById, (int id), (override));
    MOCK_METHOD(int,getCurrentTimeFormat, (), (const, override));
    MOCK_METHOD(void,setTimeFormat, (const int iTimeFormat), (const, override));
    MOCK_METHOD(int,getTextBrightness, (), (const, override));
    MOCK_METHOD(void,setTextBrightness, (const int brightness), (const, override));
    MOCK_METHOD(void,setText, (const std::string text), (const, override));
    MOCK_METHOD(void,setMode, (const int mode), (const, override));
};
