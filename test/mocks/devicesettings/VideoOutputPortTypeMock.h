#pragma once

#include <gmock/gmock.h>

#include "videoOutputPortType.hpp"

class VideoOutputPortTypeMock : public device::VideoOutputPortType {
public:
    virtual ~VideoOutputPortTypeMock() = default;

    MOCK_METHOD(int, getId, (), (const, override));
    MOCK_METHOD(const device::List<std::reference_wrapper<device::VideoResolution>>, getSupportedResolutions, (), (const, override));
};
