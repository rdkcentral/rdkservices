#include <gtest/gtest.h>

#include "Telemetry.h"

#include "FactoriesImplementation.h"
#include "RfcApiMock.h"
#include "ServiceMock.h"
#include "TelemetryMock.h"
#include "RBusMock.h"
#include "IarmBusMock.h"

namespace {
const string profileFN = _T("/tmp/DefaultProfile.json");
const string t2PpersistentFolder = _T("/tmp/.t2reportprofiles/");
const uint8_t profileContent[] = "{\"profile\":\"default\"}";
}

using namespace WPEFramework;

using ::testing::NiceMock;

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
    NiceMock<ServiceMock> service;
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    Core::ProxyType<Plugin::Telemetry> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    IARM_EventHandler_t powerEventHandler;
    string response;

    TelemetryTest()
        : T2Test()
        , plugin(Core::ProxyType<Plugin::Telemetry>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        Core::Directory(t2PpersistentFolder.c_str()).Destroy(true);

        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        EXPECT_TRUE(handler != nullptr);
                        powerEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

    }
    virtual ~TelemetryTest() override
    {
        IarmBus::getInstance().impl = nullptr;
    }
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

    ServiceMock service;
    Core::JSONRPC::Message message;
    FactoriesImplementation factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    RBusApiImplMock rBusApiImplMock;

    TelemetryRBusTest()
        : TelemetryTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);

        RBusApi::getInstance().impl = &rBusApiImplMock;
    }
    virtual ~TelemetryRBusTest() override
    {
        RBusApi::getInstance().impl = nullptr;

        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
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

TEST_F(TelemetryRBusTest, uploadLogsRbusOpenFailure)
{
    EXPECT_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](rbusHandle_t* handle, char const* componentName) {
                EXPECT_TRUE(nullptr != handle);
                EXPECT_EQ(string(componentName), _T("TelemetryThunderPlugin"));
                return RBUS_ERROR_BUS_ERROR;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_OPENING_FAILED, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsRbusMethodFailure)
{
    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                return RBUS_ERROR_BUS_ERROR;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_RPC_CALL_FAILED, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsCallbackFailed)
{
    Core::Event onReportUpload(false, true);

    struct _rbusObject rbObject;

    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_BUS_ERROR, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_EQ(text, string(_T(
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Telemetry.onReportUpload\",\"params\":{\"telemetryUploadStatus\":\"UPLOAD_FAILURE\"}}"
                )));

                onReportUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    handler.Subscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, onReportUpload.Lock());

    handler.Unsubscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsGetValueFailure)
{
    Core::Event onReportUpload(false, true);

    struct _rbusObject rbObject;

    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusObject_GetValue(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusObject_t object, char const* name) {
                EXPECT_EQ(object, &rbObject);
                EXPECT_EQ(string(name), _T("UPLOAD_STATUS"));
                return nullptr;
            }));

    ON_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_EQ(text, string(_T(
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Telemetry.onReportUpload\",\"params\":{\"telemetryUploadStatus\":\"UPLOAD_FAILURE\"}}"
                )));

                onReportUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    handler.Subscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, onReportUpload.Lock());

    handler.Unsubscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsFailure)
{
    Core::Event onReportUpload(false, true);

    struct _rbusObject rbObject;
    struct _rbusValue rbValue;

    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusValue_GetString(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusValue_t value, int* len) {
                EXPECT_EQ(value, &rbValue);
                return "FAILURE";
            }));

    ON_CALL(rBusApiImplMock, rbusObject_GetValue(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusObject_t object, char const* name) {
                EXPECT_EQ(object, &rbObject);
                EXPECT_EQ(string(name), _T("UPLOAD_STATUS"));
                return &rbValue;
            }));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_EQ(text, string(_T(
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Telemetry.onReportUpload\",\"params\":{\"telemetryUploadStatus\":\"UPLOAD_FAILURE\"}}"
                )));

                onReportUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    handler.Subscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, onReportUpload.Lock());

    handler.Unsubscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogs)
{
    Core::Event onReportUpload(false, true);

    struct _rbusObject rbObject;
    struct _rbusValue rbValue;

    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    ON_CALL(rBusApiImplMock, rbusValue_GetString(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return( "SUCCESS"));

    ON_CALL(rBusApiImplMock, rbusObject_GetValue(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](rbusObject_t object, char const* name) {
                EXPECT_EQ(object, &rbObject);
                EXPECT_EQ(string(name), _T("UPLOAD_STATUS"));
                return &rbValue;
            }));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                callback(handle, methodName, RBUS_ERROR_SUCCESS, &rbObject);

                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));

                EXPECT_EQ(text, string(_T(
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.Telemetry.onReportUpload\",\"params\":{\"telemetryUploadStatus\":\"UPLOAD_SUCCESS\"}}"
                )));

                onReportUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    handler.Subscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadReport"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, onReportUpload.Lock());

    handler.Unsubscribe(0, _T("onReportUpload"), _T("org.rdk.Telemetry"), message);

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsOnStandby)
{
    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                EXPECT_EQ(string(methodName), _T("Device.X_RDKCENTRAL-COM_T2.UploadDCMReport"));
                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    ASSERT_TRUE(powerEventHandler != nullptr);

    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);

    plugin->Deinitialize(nullptr);
}

TEST_F(TelemetryRBusTest, uploadLogsOnDeepSleep)
{
    ON_CALL(rBusApiImplMock, rbus_open(::testing::_, ::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_CALL(rBusApiImplMock, rbusMethod_InvokeAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback,  int timeout) {
                EXPECT_EQ(string(methodName), _T("Device.X_RDKCENTRAL-COM_T2.AbortDCMReport"));
                return RBUS_ERROR_SUCCESS;
            }));

    ON_CALL(rBusApiImplMock, rbus_close(::testing::_))
        .WillByDefault(
            ::testing::Return(RBUS_ERROR_SUCCESS));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    ASSERT_TRUE(powerEventHandler != nullptr);

    IARM_Bus_PWRMgr_EventData_t eventData;
    eventData.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
    eventData.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &eventData , 0);

    plugin->Deinitialize(nullptr);
}
