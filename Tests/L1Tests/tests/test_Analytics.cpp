#include <gtest/gtest.h>

#include "Analytics.h"

using namespace WPEFramework;

class AnalyticsTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Analytics> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    AnalyticsTest()
        : plugin(Core::ProxyType<Plugin::Analytics>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~AnalyticsTest() = default;
};

TEST_F(AnalyticsTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sendEvent")));
}

TEST_F(AnalyticsTest, sendEvent)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("sendEvent"), _T("{\"eventName\":\"tile_impression\", \"eventVersion\":\"1\", \"eventSource\":\"ImmerseUI\", \"eventSourceVersion\":\"1.2\", \"cetList\":[\"cet1\",\"cet2\",\"cet3\"], \"eventPayload\": { \"event_trigger\": \"user_key_select\"}}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}



