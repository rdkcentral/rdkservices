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


#ifndef HDMI_CCEC_FRAME_
#define HDMI_CCEC_FRAME_

#include <stdint.h>
#include <stddef.h>
#include <stdexcept>
#include "CCEC.hpp"

CCEC_BEGIN_NAMESPACE

class CECFrame {
    public:
        CECFrame(const uint8_t *buf = NULL, size_t len = 0);
        void reset(void);
        CECFrame subFrame(size_t start, size_t len = 0) const;
        void append(uint8_t byte);
        void append(const uint8_t *buf, size_t len);
        void append(const CECFrame &frame);
        void getBuffer(const uint8_t **buf, size_t *len) const;
        const uint8_t * getBuffer(void) const;
        uint8_t at(size_t i) const;
        size_t length(void) const;
        void hexDump(int level=6) const;
        uint8_t & operator[](size_t i);

        enum {
            MAX_LENGTH = 128,
        };
    private:
        uint8_t buf_[MAX_LENGTH];
        size_t len_;
};

CCEC_END_NAMESPACE

#endif



/** @} */
/** @} */

