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
#if 0
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "CCEC.hpp"

CCEC_BEGIN_NAMESPACE
#define MAX_LOG_BUFF 500


#define __TIMESTAMP() do { /*YYMMDD-HH:MM:SS:usec*/               \
        struct tm __tm;                                             \
        struct timeval __tv;                                        \
        gettimeofday(&__tv, NULL);                                  \
        localtime_r(&__tv.tv_sec, &__tm);                           \
        printf("\r\n%02d%02d%02d-%02d:%02d:%02d:%06d ",                 \
                            __tm.tm_year+1900-2000,                             \
                            __tm.tm_mon+1,                                      \
                            __tm.tm_mday,                                       \
                            __tm.tm_hour,                                       \
                            __tm.tm_min,                                        \
                            __tm.tm_sec,                                        \
                            (int)__tv.tv_usec);                                      \
} while(0)
/*
static const char *logLevel[][2] =
{   {"FATAL","0"},
    {"ERROR","1"},
    {"WARN","2"},
    {"EXP","3"},
    {"NOTICE","4"},
    {"INFO","5"},
    {"DEBUG","6"},
    {"TRACE","7"}
};*/

//char _CEC_LOG_PREFIX[64];
#endif
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

//static int cec_log_level = LOG_INFO;

void check_cec_log_status(void);
void CCEC_LOG(int level,const char *format, ...);
void dump_buffer(unsigned char * buf, int len);
#if 0
void CCEC_LOG(int level, const char * format ...)
{
    if ((level < LOG_MAX) && (level <= cec_log_level))
    {
        char tmp_buff[MAX_LOG_BUFF];
        va_list args;
        va_start(args, format);
        vsnprintf(tmp_buff,MAX_LOG_BUFF-1,format, args);
        va_end(args);
         __TIMESTAMP();printf("[%s]%s", _CEC_LOG_PREFIX, tmp_buff);
    }
}
#endif
//#define CCEC_DBG_PRINTF(x) do{printf x;}while(0)
//#define CCEC_ERR_PRINTF(x) do{printf x;}while(0)
//#define CCEC_EXP_PRINTF(x) do{printf x;}while(0)

#define BYTE_TO_BCD(byte_) (((((byte_) / 10) & 0x0F) << 4) | (((byte_) % 10) & 0x0F))
#define BCD_TO_BYTE(byte_) (((((byte_) & 0xF0) >> 4) * 10) + (((byte_) & 0x0F)))

CCEC_END_NAMESPACE
#endif


/** @} */
/** @} */

