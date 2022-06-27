
/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/


#ifndef _DS_COMPOSITEIN_HPP_
#define _DS_COMPOSITEIN_HPP_

#include <stdint.h>

/**
 * @file compositeIn.hpp
 * @brief Structures and classes for COMPOSITE Input are defined here
 * @ingroup compositeIn
 */

static const int8_t COMPOSITE_IN_PORT_NONE = -1;

namespace device
{


/**
 * @class CompositeInput
 * @brief This class manages COMPOSITE Input
 */
class CompositeInput
{

public:
    static CompositeInput & getInstance();

    uint8_t getNumberOfInputs        () const;
    bool    isPresented              () const;
    bool    isActivePort             (int8_t Port) const;
    int8_t  getActivePort            () const;
    bool    isPortConnected          (int8_t Port) const;
    void    selectPort               (int8_t Port) const;
    void    scaleVideo               (int32_t x, int32_t y, int32_t width, int32_t height) const;

private:
    CompositeInput ();           /* default constructor */
    virtual ~CompositeInput ();  /* destructor */
};


}   /* namespace device */


#endif /* _DS_COMPOSITEIN_HPP_ */


/** @} */
/** @} */
