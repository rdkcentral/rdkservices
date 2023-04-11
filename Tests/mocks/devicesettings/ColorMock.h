#pragma once

#include <gmock/gmock.h>

class ColorMock : public device::FrontPanelIndicator::ColorImpl {
public:
    virtual ~ColorMock() = default;

    MOCK_METHOD(const device::FrontPanelIndicator::Color&, getInstanceById, (int id), (override));
    MOCK_METHOD(const device::FrontPanelIndicator::Color&, getInstanceByName, (const std::string& name), (override));
    MOCK_METHOD(std::string,getName, (), (const, override));

};
