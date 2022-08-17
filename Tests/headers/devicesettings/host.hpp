#pragma once

#include "audioOutputPort.hpp"
#include "dsTypes.h"
#include "list.hpp"
#include "videoDevice.hpp"
#include "videoOutputPort.hpp"

namespace device {

class HostImpl {
public:
    virtual ~HostImpl() = default;

    virtual List<VideoOutputPort> getVideoOutputPorts() = 0;
    virtual List<AudioOutputPort> getAudioOutputPorts() = 0;
    virtual List<VideoDevice> getVideoDevices() = 0;
    virtual VideoOutputPort& getVideoOutputPort(const std::string& name) = 0;
    virtual AudioOutputPort& getAudioOutputPort(const std::string& name) = 0;
    virtual void getHostEDID(std::vector<uint8_t>& edid) const = 0;
    virtual std::string getDefaultVideoPortName() = 0;
    virtual std::string getDefaultAudioPortName() = 0;
};

class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    HostImpl* impl;

    List<VideoOutputPort> getVideoOutputPorts()
    {
        return impl->getVideoOutputPorts();
    }

    List<AudioOutputPort> getAudioOutputPorts()
    {
        return impl->getAudioOutputPorts();
    }

    List<VideoDevice> getVideoDevices()
    {
        return impl->getVideoDevices();
    }

    VideoOutputPort& getVideoOutputPort(const std::string& name)
    {
        return impl->getVideoOutputPort(name);
    }

    AudioOutputPort& getAudioOutputPort(const std::string& name)
    {
        return impl->getAudioOutputPort(name);
    }

    void getHostEDID(std::vector<uint8_t>& edid) const
    {
        return impl->getHostEDID(edid);
    }

    std::string getDefaultVideoPortName()
    {
        return impl->getDefaultVideoPortName();
    }

    std::string getDefaultAudioPortName()
    {
        return impl->getDefaultAudioPortName();
    }
};

}
