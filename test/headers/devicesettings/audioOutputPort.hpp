#pragma once

#include <string>
#include <vector>

namespace device {

class AudioOutputPort {
public:
    virtual ~AudioOutputPort() = default;

    virtual const std::string& getName() const = 0;
    virtual std::vector<std::string> getMS12AudioProfileList() const = 0;
    virtual void getAudioCapabilities(int* capabilities) = 0;
    virtual void getMS12Capabilities(int* capabilities) = 0;
};

}
