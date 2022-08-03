#pragma once

#include "list.hpp"
#include "videoResolution.hpp"

namespace device {

class VideoOutputPortType {
public:
    virtual ~VideoOutputPortType() = default;

    virtual int getId() const = 0;
    virtual const List<std::reference_wrapper<VideoResolution>> getSupportedResolutions() const = 0;
};

}
