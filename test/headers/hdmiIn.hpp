#pragma once

#include <stdint.h>

namespace device {

class HdmiInputImpl {
public:
    virtual ~HdmiInputImpl() = default;

    virtual uint8_t getNumberOfInputs() const = 0;
    virtual bool isPortConnected(int8_t Port) const = 0;
    virtual std::string getCurrentVideoMode() const = 0;
};

class HdmiInput {
public:
    static HdmiInput& getInstance()
    {
        static HdmiInput instance;
        return instance;
    }

    HdmiInputImpl* impl;

    uint8_t getNumberOfInputs() const
    {
        return impl->getNumberOfInputs();
    }
    bool isPortConnected(int8_t Port) const
    {
        return impl->isPortConnected(Port);
    }
    std::string getCurrentVideoMode() const
    {
        return impl->getCurrentVideoMode();
    }
};

}
