#pragma once

#include "videoOutputPortType.hpp"
#include "videoResolution.hpp"

namespace device {

class VideoOutputPort {
public:
    virtual ~VideoOutputPort() = default;

    virtual const VideoOutputPortType& getType() const = 0;
    virtual const std::string& getName() const = 0;
    virtual const VideoResolution& getDefaultResolution() const = 0;
    virtual int getHDCPProtocol() = 0;
};

}
