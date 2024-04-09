#include <gtest/gtest.h>

#include "TraceControl.h"

#include "COMLinkMock.h"
#include "ServiceMock.h"
#include "WrapsMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;

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
    WrapsImplMock  *p_wrapsImplMock   = nullptr ;
    string response;

    TraceControlJsonRpcTest()
        : plugin(Core::ProxyType<Plugin::TraceControl>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        Trace::TraceUnit::Instance().Open(volatilePath);
        p_wrapsImplMock  = new testing::NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);
    }

    virtual ~TraceControlJsonRpcTest()
    {
        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }

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
            .WillByDefault(::testing::Return("{\n"
                                             "\"console\":false,\n"
                                             "\"syslog\":true,\n"
                                             "\"abbreviated\":false\n"
                                             "}"));
        ON_CALL(service, WebPrefix())
            .WillByDefault(::testing::Return(webPrefix));
        ON_CALL(service, VolatilePath())
            .WillByDefault(::testing::Return(volatilePath));
        ON_CALL(service, Callsign())
            .WillByDefault(::testing::Return(callSign));
        ON_CALL(service, COMLink())
            .WillByDefault(::testing::Return(&comLinkMock));
        EXPECT_EQ(string(""), plugin->Initialize(&service));
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

/**
 * fails without valgrind
 * 2022-12-22T12:54:08.4645286Z Actual function call count doesn't match EXPECT_CALL(wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))...
2022-12-22T12:54:08.4645396Z          Expected: to be called once
2022-12-22T12:54:08.4645592Z            Actual: never called - unsatisfied and active
2022-12-22T12:54:08.4645786Z [  FAILED  ] TraceControlJsonRpcInitializedTest.jsonRpc (4 ms)
 */
TEST_F(TraceControlJsonRpcInitializedTest, DISABLED_jsonRpc)
{
    EXPECT_CALL(*p_wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Return());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{}"), response));

    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"console\":false,"
                                                  "\"settings\":"
                                                  "\\[(\\{\"module\":\"[^\"]+\",\"category\":\"[^\"]+\",\"state\":\"(disabled|enabled|tristated)\"\\},{0,}){0,}\\]"
                                                  "\\}"));

    //Set Plugin_TraceControl:Information:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\",\"state\":\"enabled\"}"), response));

    //Get status Plugin_TraceControl:Information:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\"}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"console\":false,"
                                                  "\"settings\":"
                                                  "\\[\\{\"module\":\"Plugin_TraceControl\",\"category\":\"Information\",\"state\":\"enabled\"\\}\\]"
                                                  "\\}"));

    //Set Plugin_TraceControl:All:disabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"state\":\"disabled\"}"), response));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("status"), _T("{\"module\":\"Plugin_TraceControl\"}"), response));
    //Check all categories are set to disabled
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{"
                                                  "\"console\":false,"
                                                  "\"settings\":"
                                                  "\\[(\\{\"module\":\"Plugin_TraceControl\",\"category\":\"[^\"]+\",\"state\":\"disabled\"\\},{0,}){0,}\\]"
                                                  "\\}"));

    //Set Plugin_TraceControl:All:enabled
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("set"), _T("{\"module\":\"Plugin_TraceControl\",\"state\":\"enabled\"}"), response));

    //Log some trace data and verify the output format
    TRACE(Trace::Information, (_T("Test1")));
}

/**
 * fails without valgrind
 * 2022-12-22T12:54:08.4661163Z Actual function call count doesn't match EXPECT_CALL(wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))...
2022-12-22T12:54:08.4661270Z          Expected: to be called once
2022-12-22T12:54:08.4661458Z            Actual: never called - unsatisfied and active
2022-12-22T12:54:08.4661670Z [  FAILED  ] TraceControlJsonRpcInitializedTest.syslogFormat (5 ms)
 */
TEST_F(TraceControlJsonRpcInitializedTest, DISABLED_syslogFormat)
{
    EXPECT_CALL(*p_wrapsImplMock, syslog(::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke(
            [&](int pri, const char* fmt, va_list args) -> void {
                EXPECT_EQ(LOG_NOTICE, pri);
                va_list args2;
                va_copy(args2, args);
                char strFmt[256];
                vsnprintf(strFmt, sizeof(strFmt), fmt, args2);
                EXPECT_THAT(std::string(strFmt), ::testing::MatchesRegex("\\[.+\\]:\\[test_TraceControlJsonRpc.cpp:[0-9]+\\] Information: Test2.+"));
                va_end(args2);
            }));

    //Log some trace data and verify the output format
    TRACE(Trace::Information, (_T("Test2")));
}
