#pragma once

#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"

namespace device {

class VideoOutputPortConfigImpl {
public:
    virtual ~VideoOutputPortConfigImpl() = default;

    virtual VideoOutputPortType& getPortType(int id) = 0;
    virtual VideoOutputPort& getPort(const std::string& name) = 0;
};

class VideoOutputPortConfig {
public:
    static VideoOutputPortConfig& getInstance()
    {
        static VideoOutputPortConfig instance;
        return instance;
    }

    VideoOutputPortConfigImpl* impl;

    VideoOutputPortType& getPortType(int id)
    {
        return impl->getPortType(id);
    }

    VideoOutputPort& getPort(const std::string& name)
    {
        return impl->getPort(name);
    }
};

}
