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
#ifndef MIRACAST_LOGGER_H
#define MIRACAST_LOGGER_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

namespace MIRACAST {

const char* methodName(const std::string& prettyFunction);
#define __METHOD_NAME__ MIRACAST::methodName(__PRETTY_FUNCTION__)

/**
 * Logging level with an increasing order of refinement
 * (TRACE_LEVEL = Finest logging)
 * It is essental to start with 0 and increase w/o gaps as the value
 * can be used for indexing in a mapping table.
 */
enum LogLevel {FATAL_LEVEL = 0, ERROR_LEVEL, WARNING_LEVEL, INFO_LEVEL, VERBOSE_LEVEL, TRACE_LEVEL};

/**
 * @brief Init logging
 * Should be called once per program run before calling log-functions
 */
void logger_init();

/**
 * @brief DeInit logging
 * Should be called once per program run before calling exit-functions
 */
void logger_deinit();

#define MIRACAST_assert(expr) do { \
                              if ( __builtin_expect(expr, true) ) \
                                {} \
                              else \
                                MIRACASTLOG_ERROR("%s", #expr); \
                            } while (0)

/**
 * @brief Log a message
 * The function is defined by logging backend.
 * Currently 2 variants are supported: MIRACAST_logger (USE_MIRACAST_LOGGER),
 *                                     stdout(default)
 */
void log(LogLevel level,
    const char* func,
    const char* file,
    int line,
    int threadID,
    const char* format, ...);

#ifdef USE_RDK_LOGGER
#define _LOG(LEVEL, FORMAT, ...)          \
    MIRACAST::log(LEVEL,                       \
         __func__, __FILE__, __LINE__, 0, \
         FORMAT,                          \
         ##__VA_ARGS__)
#else
#define _LOG(LEVEL, FORMAT, ...)          \
    MIRACAST::log(LEVEL,                       \
         __func__, __FILE__, __LINE__, syscall(__NR_gettid), \
         FORMAT,                          \
         ##__VA_ARGS__)
#endif

#define MIRACASTLOG_TRACE(FMT, ...)   _LOG(MIRACAST::TRACE_LEVEL, FMT, ##__VA_ARGS__)
#define MIRACASTLOG_VERBOSE(FMT, ...) _LOG(MIRACAST::VERBOSE_LEVEL, FMT, ##__VA_ARGS__)
#define MIRACASTLOG_INFO(FMT, ...)    _LOG(MIRACAST::INFO_LEVEL, FMT, ##__VA_ARGS__)
#define MIRACASTLOG_WARNING(FMT, ...) _LOG(MIRACAST::WARNING_LEVEL, FMT, ##__VA_ARGS__)
#define MIRACASTLOG_ERROR(FMT, ...)   _LOG(MIRACAST::ERROR_LEVEL, FMT, ##__VA_ARGS__)
#define MIRACASTLOG_FATAL(FMT, ...)   _LOG(MIRACAST::FATAL_LEVEL, FMT, ##__VA_ARGS__)

} // namespace MIRACAST

#endif  // MIRACAST_LOGGER_H
