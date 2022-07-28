#pragma once

#include "sleepMode.hpp"
#include "audioOutputPort.hpp"
#include "dsTypes.h"
#include "list.hpp"
#include "videoDevice.hpp"
#include "videoOutputPort.hpp"

namespace device {
	class HostImpl {
public:
    virtual ~HostImpl() = default;

    virtual List<std::reference_wrapper<VideoOutputPort>> getVideoOutputPorts() = 0;
    virtual List<std::reference_wrapper<AudioOutputPort>> getAudioOutputPorts() = 0;
    virtual List<std::reference_wrapper<VideoDevice>> getVideoDevices() = 0;
    virtual VideoOutputPort& getVideoOutputPort(const std::string& name) = 0;
    virtual AudioOutputPort& getAudioOutputPort(const std::string& name) = 0;
    virtual void getHostEDID(std::vector<uint8_t>& edid) const = 0;
    virtual std::string getDefaultVideoPortName() = 0;
    virtual std::string getDefaultAudioPortName() = 0;
    virtual List<SleepMode> getAvailableSleepModes() = 0;
    virtual SleepMode getPreferredSleepMode() = 0;
    virtual int setPreferredSleepMode(const SleepMode) = 0;
};

class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    HostImpl* impl;
    
    SleepMode getPreferredSleepMode()
    {
        return impl->getPreferredSleepMode();
    }

    int setPreferredSleepMode(const SleepMode)
    {
        return impl->setPreferredSleepMode(SleepMode);
    }

    List <SleepMode>  getAvailableSleepModes()
    {
        return impl->getAvailableSleepModes();
    }

    List<std::reference_wrapper<VideoOutputPort>> getVideoOutputPorts()
    {
        return impl->getVideoOutputPorts();
    }

    List<std::reference_wrapper<AudioOutputPort>> getAudioOutputPorts()
    {
        return impl->getAudioOutputPorts();
    }

    List<std::reference_wrapper<VideoDevice>> getVideoDevices()
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
