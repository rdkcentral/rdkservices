/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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
#include "MiracastLogger.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

namespace MIRACAST
{
    static inline void sync_stdout()
    {
        if (getenv("SYNC_STDOUT"))
            setvbuf(stdout, NULL, _IOLBF, 0);
    }

    const char *methodName(const std::string &prettyFunction)
    {
        size_t colons = prettyFunction.find("::");
        size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
        size_t end = prettyFunction.rfind("(") - begin;

        return prettyFunction.substr(begin, end).c_str();
    }

    static int gDefaultLogLevel = VERBOSE_LEVEL;
    static FILE *logger_file_ptr = nullptr;

    void logger_init()
    {
        sync_stdout();
        const char *level = getenv("MIRACAST_DEFAULT_LOG_LEVEL");
        if (level){
            gDefaultLogLevel = static_cast<LogLevel>(atoi(level));
        }
        const char *separate_logger = getenv("MIRACAST_SEPARATE_LOGGER_ENABLED");
        if ((nullptr != separate_logger)&&(std::string(separate_logger) == "Yes")){
            logger_file_ptr = fopen("/opt/logs/miracast.log", "a");
        }
    }

    void logger_deinit()
    {
        if (nullptr != logger_file_ptr){
            fclose(logger_file_ptr);
            logger_file_ptr = nullptr;
        }
    }

    void log(LogLevel level,
             const char *func,
             const char *file,
             int line,
             int threadID,
             const char *format, ...)
    {
        const char *levelMap[] = {"FATAL", "ERROR", "WARN", "INFO", "VERBOSE", "TRACE"};
        const short kFormatMessageSize = 4096;
        char formatted[kFormatMessageSize];

        if (((MIRACAST::FATAL_LEVEL != level)&&(MIRACAST::ERROR_LEVEL != level))&&
            (gDefaultLogLevel < level)){
                return;
        }

        va_list argptr;
        va_start(argptr, format);
        vsnprintf(formatted, kFormatMessageSize, format, argptr);
        va_end(argptr);

        if (nullptr!=logger_file_ptr)
        {
            char timestamp[0xFF] = {0};
            struct timespec spec;
            struct tm tm;

            clock_gettime(CLOCK_REALTIME, &spec);
            gmtime_r(&spec.tv_sec, &tm);
            long ms = spec.tv_nsec / 1.0e6;

            sprintf(timestamp, "%02d%02d%02d-%02d:%02d:%02d.%03ld",
                    tm.tm_year % 100,
                    tm.tm_mon + 1,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec,
                    ms);

            if (threadID)
            {
                fprintf(logger_file_ptr, "%s [%s] [tid=%d] %s:%s:%d %s\n",
                        timestamp,
                        levelMap[static_cast<int>(level)],
                        threadID,
                        func, basename(file), line,
                        formatted);
            }
            else
            {
                fprintf(logger_file_ptr, "%s [%s] %s:%s:%d %s\n",
                        timestamp,
                        levelMap[static_cast<int>(level)],
                        func, basename(file), line,
                        formatted);
            }

            fflush(logger_file_ptr);
        }
        else{
            fprintf(stderr, "[MiracastTestApp][%d] %s [%s:%d] %s: %s \n",
                    (int)syscall(SYS_gettid),
                    levelMap[static_cast<int>(level)],
                    basename(file),
                    line,
                    func,
                    formatted);

             fflush(stderr);
        }
    }
} // namespace MIRACAST
