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
#include <semaphore.h>
#include <sys/time.h>

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

    static int gDefaultLogLevel = INFO_LEVEL;
    static FILE *logger_file_ptr = nullptr;
    static std::string service_name = "NOT-DEFINED";
    static sem_t separate_logger_sync;

    void logger_init(const char* module_name)
    {
        const char *separate_logger = getenv("MIRACAST_SEPARATE_LOGGER_ENABLED");
        const char *level = getenv("MIRACAST_DEFAULT_LOG_LEVEL");

        sync_stdout();
        sem_init(&separate_logger_sync, 0, 1);

        if (level)
        {
            gDefaultLogLevel = static_cast<LogLevel>(atoi(level));
        }

        if ((nullptr != separate_logger)&&(std::string(separate_logger) == "Yes"))
        {
            std::string logger_filename = "/opt/logs/";
            logger_filename.append(module_name);
            logger_filename.append(".log");
            logger_file_ptr = fopen( logger_filename.c_str() , "a");
        }

        if ( nullptr != module_name )
        {
            service_name = module_name;
        }
    }

    void logger_deinit()
    {
        if (nullptr != logger_file_ptr)
        {
            sem_wait(&separate_logger_sync);
            fclose(logger_file_ptr);
            logger_file_ptr = nullptr;
            sem_post(&separate_logger_sync);
        }
        sem_destroy(&separate_logger_sync);
    }

    void enable_separate_logger( std::string filename )
    {
        sem_wait(&separate_logger_sync);
        if (nullptr != logger_file_ptr)
        {
            fclose(logger_file_ptr);
            logger_file_ptr = nullptr;
	}

	if ( filename.empty())
	{
	    filename = service_name;
	}

	std::string logger_filename = "/opt/logs/";
	logger_filename.append(filename);
	logger_filename.append(".log");
	logger_file_ptr = fopen( logger_filename.c_str() , "a");
        sem_post(&separate_logger_sync);
    }

    void disable_separate_logger( void )
    {
        sem_wait(&separate_logger_sync);
        if (nullptr != logger_file_ptr)
        {
            fclose(logger_file_ptr);
            logger_file_ptr = nullptr;
        }
        sem_post(&separate_logger_sync);
    }

    void set_loglevel(LogLevel level)
    {
        gDefaultLogLevel = level;
    }

    void current_time(char *time_str)
    {
	    struct timeval tv;
	    gettimeofday(&tv, NULL);

	    long microseconds = tv.tv_usec;

	    // Convert time to human-readable format
	    struct tm *tm_info;
	    tm_info = localtime(&tv.tv_sec);

	    sprintf(time_str, "%02d:%02d:%02d:%06ld", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, microseconds);
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

	sem_wait(&separate_logger_sync);
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
	#ifdef UNIT_TESTING
		char time[24] = {0};
		current_time(time);
		fprintf(stderr, "[%s][%d] %s [%s:%d:%s] %s: %s \n",
                    service_name.c_str(),
                    (int)syscall(SYS_gettid),
                    levelMap[static_cast<int>(level)],
                    basename(file),
                    line,
		    time,
                    func,
                    formatted);
	#else
            fprintf(stderr, "[%s][%d] %s [%s:%d] %s: %s \n",
                    service_name.c_str(),
                    (int)syscall(SYS_gettid),
                    levelMap[static_cast<int>(level)],
                    basename(file),
                    line,
                    func,
                    formatted);
	#endif

             fflush(stderr);
        }
	sem_post(&separate_logger_sync);
    }
} // namespace MIRACAST
