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
    void    selectPort               (int8_t Port) const;
    void    scaleVideo               (int32_t x, int32_t y, int32_t width, int32_t height) const;
	void getEDIDBytesInfo (int iHdmiPort, std::vector<uint8_t> &edid) const;
	void getHDMISPDInfo (int iHdmiPort, std::vector<uint8_t> &data);
    void setEdidVersion (int iHdmiPort, int iEdidVersion);
    void getEdidVersion (int iHdmiPort, int *iEdidVersion);
	void getHdmiALLMStatus (int iHdmiPort, bool *allmStatus);
	void getSupportedGameFeatures (std::vector<std::string> &featureList);
};

}
