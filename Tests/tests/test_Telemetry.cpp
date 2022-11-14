#include <gtest/gtest.h>

#include "Telemetry.h"

#include "FactoriesImplementation.h"
#include "RfcApiMock.h"
#include "ServiceMock.h"
#include "TelemetryMock.h"
#include "RBusMock.h"

namespace {
const string profileFN = _T("/tmp/DefaultProfile.json");
const string t2PpersistentFolder = _T("/tmp/.t2reportprofiles/");
const uint8_t profileContent[] = "{\"profile\":\"default\"}";
}

using namespace WPEFramework;

class T2Test : public ::testing::Test {
protected:
    TelemetryApiImplMock telemetryApiImplMock;

    T2Test()
    {
        TelemetryApi::getInstance().impl = &telemetryApiImplMock;

        EXPECT_CALL(telemetryApiImplMock, t2_init(::testing::_))
            .Times(1);
    }
    virtual ~T2Test()
    {
        TelemetryApi::getInstance().impl = nullptr;
    }
};

class TelemetryTest : public T2Test {
protected:
    ServiceMock service;
    Core::ProxyType<Plugin::Telemetry> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    TelemetryTest()
        : T2Test()
        , plugin(Core::ProxyType<Plugin::Telemetry>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
        Core::Directory(t2PpersistentFolder.c_str()).Destroy(true);
    }
    virtual ~TelemetryTest() override = default;
};

class TelemetryRfcTest : public TelemetryTest {
protected:
    RfcApiImplMock rfcApiImplMock;

    TelemetryRfcTest()
        : TelemetryTest()
    {
        RfcApi::getInstance().impl = &rfcApiImplMock;
    }
    virtual ~TelemetryRfcTest() override
    {
        RfcApi::getInstance().impl = nullptr;
    }
};

class TelemetryRBusTest : public TelemetryTest {
protected:
    RBusApiImplMock rBusApiImplMock;

    TelemetryRBusTest()
        : TelemetryTest()
    {
        RBusApi::getInstance().impl = &rBusApiImplMock;
    }
    virtual ~TelemetryRBusTest() override
    {
        RBusApi::getInstance().impl = nullptr;
    }
};

TEST_F(TelemetryTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setReportProfileStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("logApplicationEvent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("uploadReport")));
}

TEST_F(TelemetryRfcTest, InitializeDefaultProfile)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(
            ::testing::Return("{"
                              "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                              "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
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

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRfcTest, InitializeDefaultProfileRFCFailure)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(
            ::testing::Return("{"
                              "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                              "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }));
    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();
        Core::File file(profileFN);
        file.Create();
        file.Write(profileContent, sizeof(profileContent));
    }

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryTest, InitializeZeroSizeDefaultProfile)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(
            ::testing::Return("{"
                              "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                              "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));
    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();
        Core::File file(profileFN);
        file.Create();
    }

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryTest, InitializePersistentFolder)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(
            ::testing::Return("{"
                              "\"t2PersistentFolder\":\"/tmp/.t2reportprofiles/\","
                              "\"defaultProfilesFile\":\"/tmp/DefaultProfile.json\""
                              "}"));
    {
        Core::Directory(t2PpersistentFolder.c_str()).CreatePath();
        Core::File file(t2PpersistentFolder + "SomeReport");
        file.Create();
    }

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRfcTest, Plugin)
{
    ON_CALL(service, ConfigLine())
        .WillByDefault(
            ::testing::Return("{}"));
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }))
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(string(pcCallerID), _T("Telemetry"));
                EXPECT_EQ(string(pcParameterName), _T("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.FTUEReport.Enable"));
                EXPECT_EQ(string(pcParameterValue), _T("false"));
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);

                return WDMP_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
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

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setReportProfileStatus"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"wrongvalue\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"STARTED\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"STARTED\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setReportProfileStatus"), _T("{\"status\":\"COMPLETE\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("logApplicationEvent"), _T("{\"eventName\":\"NAME\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("logApplicationEvent"), _T("{\"eventValue\":\"VALUE\"}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("logApplicationEvent"), _T("{\"eventName\":\"NAME\", \"eventValue\":\"VALUE\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":true}"));

    plugin->Deinitialize(nullptr);
}
