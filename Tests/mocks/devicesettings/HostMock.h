#pragma once

#include <gmock/gmock.h>

#include "host.hpp"

class HostImplMock : public device::HostImpl {
public:
    virtual ~HostImplMock() = default;

    MOCK_METHOD(device::List<device::VideoOutputPort>, getVideoOutputPorts, (), (override));
    MOCK_METHOD(device::List<device::AudioOutputPort>, getAudioOutputPorts, (), (override));
    MOCK_METHOD(device::List<device::VideoDevice>, getVideoDevices, (), (override));
    MOCK_METHOD(device::VideoOutputPort&, getVideoOutputPort, (const std::string& name), (override));
    MOCK_METHOD(device::AudioOutputPort&, getAudioOutputPort, (const std::string& name), (override));
    MOCK_METHOD(void, getHostEDID, (std::vector<uint8_t> & edid), (const, override));
    MOCK_METHOD(std::string, getDefaultVideoPortName, (), (override));
    MOCK_METHOD(std::string, getDefaultAudioPortName, (), (override));
};
