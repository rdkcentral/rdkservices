#include <gtest/gtest.h>

#include "Telemetry.h"

#include "FactoriesImplementation.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class TelemetryTestFixture : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::Telemetry> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;

    TelemetryTestFixtureTestFixture()
        : plugin(Core::ProxyType<Plugin::Telemetry>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~TelemetryTestFixtureTestFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TelemetryTestFixture, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setReportProfileStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("logApplicationEvent")));
}

