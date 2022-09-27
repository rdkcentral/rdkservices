#pragma once

#include "list.hpp"
#include "videoResolution.hpp"

namespace device {

class VideoOutputPortTypeImpl {
public:
    virtual ~VideoOutputPortTypeImpl() = default;

    virtual int getId() const = 0;
    virtual const List<VideoResolution> getSupportedResolutions() const = 0;
};

class VideoOutputPortType {
public:
    VideoOutputPortTypeImpl* impl;

    int getId() const
    {
        return impl->getId();
    }

    const List<VideoResolution> getSupportedResolutions() const
    {
        return impl->getSupportedResolutions();
    }
};

}
