/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


/**
* @defgroup hdmicec
* @{
* @defgroup ccec
* @{
**/


#ifndef HDMI_CCEC_OPERAND_HPP_
#define HDMI_CCEC_OPERAND_HPP_

#include <stdint.h>

#include <vector>
#include <string>


#include "CCEC.hpp"
#include "CECFrame.hpp"

CCEC_BEGIN_NAMESPACE

class CECFrame;

class Operand
{
public:
    virtual CECFrame &serialize(CECFrame &frame) const = 0; 

    virtual const std::string toString(void) const {
    	return "Not Implemented";
    }
    virtual const std::string name(void) const {
    	return "Operand";
    }
    virtual bool validate(void) const {
    	return true;
    }
    CECFrame serialize(void) const {
        CECFrame frame;
        return serialize(frame);
    }
};

enum
{
	BROADCAST = (0x01),
	UNICAST   = (0x01 << 1),
};


#define BYTE_TO_BCD(byte_) (((((byte_) / 10) & 0x0F) << 4) | (((byte_) % 10) & 0x0F))
#define BCD_TO_BYTE(byte_) (((((byte_) & 0xF0) >> 4) * 10) + (((byte_) & 0x0F)))

CCEC_END_NAMESPACE
#endif


/** @} */
/** @} */

