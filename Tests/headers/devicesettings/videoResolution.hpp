#pragma once

#include <string>

namespace device {

class VideoResolutionImpl {
public:
    virtual ~VideoResolutionImpl() = default;

    virtual const std::string& getName() const = 0;
};

class VideoResolution {
public:
    VideoResolutionImpl* impl;

    const std::string& getName() const
    {
        return impl->getName();
    }
};

}
