/* If not stated otherwise in this file or this component's LICENSE file the
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
*/

#ifndef THUNDER_TEST_BASE_H
#define THUNDER_TEST_BASE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <core/core.h>
#include <core/JSONRPC.h>
#include <com/com.h>
#include <protocols/JSONRPCLink.h>
#include <core/Portability.h>
#include <condition_variable>

using namespace WPEFramework;

/* Logging macros. */
#define THUNDER_TEST_SHORT_FILE (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define THUNDER_TEST_LOG_INFO(fmt, ...) ThunderTestBase::Log(ThunderTestBase::LogLevel::kInfo, THUNDER_TEST_SHORT_FILE, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define THUNDER_TEST_LOG_WARN(fmt, ...) ThunderTestBase::Log(ThunderTestBase::LogLevel::kWarn, THUNDER_TEST_SHORT_FILE, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define THUNDER_TEST_LOG_ERROR(fmt, ...) ThunderTestBase::Log(ThunderTestBase::LogLevel::kError, THUNDER_TEST_SHORT_FILE, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


class ThunderTestBase : public ::testing::Test
{
    protected:
        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Invoke a service method
         *
         * @param[in] callsign Service callsign
         * @param[in] method Method name
         * @param[in] params Method parameters
         * @param[out] results Method results
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t InvokeMethod(const char *callsign, const char *method, JsonObject &params, JsonObject &results);

        /**
         * @brief Activate a service plugin
         *
         * @param[in] callsign Service callsign
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t Activate(const char *callsign);

        /**
         * @brief Deactivate a service plugin
         *
         * @param[in] callsign Service callsign
         * @return Zero (Core::ERROR_NONE) on succes or another value on error
         */
        uint32_t Deactivate(const char *callsign);


    public:
        /** @brief Log levels */
        enum class LogLevel
        {
            kError = 0,
            kWarn,
            kInfo
        };

        /**
         * @brief Log a test message
         *
         * @param[in] level Log level
         * @param[in] file Short filename
         * @param[in] function Function name
         * @param[in] line Line number
         * @param[in] format Formatted message
         */
        static void Log(LogLevel level, const char* file, const char* function, int line, const char* format, ...);

    private:
        /** @brief Pipe file to send commands to the WPEFramework process. */
        FILE *m_fp;

};

#endif /* THUNDER_TEST_BASE_H */
