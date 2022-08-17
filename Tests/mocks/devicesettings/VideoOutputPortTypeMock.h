#pragma once

#include <gmock/gmock.h>

#include "videoOutputPortType.hpp"

class VideoOutputPortTypeMock : public device::VideoOutputPortTypeImpl {
public:
    virtual ~VideoOutputPortTypeMock() = default;

    MOCK_METHOD(int, getId, (), (const, override));
    MOCK_METHOD(const device::List<device::VideoResolution>, getSupportedResolutions, (), (const, override));
};
