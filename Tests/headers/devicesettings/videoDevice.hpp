#pragma once

namespace device {

class VideoDevice {
public:
    virtual ~VideoDevice() = default;

    virtual int getFRFMode(int* frfmode) const = 0;
    virtual int setFRFMode(int frfmode) const = 0;
    virtual int getCurrentDisframerate(char* framerate) const = 0;
    virtual int setDisplayframerate(const char* framerate) const = 0;
};

}
