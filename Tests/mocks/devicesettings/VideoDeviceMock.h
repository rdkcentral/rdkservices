#pragma once

#include <gmock/gmock.h>

#include "host.hpp"

class VideoDeviceMock : public device::VideoDeviceImpl {
public:
    virtual ~VideoDeviceMock() = default;

    MOCK_METHOD(int, getFRFMode, (int* frfmode), (const, override));
    MOCK_METHOD(int, setFRFMode, (int frfmode), (const, override));
    MOCK_METHOD(int, getCurrentDisframerate, (char* framerate), (const, override));
    MOCK_METHOD(int, setDisplayframerate, (const char* framerate), (const, override));
};
