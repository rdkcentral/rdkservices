# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#include <cstdio>
#include <cstdlib>
#include <sys/shm.h>
#include "Module.h"
#include <core/core.h>
#include "thunder_test_base.h"

#ifndef CMAKE_INSTALL_PREFIX
#error CMAKE_INSTALL_PREFIX must be defined
#endif

#define THUNDER_ACCESS                  _T("THUNDER_ACCESS")            /* WPEFramework network address variable. */
#define THUNDER_ADDRESS                 "127.0.0.1"                     /* WPEFramework network address. */
#ifndef THUNDER_PORT
#define THUNDER_PORT                    "9998"                         /* Default WPEFramework port (as a string). */
#endif
#define TEST_CALLSIGN                   _T("thunder_tests.1")           /* Test module callsign. */
#define INVOKE_TIMEOUT                  1000                            /* Method invoke timeout in milliseconds. */

using namespace WPEFramework;

/**
 * @brief L2 tests for the thunder plugins
 */

void ThunderTestBase::SetUp()
{
    char command[512];
    char address[20];

    uint32_t status;

    /* Spawn the Thunder process. */
    snprintf(command, sizeof(command), "WPEFramework -c %s/../etc/WPEFramework/config.json", CMAKE_INSTALL_PREFIX);
    m_fp = popen(command, "w");
    if (nullptr == m_fp)
    {
        printf("Failed to start WPEFramework. Ensure that PATH and LD_LIRARY_PATH are set\n"
               "For example:\n"
               "$ export PATH=`pwd`/install/usr/bin:$PATH\n"
               "$ export LD_LIBRARY_PATH=`pwd`/install/usr/lib:$LD_LIBRARY_PATH\n");
    }
    EXPECT_NE(nullptr, m_fp);

    /* Allow time for the server to start. */
    (void)sleep(1U);

    /* Configure the JSONRPC connection, as the test starts before WPEFramework. */
    snprintf(address, sizeof(address), "%s:%s", THUNDER_ADDRESS, THUNDER_PORT);
    WPEFramework::Core::SystemInfo::SetEnvironment(THUNDER_ACCESS, address);

    /* Activate the SystemService plugin. */
    status = Activate("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);

}

void ThunderTestBase::TearDown()
{
    uint32_t status;
    /* Deactivate the SystemService  plugin. */
    status = Deactivate("org.rdk.System");
    EXPECT_EQ(Core::ERROR_NONE, status);

    if (nullptr != m_fp)
    {
        /* Terminate the Thunder process and wait for it to exit. */
        fprintf(m_fp, "Q\n");
        (void)pclose(m_fp);
        m_fp = nullptr;
    }
}

/**
 * @brief Invoke a service method
 *
 * @param[in] callsign Service callsign
 * @param[in] method Method name
 * @param[in] params Method parameters
 * @param[out] results Method results
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t ThunderTestBase::InvokeMethod(const char *callsign, const char *method, JsonObject &params, JsonObject &results)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(std::string(callsign), TEST_CALLSIGN);
    std::string message;
    std::string reply;
    uint32_t status;

    params.ToString(message);
    THUNDER_TEST_LOG_INFO("Invoking %s.%s, parameters %s", callsign, method, message.c_str());

    results = JsonObject();
    status = jsonrpc.Invoke<JsonObject, JsonObject>(INVOKE_TIMEOUT, std::string(method), params, results);

    results.ToString(reply);
    THUNDER_TEST_LOG_INFO("Status %u, results %s", status, reply.c_str());

    return status;
}

/**
 * @brief Activate a service plugin
 *
 * @param[in] callsign Service callsign
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t ThunderTestBase::Activate(const char *callsign)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    THUNDER_TEST_LOG_INFO("Activating %s", callsign);

    params["callsign"] = callsign;
    status = InvokeMethod("Controller.1", "activate", params, result);

    return status;
}

/**
 * @brief Deactivate a service plugin
 *
 * @param[in] callsign Service callsign
 * @return Zero (Core::ERROR_NONE) on succes or another value on error
 */
uint32_t ThunderTestBase::Deactivate(const char *callsign)
{
    JsonObject params;
    JsonObject result;
    uint32_t status;

    THUNDER_TEST_LOG_INFO("Deactivating %s", callsign);

    params["callsign"] = callsign;
    status = InvokeMethod("Controller.1", "deactivate", params, result);
    return status;
}
/**
 * @brief Log a test message
 *
 * @param[in] level Log level
 * @param[in] file Short filename
 * @param[in] function Function name
 * @param[in] line Line number
 * @param[in] format Formatted message
 */
void ThunderTestBase::Log(LogLevel level, const char* file, const char* function, int line, const char* format, ...)
{
	static std::map<LogLevel, const char *> levelMap =
    {
        {ThunderTestBase::LogLevel::kError, "!ERROR"},
        {ThunderTestBase::LogLevel::kWarn, "!WARN"},
        {ThunderTestBase::LogLevel::kInfo, "!INFO"}
    };
    const char *kSegment = "TEST";
    const short kFormatMessageSize = 4096;
    char formatted[kFormatMessageSize];
    va_list argptr;
    va_start(argptr, format);
    (void)vsnprintf(formatted, kFormatMessageSize, format, argptr);
    va_end(argptr);

    char timestamp[0xFF] = {0};
    struct timespec spec;
    struct tm tm;
    (void)clock_gettime(CLOCK_REALTIME, &spec);
    gmtime_r(&spec.tv_sec, &tm);
    long ms = spec.tv_nsec / 1.0e6;
    (void)sprintf(timestamp, "%02d%02d%02d-%02d:%02d:%02d.%03ld",
        tm.tm_year % 100,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        ms);

    fprintf(stderr, "%s %s\t-%s [P:%08x T:%08x] [M:%s L:%d F:%s] %s\n",
            timestamp,
            levelMap[level],
            kSegment,
            getpid(),
            static_cast<uint32_t>(pthread_self()),
            file,
            line,
            function,
            formatted);
}

