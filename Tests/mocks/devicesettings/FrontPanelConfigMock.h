#pragma once

#include <gmock/gmock.h>

class FrontPanelConfigMock : public device::FrontPanelConfigImpl {
public:
    virtual ~FrontPanelConfigMock() = default;

    MOCK_METHOD(device::List<device::FrontPanelIndicator>,getIndicators, (), (const, override));
    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (const std::string &name), (const, override));
    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (int id), (const, override));
    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (), (const, override));
    MOCK_METHOD(device::List<device::FrontPanelTextDisplay>,getTextDisplays, (), (const, override));
};

