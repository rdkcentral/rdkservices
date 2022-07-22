#pragma once

#include <stdint.h>

namespace device {
class CompositeInputImpl {
public:
    virtual ~CompositeInputImpl() = default;
    virtual uint8_t getNumberOfInputs        () const = 0 ;
    virtual bool    isPortConnected          (int8_t Port) const = 0 ;
    virtual void    selectPort               (int8_t Port) const = 0 ;
    virtual void    scaleVideo               (int32_t x, int32_t y, int32_t width, int32_t height) const = 0 ;
};

class CompositeInput {
public:
    static CompositeInput & getInstance()
    {
        static CompositeInput instance;
        return instance;
    }
    
    CompositeInputImpl* impl;

    uint8_t getNumberOfInputs() const
    {
        return impl->getNumberOfInputs();
    }
    bool isPortConnected(int8_t Port) const
    {
        return impl->isPortConnected(Port);
    }
    void selectPort(int8_t Port) const
    {
        return impl->selectPort(Port);
    }
    void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const
    {
        return impl->scaleVideo(x,y,width,height);
    }
};

}
