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

        Core::Directory(t2PpersistentFolder.c_str()).Destroy(true);
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
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
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
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
                return;
            }));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Telemetry"));
                EXPECT_EQ(string(pcParameterName), _T("Device.X_RDKCENTRAL-COM_T2.ReportProfiles"));
                EXPECT_EQ(string(pcParameterValue), _T("{\\\"profile\\\":\\\"default\\\"}"));
                EXPECT_EQ(eDataType, WDMP_STRING);

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

TEST_F(TelemetryTestFixture, InitializeDefaultProfileRFCFailure)
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
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
                return;
            }));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
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


TEST_F(TelemetryTestFixture, InitializeZeroSizeDefaultProfile)
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
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
                return;
            }));

    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();

        Core::File file(profileFN);
        file.Create();
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
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
                return;
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
    EXPECT_CALL(service, ConfigLine())
        .Times(1)
        .WillOnce(
            ::testing::Return("{}"));

    EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *component) {
                EXPECT_EQ(string(component), _T("Thunder_Plugins"));
                return;
            }));

    Core::ProxyType<Plugin::Telemetry> plugin(Core::ProxyType<Plugin::Telemetry>::Create());
    Core::JSONRPC::Handler* handler(&((Core::JSONRPC::Handler&)(*(plugin))));

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Telemetry"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.FTUEReport.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("false"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);

                return WDMP_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Telemetry"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.FTUEReport.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("true"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);

                return WDMP_SUCCESS;
            }));


    EXPECT_CALL(telemetryApiImplMock, t2_event_s(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* marker, char* value) {
                EXPECT_EQ(string(marker), _T("NAME"));
                EXPECT_EQ(string(value), _T("VALUE"));
                return T2ERROR_SUCCESS;
            }));

    // Initialize
    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"wrongvalue\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler->Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"STARTED\"}"), response));
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
