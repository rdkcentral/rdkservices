#pragma once

#include <stdint.h>

namespace device {

class HdmiInput {
public:
    static CompositeInput & getInstance();

	uint8_t getNumberOfInputs        () const;
	bool    isPortConnected          (int8_t Port) const;
	void    selectPort               (int8_t Port) const;
    void    scaleVideo               (int32_t x, int32_t y, int32_t width, int32_t height) const;
};

}