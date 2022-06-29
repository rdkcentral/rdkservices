#include <gtest/gtest.h>

#include "Telemetry.h"
#include "ServiceMock.h"
#include "RfcApiMock.h"
#include "TelemetryMock.h"

#include "FactoriesImplementation.h"

namespace {

const string profileFN = _T("/tmp/DefaultProfile.json");
const string t2PpersistentFolder = _T("/tmp/.t2reportprofiles/");

const uint8_t profileContent[] = "{\"profile\":\"default\"}";
}

using namespace WPEFramework;

class TelemetryTestFixture : public ::testing::Test {
protected:
    
    Core::JSONRPC::Connection connection;
    RfcApiImplMock rfcApiImplMock;
    TelemetryApiImplMock telemetryApiImplMock;

    string response;
    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;

    TelemetryTestFixture()
        : connection(1, 0)
    {
        fprintf(stderr, "TelemetryTestFixture::TelemetryTestFixture()\n");
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual ~TelemetryTestFixture()
    {
        PluginHost::IFactories::Assign(nullptr);
    }

    virtual void SetUp()
    {
        RfcApi::getInstance().impl = &rfcApiImplMock;
        TelemetryApi::getInstance().impl = &telemetryApiImplMock;
    }

    virtual void TearDown()
    {
        RfcApi::getInstance().impl = nullptr;
        TelemetryApi::getInstance().impl = nullptr;
    }
};

TEST_F(TelemetryTestFixture, RegisteredMethods)
{
    EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *component) {
                return;
            }));

    Core::ProxyType<Plugin::Telemetry> plugin(Core::ProxyType<Plugin::Telemetry>::Create());
    Core::JSONRPC::Handler* handler(&((Core::JSONRPC::Handler&)(*(plugin))));

    EXPECT_EQ(Core::ERROR_NONE, handler->Exists(_T("setReportProfileStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler->Exists(_T("logApplicationEvent")));
}

TEST_F(TelemetryTestFixture, InitializeDefaultProfile)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                                "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                                "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));

    EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *component) {
                return;
            }));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_SUCCESS;
            }));

    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();

        Core::File file(profileFN);
        file.Create();
        file.Write(profileContent, sizeof(profileContent));
    }

    Core::ProxyType<Plugin::Telemetry> plugin(Core::ProxyType<Plugin::Telemetry>::Create());
    
    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    // Deinitialize
    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryTestFixture, InitializePersistentFolder)
{
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{"
                                "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                                "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));

    EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *component) {
                return;
            }));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_SUCCESS;
            }));

    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();
        
        Core::File file(t2PpersistentFolder + "SomeReport");
        file.Create();
    }

    Core::ProxyType<Plugin::Telemetry> plugin(Core::ProxyType<Plugin::Telemetry>::Create());
    
    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    // Deinitialize
    plugin->Deinitialize(nullptr);
}


TEST_F(TelemetryTestFixture, Plugin)
{
    EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *component) {
                return;
            }));

    Core::ProxyType<Plugin::Telemetry> plugin(Core::ProxyType<Plugin::Telemetry>::Create());
    Core::JSONRPC::Handler* handler(&((Core::JSONRPC::Handler&)(*(plugin))));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {

                return WDMP_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_SUCCESS;
            }));


    EXPECT_CALL(telemetryApiImplMock, t2_event_s(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* marker, char* value) {
                return T2ERROR_SUCCESS;
            }));

    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"wrongvalue\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
    
    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"STARTED\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));


    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"COMPLETE\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("logApplicationEvent"), _T("{\"eventName\":\"NAME\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("logApplicationEvent"), _T("{\"eventValue\":\"VALUE\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("logApplicationEvent"), _T("{\"eventName\":\"NAME\", \"eventValue\":\"VALUE\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    // Deinitialize
    plugin->Deinitialize(nullptr);
}
