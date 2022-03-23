#include "hdmiIn.hpp"

namespace device
{

    HdmiInput &HdmiInput::getInstance()
    {
        static HdmiInput hdmiInput;
        return hdmiInput;
    }

    uint8_t HdmiInput::getNumberOfInputs() const
    {
        return 1;
    }
    bool HdmiInput::isPresented() const
    {
        return true;
    }
    bool HdmiInput::isActivePort(int8_t Port) const
    {
        return true;
    }
    int8_t HdmiInput::getActivePort() const
    {
        return 10;
    }
    bool HdmiInput::isPortConnected(int8_t Port) const
    {
        return true;
    }
    void HdmiInput::selectPort(int8_t Port) const {}
    void HdmiInput::scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const {}
    void HdmiInput::selectZoomMode(int8_t zoomMode) const {}
    void HdmiInput::pauseAudio() const {}
    void HdmiInput::resumeAudio() const {}
    std::string HdmiInput::getCurrentVideoMode() const { return "1080p60fps"; }
    void HdmiInput::getEDIDBytesInfo(int iHdmiPort, std::vector<uint8_t> &edid) const
    {
    }
    void HdmiInput::getHDMISPDInfo(int iHdmiPort, std::vector<uint8_t> &data)
    {
    }
    void HdmiInput::setEdidVersion(int iHdmiPort, int iEdidVersion)
    {
    }
    void HdmiInput::getEdidVersion(int iHdmiPort, int *iEdidVersion)
    {
    }

    HdmiInput::HdmiInput() = default;
    HdmiInput::~HdmiInput() = default;

} /* namespace device */