#pragma once

#include <gmock/gmock.h>

#include "devicesettings.h"

class DisplayMock : public device::DisplayImpl {
public:
    virtual ~DisplayMock() = default;

    MOCK_METHOD(void, getEDIDBytes, (std::vector<uint8_t> &edid), (const, override));
};
