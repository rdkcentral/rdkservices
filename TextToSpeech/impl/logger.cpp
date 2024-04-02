/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#include "logger.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <ctime>

#ifdef USE_RDK_LOGGER
#include "rdk_debug.h"
#endif

namespace TTS {

    static inline void sync_stdout()
    {
        if (getenv("SYNC_STDOUT"))
            setvbuf(stdout, NULL, _IOLBF, 0);
    }

    const char* methodName(const std::string& prettyFunction)
    {
        size_t colons = prettyFunction.find("::");
        size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
        size_t end = prettyFunction.rfind("(") - begin;
        const char *prettyFunctionSub = prettyFunction.substr(begin,end).c_str();

        return prettyFunctionSub;
    }

#ifdef USE_RDK_LOGGER

    void logger_init()
    {
        sync_stdout();
        TTS_logger_init("/etc/debug.ini");
    }

    void log(LogLevel level,
            const char* func,
            const char* file,
            int line,
            int, // thread id is already handled by TTS_logger
            const char* format, ...)
    {
        const TTS_LogLevel levelMap[] =
        {TTS_LOG_FATAL, TTS_LOG_ERROR, TTS_LOG_WARN, TTS_LOG_INFO, TTS_LOG_DEBUG, TTS_LOG_TRACE1};

        const short kFormatMessageSize = 4096;
        // TTSLogger is backed with log4c which has its own default level
        // for filtering messages. Therefore, we don't check level here.
        char userFormatted[kFormatMessageSize];
        char finalFormatted[kFormatMessageSize];

        va_list argptr;
        va_start(argptr, format);
        vsnprintf(userFormatted, kFormatMessageSize, format, argptr);
        snprintf(finalFormatted, kFormatMessageSize, "%s:%s:%d %s", func,
                basename(file),
                line,
                userFormatted);
        va_end(argptr);

        // Currently, we use customized layout 'comcast_dated_nocr' in log4c.
        // This layout doesn't have trailing carriage return, so we need
        // to add it explicitly.
        // Once the default layout is used, this addition should be deleted.
        RDK_LOG(levelMap[static_cast<int>(level)],
                "LOG.RDK.TTS",
                "%s\n",
                finalFormatted);

        if (FATAL_LEVEL == level)
            std::abort();
    }

#else

    static int gDefaultLogLevel = INFO_LEVEL;
    // static int gDefaultLogLevel = VERBOSE_LEVEL;

    void logger_init()
    {
        sync_stdout();
        const char* level = getenv("TTS_DEFAULT_LOG_LEVEL");
        if (level)
            gDefaultLogLevel = static_cast<LogLevel>(atoi(level));
    }

    void log(LogLevel level,
            const char* func,
            const char* file,
            int line,
            int threadID,
            const char* format, ...)
    {
        if (gDefaultLogLevel < level)
            return;

        const char* levelMap[] = {"Fatal", "Error", "Warning", "Info", "Verbose", "Trace"};
        const short kFormatMessageSize = 4096;
        char formatted[kFormatMessageSize];

        va_list argptr;
        va_start(argptr, format);
        vsnprintf(formatted, kFormatMessageSize, format, argptr);
        va_end(argptr);

        char timestamp[0xFF] = {0};
        struct timespec spec;
        struct tm tm;

        clock_gettime(CLOCK_REALTIME, &spec);
        gmtime_r(&spec.tv_sec, &tm);
        long ms = spec.tv_nsec / 1.0e6;

        snprintf(timestamp, sizeof(timestamp), "%02d%02d%02d-%02d:%02d:%02d.%03ld",
                tm.tm_year % 100,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                ms);

        if (threadID)
        {
            printf("%s [%s] [pid=%d,tid=%d] %s:%s:%d %s\n",
                    timestamp,
                    levelMap[static_cast<int>(level)],
                    getpid(),
                    threadID,
                    func, basename(file), line,
                    formatted);
        }
        else
        {
            printf("%s [%s] %s:%s:%d %s\n",
                    timestamp,
                    levelMap[static_cast<int>(level)],
                    func, basename(file), line,
                    formatted);
        }

        fflush(stdout);

        if (FATAL_LEVEL == level)
            std::abort();
    }

#endif // USE_TTS_LOGGER

} // namespace TTS
