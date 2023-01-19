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


#ifndef HDMI_CCEC_UTIL_HPP_
#define HDMI_CCEC_UTIL_HPP_

#include "CCEC.hpp"

CCEC_BEGIN_NAMESPACE
class CECFrame;

class Noop
{
public:
	inline size_t deserialize(const CECFrame &frame, size_t startPos = 0) {
		/* Noop */
		return 0;
	}
    inline CECFrame &serialize(CECFrame &frame) const {
		/* Noop */
    	return frame;
    }

    void print(void) const {

    }
};


#define LOG_FATAL 0 
#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_EXP 3
#define LOG_NOTICE 4
#define LOG_INFO 5
#define LOG_DEBUG 6
#define LOG_TRACE 7
#define LOG_MAX 8


void check_cec_log_status(void);
void CCEC_LOG(int level,const char *format, ...);
void dump_buffer(unsigned char * buf, int len);

//#define CCEC_DBG_PRINTF(x) do{printf x;}while(0)
//#define CCEC_ERR_PRINTF(x) do{printf x;}while(0)
//#define CCEC_EXP_PRINTF(x) do{printf x;}while(0)

#define BYTE_TO_BCD(byte_) (((((byte_) / 10) & 0x0F) << 4) | (((byte_) % 10) & 0x0F))
#define BCD_TO_BYTE(byte_) (((((byte_) & 0xF0) >> 4) * 10) + (((byte_) & 0x0F)))

CCEC_END_NAMESPACE
#endif


/** @} */
/** @} */

