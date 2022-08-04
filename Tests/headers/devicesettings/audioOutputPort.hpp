#pragma once

#include <string>
#include <vector>

namespace device {

class AudioOutputPortImpl {
public:
    virtual ~AudioOutputPortImpl() = default;

    virtual const std::string& getName() const = 0;
    virtual std::vector<std::string> getMS12AudioProfileList() const = 0;
    virtual void getAudioCapabilities(int* capabilities) = 0;
    virtual void getMS12Capabilities(int* capabilities) = 0;
};

class AudioOutputPort {
public:
    AudioOutputPortImpl* impl;

    const std::string& getName() const
    {
        return impl->getName();
    }

    std::vector<std::string> getMS12AudioProfileList() const
    {
        return impl->getMS12AudioProfileList();
    }

    void getAudioCapabilities(int* capabilities)
    {
        return impl->getAudioCapabilities(capabilities);
    }

    void getMS12Capabilities(int* capabilities)
    {
        return impl->getMS12Capabilities(capabilities);
    }
};

}
