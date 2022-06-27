
/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#ifndef _DS_HDMIIN_HPP_
#define _DS_HDMIIN_HPP_

#include <stdint.h>
#include <vector>

#include "dsTypes.h"
/**
 * @file hdmiIn.hpp
 * @brief Structures and classes for HDMI Input are defined here
 * @ingroup hdmiIn
 */

static const int8_t HDMI_IN_PORT_NONE = -1;

namespace device 
{


/**
 * @class HdmiInput
 * @brief This class manages HDMI Input
 */
class HdmiInput  
{

public:
    static HdmiInput & getInstance();

    uint8_t getNumberOfInputs        () const;
    bool    isPresented              () const;
    bool    isActivePort             (int8_t Port) const;
    int8_t  getActivePort            () const;
    bool    isPortConnected          (int8_t Port) const;
    void    selectPort               (int8_t Port) const;
    void    scaleVideo               (int32_t x, int32_t y, int32_t width, int32_t height) const;
    void    selectZoomMode           (int8_t zoomMode) const;
    void    pauseAudio               () const;
    void    resumeAudio              () const;
    std::string  getCurrentVideoMode () const;
    void getCurrentVideoModeObj (dsVideoPortResolution_t& resolution);
    void getEDIDBytesInfo (int iHdmiPort, std::vector<uint8_t> &edid) const;
    void getHDMISPDInfo (int iHdmiPort, std::vector<uint8_t> &data);
    void setEdidVersion (int iHdmiPort, int iEdidVersion);
    void getEdidVersion (int iHdmiPort, int *iEdidVersion);
    void getHdmiALLMStatus (int iHdmiPort, bool *allmStatus);
    void getSupportedGameFeatures (std::vector<std::string> &featureList);

private:
    HdmiInput ();           /* default constructor */
    virtual ~HdmiInput ();  /* destructor */
};


}   /* namespace device */


#endif /* _DS_HDMIIN_HPP_ */


/** @} */
/** @} */
