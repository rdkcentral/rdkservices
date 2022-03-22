#pragma once

#include <vector>

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

class HostImpl {
public:
    virtual ~HostImpl() = default;

    virtual List<std::reference_wrapper<VideoDevice>> getVideoDevices() = 0;
};

extern HostImpl* gHostImpl;

class Host {
public:
    static Host& getInstance()
    {
        static Host instance;
        return instance;
    }

    List<std::reference_wrapper<VideoDevice>> getVideoDevices()
    {
        return gHostImpl->getVideoDevices();
    }
};

}
