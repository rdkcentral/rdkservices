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

#ifndef NM_LOGGER_H
#define NM_LOGGER_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "Module.h"

namespace NetworkManagerLogger {
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
void Init();

/**
 * @brief To set log level while runtime
 */
void SetLevel(LogLevel level);

/**
 * @brief Log a message
 * The function is defined by logging backend.
 * Currently 2 variants are supported: RDKLOGGER & stdout(default)
 */
void logPrint(LogLevel level, const char* file, const char* func, int line, const char* format, ...) __attribute__ ((format (printf, 5, 6)));


#define NMLOG_TRACE(FMT, ...)   logPrint(NetworkManagerLogger::TRACE_LEVEL, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define NMLOG_VERBOSE(FMT, ...) logPrint(NetworkManagerLogger::VERBOSE_LEVEL, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define NMLOG_INFO(FMT, ...)    logPrint(NetworkManagerLogger::INFO_LEVEL, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define NMLOG_WARNING(FMT, ...) logPrint(NetworkManagerLogger::WARNING_LEVEL, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define NMLOG_ERROR(FMT, ...)   logPrint(NetworkManagerLogger::ERROR_LEVEL, __FILE__, __func__, __LINE__, FMT, ##__VA_ARGS__)
#define NMLOG_FATAL(FMT, ...)   logPrint(NetworkManagerLogger::FATAL_LEVEL, __FILE__,__func__, __LINE__, FMT, ##__VA_ARGS__)

} // namespace NetworkManagerLogger

#endif  // NM_LOGGER_H
