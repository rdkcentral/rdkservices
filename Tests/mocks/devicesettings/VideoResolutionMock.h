#pragma once

#include <gmock/gmock.h>

#include "videoResolution.hpp"

class VideoResolutionMock : public device::VideoResolution {
public:
    virtual ~VideoResolutionMock() = default;

    MOCK_METHOD(const std::string&, getName, (), (const, override));
};
