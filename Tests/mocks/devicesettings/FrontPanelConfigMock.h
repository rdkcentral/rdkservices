#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class FrontPanelConfigImplMock : public device::FrontPanelConfigImpl {
public:
    virtual ~FrontPanelConfigImplMock() = default;

    MOCK_METHOD(device::List<device::FrontPanelIndicator>,getIndicators, (), (const, override));
    MOCK_METHOD(device::List<device::FrontPanelTextDisplay>,getTextDisplays, (), (const, override));


    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (std::string name), (const, override));
    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (int id), (const, override));
    MOCK_METHOD(device::FrontPanelTextDisplay&,getTextDisplay, (), (const, override));
   
};

