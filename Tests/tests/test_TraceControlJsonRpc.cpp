/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 **/

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include "TraceControl.h"
#include "WrapsMock.h"

#include "ServiceMock.h"
#include "COMLinkMock.h"

using namespace WPEFramework;

namespace {
const string callSign = _T("TraceControl");
const string webPrefix = _T("/Service/TraceControl");
const string volatilePath = _T("/tmp/");
}

class TraceControlJsonRpcTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::TraceControl> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    WrapsImplMock wrapsImplMock;
    string response;

    TraceControlJsonRpcTest()
        : plugin(Core::ProxyType<Plugin::TraceControl>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        Trace::TraceUnit::Instance().Open(volatilePath);
        Wraps::getInstance().impl = &wrapsImplMock;
    }

    virtual ~TraceControlJsonRpcTest()
    {
        Wraps::getInstance().impl = nullptr;
        plugin.Release();
        Trace::TraceUnit::Instance().Close();
    }
};

class TraceControlJsonRpcInitializedTest : public TraceControlJsonRpcTest {
protected:
    ServiceMock service;
    COMLinkMock comLinkMock;

    TraceControlJsonRpcInitializedTest()
        : TraceControlJsonRpcTest()
    {
        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return("{}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
         ON_CALL(service, COMLink())
             .WillByDefault(::testing::Return(&comLinkMock));
    }
    virtual ~TraceControlJsonRpcInitializedTest() override
    {
        plugin->Deinitialize(&service);
    }
};

TEST_F(TraceControlJsonRpcTest, registeredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("set")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("status")));
}

TEST_F(TraceControlJsonRpcInitializedTest, jsonRpc)
{
    ON_CALL(wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))
    .WillByDefault(::testing::Invoke(
        [&](int pri, const char* fmt, va_list args) -> void {
            EXPECT_EQ(LOG_NOTICE, pri);
            va_list args2;
            va_copy(args2, args);
            char strFmt[256];
            vsprintf(strFmt, fmt, args2);
            std::string strFmt_local(strFmt);
            EXPECT_THAT(strFmt_local, ::testing::MatchesRegex("\\[.+\\]: Test1.+"));
            va_end(args2);
        }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{}"), response));

    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                "\"settings\":"
                                                "\\[(\\{\"module\":\"[^\"]+\",\"category\":\"[^\"]+\",\"state\":\"(disabled|enabled|tristated)\"\\},{0,}){0,}\\]"
                                                "\\}"));

    //Set Plugin_TraceControl:Information:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\",\"state\":\"enabled\"}"), response));

    //Get status Plugin_TraceControl:Information:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                "\"settings\":"
                                                "\\[\\{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\",\"state\":\"enabled\"\\}\\]"
                                                "\\}"));

    //Set Plugin_TraceControl:All:disabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"state\":\"disabled\"}"), response));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{\"module\":\"Plugin_TraceControl\"}"), response));
    //Check all categories are set to disabled
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                "\"settings\":"
                                                "\\[(\\{\"module\":\"Plugin_TraceControl\",\"category\":\"[^\"]+\",\"state\":\"disabled\"\\},{0,}){0,}\\]"
                                                "\\}"));

    //Set Plugin_TraceControl:All:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"state\":\"enabled\"}"), response));

     //Log some trace data and verify the output format
    TRACE(Trace::Information, (_T("Test1")));
}

TEST_F(TraceControlJsonRpcInitializedTest, syslogFormat)
{
    ON_CALL(wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))
    .WillByDefault(::testing::Invoke(
        [&](int pri, const char* fmt, va_list args) -> void {
            EXPECT_EQ(LOG_NOTICE, pri);
            va_list args2;
            va_copy(args2, args);
            char strFmt[256];
            vsprintf(strFmt, fmt, args2);
            std::string strFmt_local(strFmt);
            EXPECT_THAT(strFmt_local, ::testing::MatchesRegex("\\[.+\\]:\\[test_TraceControlJsonRpc.cpp:[0-9]+\\] Information: Test2.+"));
            va_end(args2);
        }));

    ON_CALL(service, ConfigLine())
        .WillByDefault(::testing::Return("{\n"
                                            "\"console\":false,\n"
                                            "\"syslog\":true,\n"
                                            "\"abbreviated\":false\n"
                                         "}"));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

     //Log some trace data and verify the output format
    TRACE(Trace::Information, (_T("Test2")));
}
