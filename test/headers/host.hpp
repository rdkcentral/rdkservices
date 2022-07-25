#pragma once

#include <vector>
#include "sleepMode.hpp"
#include "videoOutputPort.hpp"
#include "audioOutputPort.hpp"

namespace device {

template <class T>
using List = std::vector<T>;

class VideoDevice {
public:
    virtual ~VideoDevice() = default;

    virtual int getFRFMode(int* frfmode) const = 0;
    virtual int setFRFMode(int frfmode) const = 0;
    virtual int getCurrentDisframerate(char* framerate) const = 0;
    virtual int setDisplayframerate(const char* framerate) const = 0;

    
};

class systemServ {
public:
    virtual int setMode(const char* mode, const int duration) const = 0 ;
    virtual int getMode(char* mode, int* duration) const = 0 ;

};

class systemFile{
public:
    virtual bool isGzEnabledHelper(bool* enabled) const = 0;
    virtual bool setGzEnabled(bool enabled) const =0;
};

class systemRFC {
    //virtual int setRFC()

};

class systemPower{
    virtual int setDevicePowerState(const char* powerState, const char* standbyReason) const =0;
    virtual int getDevicePowerState(char* powerState) const =0;
};

class systemIARM{


};

class HostImpl {
public:
    virtual ~HostImpl() = default;

    virtual List<std::reference_wrapper<VideoDevice>> getVideoDevices() = 0;
};

class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    HostImpl* impl;

    List<VideoOutputPort> getVideoOutputPorts();
    List<AudioOutputPort> getAudioOutputPorts();

    SleepMode getPreferredSleepMode();
    int setPreferredSleepMode(const SleepMode);
    List <SleepMode>  getAvailableSleepModes();


    List<std::reference_wrapper<VideoDevice>> getVideoDevices()
    {
        return impl->getVideoDevices();
    }
};

}
