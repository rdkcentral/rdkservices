#pragma once

#include <gmock/gmock.h>

#include "audioOutputPort.hpp"

class AudioOutputPortMock : public device::AudioOutputPortImpl {
public:
    virtual ~AudioOutputPortMock() = default;

    MOCK_METHOD(const std::string&, getName, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, getMS12AudioProfileList, (), (const, override));
    MOCK_METHOD(void, getAudioCapabilities, (int* capabilities), (override));
    MOCK_METHOD(void, getMS12Capabilities, (int* capabilities), (override));
};
