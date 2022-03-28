#pragma once

#include <gmock/gmock.h>

#include "host.hpp"

class HostImplMock : public device::HostImpl {
public:
    virtual ~HostImplMock() = default;

    MOCK_METHOD(device::List<std::reference_wrapper<device::VideoDevice>>, getVideoDevices, (), (override));
};
