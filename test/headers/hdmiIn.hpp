#pragma once

#include <stdint.h>

namespace device {

class HdmiInputImpl {
public:
    virtual ~HdmiInputImpl() = default;

    virtual uint8_t getNumberOfInputs() const = 0;
    virtual bool isPortConnected(int8_t Port) const = 0;
    virtual std::string getCurrentVideoMode() const = 0;
    virtual void selectPort (int8_t Port) const = 0;
    virtual void scaleVideo (int32_t x, int32_t y, int32_t width, int32_t height) const = 0;
    virtual void getEDIDBytesInfo (int iHdmiPort, std::vector<uint8_t> &edid) const = 0;
    virtual void getHDMISPDInfo (int iHdmiPort, std::vector<uint8_t> &data) = 0;
    virtual void setEdidVersion (int iHdmiPort, int iEdidVersion) = 0;
    virtual void getEdidVersion (int iHdmiPort, int *iEdidVersion) = 0;
    virtual void getHdmiALLMStatus (int iHdmiPort, bool *allmStatus) = 0;
    virtual void getSupportedGameFeatures (std::vector<std::string> &featureList) = 0;
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
    void selectPort (int8_t Port) const
    {
        return impl->selectPort(Port);
    }
    void scaleVideo (int32_t x, int32_t y, int32_t width, int32_t height) const
    {
        return impl->scaleVideo (x, y, width, height);
    }
    void getEDIDBytesInfo (int iHdmiPort, std::vector<uint8_t> &edid) const
    {
        return impl->getEDIDBytesInfo(iHdmiPort, edid);
    }
    void getHDMISPDInfo (int iHdmiPort, std::vector<uint8_t> &data)
    {
        return impl->getHDMISPDInfo(iHdmiPort, data);
    }
    void setEdidVersion (int iHdmiPort, int iEdidVersion)
    {
        return impl->setEdidVersion(iHdmiPort, iEdidVersion);
    }
    void getEdidVersion (int iHdmiPort, int *iEdidVersion)
    {
        return impl->getEdidVersion(iHdmiPort,iEdidVersion);
    }
    void getHdmiALLMStatus (int iHdmiPort, bool *allmStatus)
    {
        return impl->getHdmiALLMStatus(iHdmiPort, allmStatus);
    }
    void getSupportedGameFeatures (std::vector<std::string> &featureList)
    {
        return impl->getSupportedGameFeatures(featureList);
    }
};

}
