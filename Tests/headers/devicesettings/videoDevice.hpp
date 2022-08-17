#pragma once

namespace device {

class VideoDeviceImpl {
public:
    virtual ~VideoDeviceImpl() = default;

    virtual int getFRFMode(int* frfmode) const = 0;
    virtual int setFRFMode(int frfmode) const = 0;
    virtual int getCurrentDisframerate(char* framerate) const = 0;
    virtual int setDisplayframerate(const char* framerate) const = 0;
};

class VideoDevice {
public:
    VideoDeviceImpl* impl;

    int getFRFMode(int* frfmode) const
    {
        return impl->getFRFMode(frfmode);
    }

    int setFRFMode(int frfmode) const
    {
        return impl->setFRFMode(frfmode);
    }

    int getCurrentDisframerate(char* framerate) const
    {
        return impl->getCurrentDisframerate(framerate);
    }

    int setDisplayframerate(const char* framerate) const
    {
        return impl->setDisplayframerate(framerate);
    }
};

}
