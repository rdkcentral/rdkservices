#pragma once

#include <gmock/gmock.h>

#include "videoOutputPort.hpp"

class VideoOutputPortMock : public device::VideoOutputPortImpl {
public:
    virtual ~VideoOutputPortMock() = default;

    MOCK_METHOD(const device::VideoOutputPortType&, getType, (), (const, override));
    MOCK_METHOD(const std::string&, getName, (), (const, override));
    MOCK_METHOD(const device::VideoResolution&, getDefaultResolution, (), (const, override));
    MOCK_METHOD(int, getHDCPProtocol, (), (override));
};
