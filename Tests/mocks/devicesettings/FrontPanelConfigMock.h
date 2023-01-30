#pragma once

#include <gmock/gmock.h>

class FrontPanelConfigMock : public device::FrontPanelConfigImpl {
public:
    virtual ~FrontPanelConfigMock() = default;

    MOCK_METHOD(device::List<device::FrontPanelIndicator>, getIndicators, (), (override));
    MOCK_METHOD(device::FrontPanelTextDisplay&, getTextDisplay, (const std::string& name), (override));
};
