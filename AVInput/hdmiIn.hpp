#pragma once

#include <stdint.h>
#include <string>
#include <vector>

static const int8_t HDMI_IN_PORT_NONE = -1;

namespace device
{

    class HdmiInput
    {

    public:
        static HdmiInput &getInstance();

        uint8_t getNumberOfInputs() const;
        bool isPresented() const;
        bool isActivePort(int8_t Port) const;
        int8_t getActivePort() const;
        bool isPortConnected(int8_t Port) const;
        void selectPort(int8_t Port) const;
        void scaleVideo(int32_t x, int32_t y, int32_t width, int32_t height) const;
        void selectZoomMode(int8_t zoomMode) const;
        void pauseAudio() const;
        void resumeAudio() const;
        std::string getCurrentVideoMode() const;
        void getEDIDBytesInfo(int iHdmiPort, std::vector<uint8_t> &edid) const;
        void getHDMISPDInfo(int iHdmiPort, std::vector<uint8_t> &data);
        void setEdidVersion(int iHdmiPort, int iEdidVersion);
        void getEdidVersion(int iHdmiPort, int *iEdidVersion);

    private:
        HdmiInput();          /* default constructor */
        virtual ~HdmiInput(); /* destructor */
    };

} /* namespace device */
