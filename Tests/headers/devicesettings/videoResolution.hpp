#pragma once

#include <string>

namespace device {

class VideoResolution {
public:
    virtual ~VideoResolution() = default;

    virtual const std::string& getName() const = 0;
};

}
