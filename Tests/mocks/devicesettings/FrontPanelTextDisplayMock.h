#pragma once

#include <gmock/gmock.h>

class FrontPanelTextDisplayMock : public device::FrontPanelTextDisplayImpl {
public:
    virtual ~FrontPanelTextDisplayMock() = default;

    MOCK_METHOD(int, getCurrentTimeFormat, (), (override));
    MOCK_METHOD(void, setTimeFormat, (const int iTimeFormat), (override));
};
