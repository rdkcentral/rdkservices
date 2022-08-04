#pragma once

#include "videoOutputPortType.hpp"
#include "videoResolution.hpp"

namespace device {

class VideoOutputPortImpl {
public:
    virtual ~VideoOutputPortImpl() = default;

    virtual const VideoOutputPortType& getType() const = 0;
    virtual const std::string& getName() const = 0;
    virtual const VideoResolution& getDefaultResolution() const = 0;
    virtual int getHDCPProtocol() = 0;
};

class VideoOutputPort {
public:
    VideoOutputPortImpl* impl;

    const VideoOutputPortType& getType() const
    {
        return impl->getType();
    }

    const std::string& getName() const
    {
        return impl->getName();
    }

    const VideoResolution& getDefaultResolution() const
    {
        return impl->getDefaultResolution();
    }

    int getHDCPProtocol()
    {
        return impl->getHDCPProtocol();
    }
};

}
