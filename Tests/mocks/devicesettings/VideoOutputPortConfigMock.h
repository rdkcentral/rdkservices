#pragma once

#include <gmock/gmock.h>

#include "videoOutputPortConfig.hpp"

class VideoOutputPortConfigImplMock : public device::VideoOutputPortConfigImpl {
public:
    virtual ~VideoOutputPortConfigImplMock() = default;

    MOCK_METHOD(device::VideoOutputPortType&, getPortType, (int id), (override));
    MOCK_METHOD(device::VideoOutputPort&, getPort, (const std::string& name), (override));
};
