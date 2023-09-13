/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

#include <gtest/gtest.h>

#include "SystemServices.h"

#include "FactoriesImplementation.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "RfcApiMock.h"
#include "ServiceMock.h"
#include "DispatcherMock.h"
#include "SleepModeMock.h"
#include "WrapsMock.h"

#include "deepSleepMgr.h"
#include "exception.hpp"

#include <fstream>

using namespace WPEFramework;

using ::testing::NiceMock;

class SystemServicesTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::SystemServices> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;
    NiceMock<RfcApiImplMock> rfcApiImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
    NiceMock<IarmBusImplMock> iarmBusImplMock;
    NiceMock<HostImplMock> hostImplMock;

    SystemServicesTest()
        : plugin(Core::ProxyType<Plugin::SystemServices>::Create())
        , handler(*plugin)
        , connection(1, 0)
    {
        RfcApi::getInstance().impl = &rfcApiImplMock;
        Wraps::getInstance().impl = &wrapsImplMock;
        IarmBus::getInstance().impl = &iarmBusImplMock;
        device::Host::getInstance().impl = &hostImplMock;
    }

    virtual ~SystemServicesTest() override
    {
        RfcApi::getInstance().impl = nullptr;
        Wraps::getInstance().impl = nullptr;
        IarmBus::getInstance().impl = nullptr;
        device::Host::getInstance().impl = nullptr;
    }
};

class SystemServicesEventTest : public SystemServicesTest {
protected:
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IDispatcher* dispatcher;

    SystemServicesEventTest()
        : SystemServicesTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        dispatcher = static_cast<PluginHost::IDispatcher*>(
            plugin->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher->Activate(&service);
    }

    virtual ~SystemServicesEventTest() override
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        PluginHost::IFactories::Assign(nullptr);
    }
};

class SystemServicesEventIarmTest : public SystemServicesEventTest {
protected:
    IARM_BusCall_t SysModeChange;
    IARM_EventHandler_t systemStateChanged;
    IARM_EventHandler_t thermMgrEventsHandler;
    IARM_EventHandler_t powerEventHandler;

    SystemServicesEventIarmTest()
        : SystemServicesEventTest()
    {
        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    if ((string(IARM_BUS_SYSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE)) {
                        systemStateChanged = handler;
                    }
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED)) {
                        thermMgrEventsHandler = handler;
                    }
                    if ((string(IARM_BUS_PWRMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_PWRMGR_EVENT_MODECHANGED)) {
                        powerEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));
        ON_CALL(iarmBusImplMock, IARM_Bus_RegisterCall(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* methodName, IARM_BusCall_t handler) {
                    if (string(IARM_BUS_COMMON_API_SysModeChange) == string(methodName)) {
                        SysModeChange = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~SystemServicesEventIarmTest() override
    {
        plugin->Deinitialize(&service);
    }

    virtual void SetUp()
    {
        ASSERT_TRUE(SysModeChange != nullptr);
        ASSERT_TRUE(systemStateChanged != nullptr);
        ASSERT_TRUE(thermMgrEventsHandler != nullptr);
        ASSERT_TRUE(powerEventHandler != nullptr);
    }
};

TEST_F(SystemServicesTest, TestedAPIsShouldExist)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestSystemUptime")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("fireFirmwarePendingReboot")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFirmwareAutoReboot")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFirmwareRebootDelay")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastFirmwareFailureReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDownloadedFirmwareInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareDownloadPercent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTimeZoneDST")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimeZoneDST")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo2")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isOptOutTelemetry")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOptOutTelemetry")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemVersions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("queryMocaStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableMoca")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFirmware")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDeepSleepTimer")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setNetworkStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getNetworkStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFriendlyName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setFriendlyName")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferredStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferredStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAvailableStandbyModes")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getWakeupReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastWakeupKeyCode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimeZones")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastDeepSleepReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCoreTemperature")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTemperatureThresholds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTemperatureThresholds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getOvertempGraceInterval")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOvertempGraceInterval")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getRFCConfig")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableXREConnectionRetention")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPowerState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPowerState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPowerStateIsManagedByDevice")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPowerStateBeforeReboot")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setWakeupSrcConfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("reboot")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getStateInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBootLoaderPattern")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMacAddresses")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearLastDeepSleepReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMfgSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getXconfParams")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPlatformConfiguration")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("uploadLogs")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("uploadLogsAsync")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("abortLogUpload")));
}

TEST_F(SystemServicesTest, SystemUptime)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestSystemUptime"), _T("{}"), response));

    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
                                                     "\"systemUptime\":\"[0-9]+.[0-9]+\","
                                                     "\"success\":true"
                                                     "\\}")));
}

TEST_F(SystemServicesEventTest, PendingReboot)
{
    Core::Event onFirmwarePendingReboot(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.System.onFirmwarePendingReboot\","
                                          "\"params\":"
                                          "{"
                                          "\"fireFirmwarePendingReboot\":600"
                                          "}"
                                          "}")));

                onFirmwarePendingReboot.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onFirmwarePendingReboot"), _T("org.rdk.System"), message);

    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("fireFirmwarePendingReboot"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, onFirmwarePendingReboot.Lock());

    handler.Unsubscribe(0, _T("onFirmwarePendingReboot"), _T("org.rdk.System"), message);
}

TEST_F(SystemServicesTest, AutoReboot)
{
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"), 0);
                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{\"enable\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, RebootDelay)
{
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":86401}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":10}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, Firmware)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\nVERSION=000.36.0.0\nBUILD_TIME=\"2022-08-05 16:14:54\"\n";
    file.close();

    file.open("/opt/fwdnldstatus.txt");
    file << "Method|xconf \nProto|\nStatus|Failure\nReboot|\nFailureReason|Invalid Request\nDnldVersn|AX013AN_5.6p4s1_VBN_sey\nDnldFile|\nDnlURL|https://dac15cdlserver.ae.ccp.xcal.tv:443/Images/AX013AN_5.6p4s1_VBN_sey-signed.bin\n FwupdateState|Failed\n";

    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLastFirmwareFailureReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"failReason\":\"None\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDownloadedFirmwareInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"currentFWVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"downloadedFWVersion\":\"\",\"downloadedFWLocation\":\"\",\"isRebootDeferred\":false,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareDownloadPercent"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"downloadPercent\":-1,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateState"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"firmwareUpdateState\":0,\"success\":true}"));
}

TEST_F(SystemServicesEventTest, Timezone)
{
    Core::Event changed1(false, true);
    Core::Event changed2(false, true);
    Core::Event changed3(false, true);
    Core::Event changed4(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(4)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTimeZoneDSTChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"oldTimeZone\":\".*\","
                                                             "\"newTimeZone\":\"America\\\\/New_York\","
                                                             "\"oldAccuracy\":\".*\","
                                                             "\"newAccuracy\":\".*\""
                                                             "\\}"
                                                             "\\}")));

                changed1.SetEvent();

                return Core::ERROR_NONE;
            }))

        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.System.onTimeZoneDSTChanged\","
                                          "\"params\":"
                                          "{"
                                          "\"oldTimeZone\":\"America\\/New_York\","
                                          "\"newTimeZone\":\"America\\/New_York\","
                                          "\"oldAccuracy\":\"INITIAL\","
                                          "\"newAccuracy\":\"INTERIM\""
                                          "}"
                                          "}")));

                changed2.SetEvent();

                return Core::ERROR_NONE;
            }))

        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.System.onTimeZoneDSTChanged\","
                                          "\"params\":"
                                          "{"
                                          "\"oldTimeZone\":\"America\\/New_York\","
                                          "\"newTimeZone\":\"America\\/Costa_Rica\","
                                          "\"oldAccuracy\":\"INTERIM\","
                                          "\"newAccuracy\":\"FINAL\""
                                          "}"
                                          "}")));

                changed3.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.System.onTimeZoneDSTChanged\","
                                          "\"params\":"
                                          "{"
                                          "\"oldTimeZone\":\"America\\/Costa_Rica\","
                                          "\"newTimeZone\":\"America\\/New_York\","
                                          "\"oldAccuracy\":\"FINAL\","
                                          "\"newAccuracy\":\"FINAL\""
                                          "}"
                                          "}")));

                changed4.SetEvent();

                return Core::ERROR_NONE;
            })) ;

    handler.Subscribe(0, _T("onTimeZoneDSTChanged"), _T("org.rdk.System"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/New_York\",\"accuracy\":\"INITIAL\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed1.Lock());


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"timeZone\":\"America\\/New_York\",\"accuracy\":\"INITIAL\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/New_York\",\"accuracy\":\"INTERIM\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed2.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"timeZone\":\"America\\/New_York\",\"accuracy\":\"INTERIM\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/Costa_Rica\",\"accuracy\":\"FINAL\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed3.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"timeZone\":\"America\\/Costa_Rica\",\"accuracy\":\"FINAL\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/New_York\",\"accuracy\":\"<wrong accuracy>\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed3.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"timeZone\":\"America\\/New_York\",\"accuracy\":\"FINAL\",\"success\":true}"));

    handler.Unsubscribe(0, _T("onTimeZoneDSTChanged"), _T("org.rdk.System"), message);
}

TEST_F(SystemServicesTest, InvalidTerritory)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"U-NYC\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US@\",\"region\":\"US-NYC\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-N$C\"}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US12\",\"region\":\"US-NYC\"}"), response));
}

TEST_F(SystemServicesEventTest, ValidTerritory)
{
    Core::Event changed1(false, true);
    Core::Event changed2(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTerritoryChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"oldTerritory\":\".*\","
                                                             "\"newTerritory\":\"USA\","
                                                             "\"oldRegion\":\".*\","
                                                             "\"newRegion\":\"US-NYC\""
                                                             "\\}"
                                                             "\\}")));

                changed1.SetEvent();

                return Core::ERROR_NONE;
            }))
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, string(_T("{"
                                          "\"jsonrpc\":\"2.0\","
                                          "\"method\":\"org.rdk.System.onTerritoryChanged\","
                                          "\"params\":"
                                          "{"
                                          "\"oldTerritory\":\"USA\","
                                          "\"newTerritory\":\"GBR\","
                                          "\"oldRegion\":\"US-NYC\","
                                          "\"newRegion\":\"GB-EGL\""
                                          "}"
                                          "}")));

                changed2.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onTerritoryChanged"), _T("org.rdk.System"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-NYC\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed1.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"territory\":\"USA\",\"region\":\"US-NYC\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"GBR\",\"region\":\"GB-EGL\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, changed2.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"territory\":\"GBR\",\"region\":\"GB-EGL\",\"success\":true}"));

    handler.Unsubscribe(0, _T("onTerritoryChanged"), _T("org.rdk.System"), message);
}

TEST_F(SystemServicesTest, rebootReason)
{
    ofstream file("/opt/logs/rebootInfo.log");
    file << "PreviousRebootReason: RebootReason: Triggered from SystemServices! MAINTENANCE_REBOOT \n PreviousRebootTime: 18.08.2022_09:51.38\nPreviousRebootInitiatedBy: HAL_CDL_notify_mgr_event\nPreviousCustomReason: MAINTENANCE_REBOOT \n PreviousOtherReason: MAINTENANCE_REBOOT \n";
    file.close();

    file.open("/opt/persistent/previousreboot.info");
    file << "{\n\"timestamp\":\"Thu Aug 18 13:51:39 UTC 2022\",\n \"source\":\"HAL_CDL_notify_mgr_event\",\n \"reason\":\"OPS_TRIGGERED\",\n \"customReason\":\"Unknown\",\n \"otherReason\":\"Rebooting the box due to VL_CDL_MANAGER_EVENT_REBOOT...!\"\n}\n";
    file.close();

    file.open("/opt/persistent/hardpower.info");
    file << "{\n\"lastHardPowerReset\":\"Thu Aug 18 13:51:39 UTC 2022\"\n}\n";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"timeStamp\":\"18.08.2022_09:51.38\",\"reason\":\"Triggered from SystemServices! MAINTENANCE_REBOOT\",\"source\":\"HAL_CDL_notify_mgr_event\",\"customReason\":\"MAINTENANCE_REBOOT\",\"otherReason\":\"MAINTENANCE_REBOOT\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo2"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"rebootInfo\":{\"timestamp\":\"Thu Aug 18 13:51:39 UTC 2022\",\"source\":\"HAL_CDL_notify_mgr_event\",\"reason\":\"OPS_TRIGGERED\",\"customReason\":\"Unknown\",\"lastHardPowerReset\":\"Thu Aug 18 13:51:39 UTC 2022\"},\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"reason\":\"OPS_TRIGGERED\",\"success\":true}"));
}

TEST_F(SystemServicesTest, Telemetry)
{
    ofstream file("/opt/tmtryoptout");
    file << "false";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"Opt-Out\":false,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOptOutTelemetry"), _T("{\"Opt-Out\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"Opt-Out\":true,\"success\":true}"));
}

TEST_F(SystemServicesTest, SystemVersions)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\nVERSION=000.36.0.0\nBUILD_TIME=\"2022-08-05 16:14:54\"\n";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSystemVersions"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"stbVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"receiverVersion\":\"000.36.0.0\",\"stbTimestamp\":\"Fri 05 Aug 2022 16:14:54 AP UTC\",\"success\":true}"));
}

TEST_F(SystemServicesTest, MocaStatus)
{
    ON_CALL(wrapsImplMock, system(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/etc/init.d/moca_init start")));
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("enableMoca"), _T("{}"), response));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableMoca"), _T("{\"value\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_TRUE(Core::File(string(_T("/opt/enablemoca"))).Exists());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("queryMocaStatus"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mocaEnabled\":true,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableMoca"), _T("{\"value\":false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_FALSE(Core::File(string(_T("/opt/enablemoca"))).Exists());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("queryMocaStatus"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mocaEnabled\":false,\"success\":true}"));
}

TEST_F(SystemServicesTest, updateFirmware)
{
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/swupdate_utility.sh 0 4 >> /opt/logs/swupdate.log &")));
                return nullptr;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("updateFirmware"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, Mode)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"modeInfo\":{\"mode\":\"\",\"duration\":0},\"success\":true}"));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setMode"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":{}}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":{\"mode\":\"unknown\",\"duration\":0}}"), response));

    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DAEMON_NAME)));
                EXPECT_EQ(string(methodName), string(_T("DaemonSysModeChange")));
                return IARM_RESULT_SUCCESS;
            });

    ON_CALL(wrapsImplMock, system(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("rm -f /opt/warehouse_mode_active")));
                return 0;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":{\"mode\":\"NORMAL\",\"duration\":-1}}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"modeInfo\":{\"mode\":\"NORMAL\",\"duration\":0},\"success\":true}"));
}

TEST_F(SystemServicesTest, setDeepSleepTimer)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setDeepSleepTimer"), _T("{}"), response));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetDeepSleepTimeOut)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDeepSleepTimer"), _T("{\"seconds\":5}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, setNetworkStandbyMode)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setNetworkStandbyMode"), _T("{}"), response));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetNetworkStandbyMode)));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setNetworkStandbyMode"), _T("{\"nwStandby\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, getsetFriendlyName)
{       
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFriendlyName"), _T("{\"friendlyName\": \"friendlyTest\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
                
                
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFriendlyName"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"friendlyName\":\"friendlyTest\",\"success\":true}"));
}  


TEST_F(SystemServicesTest, getNetworkStandbyMode)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetNetworkStandbyMode)));
                auto param = static_cast<IARM_Bus_PWRMgr_NetworkStandbyMode_Param_t*>(arg);
                param->bStandbyMode = true;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getNetworkStandbyMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"nwStandby\":true,\"success\":true}"));
}

TEST_F(SystemServicesTest, setPreferredStandbyMode)
{
    device::SleepMode mode;
    NiceMock<SleepModeMock> sleepModeMock;
    device::SleepMode::getInstance().impl = &sleepModeMock;

    ON_CALL(sleepModeMock, getInstanceByName)
        .WillByDefault(::testing::Invoke(
            [&](const std::string& name) -> device::SleepMode& {
                EXPECT_EQ(name, "LIGHT_SLEEP");
                return mode;
            }));
    EXPECT_CALL(hostImplMock, setPreferredSleepMode)
        .Times(2)
        .WillOnce(::testing::Return(0))
        .WillOnce(::testing::Invoke(
            [](const device::SleepMode) -> int {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPreferredStandbyMode"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPreferredStandbyMode"), _T("{\"standbyMode\":\"LIGHT_SLEEP\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPreferredStandbyMode"), _T("{\"standbyMode\":\"LIGHT_SLEEP\"}"), response));
}

TEST_F(SystemServicesTest, getPreferredStandbyMode)
{
    device::SleepMode mode;
    NiceMock<SleepModeMock> sleepModeMock;
    mode.impl = &sleepModeMock;
    string sleepModeString(_T("DEEP_SLEEP"));

    ON_CALL(hostImplMock, getPreferredSleepMode)
        .WillByDefault(::testing::Return(mode));
    ON_CALL(sleepModeMock, toString)
        .WillByDefault(::testing::ReturnRef(sleepModeString));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreferredStandbyMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"preferredStandbyMode\":\"DEEP_SLEEP\",\"success\":true}"));

    ON_CALL(hostImplMock, getPreferredSleepMode)
        .WillByDefault(::testing::Invoke(
            []() -> device::SleepMode {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPreferredStandbyMode"), _T("{}"), response));
}

TEST_F(SystemServicesTest, getAvailableStandbyModes)
{
    device::SleepMode mode;
    NiceMock<SleepModeMock> sleepModeMock;
    mode.impl = &sleepModeMock;
    string sleepModeString(_T("DEEP_SLEEP"));

    ON_CALL(hostImplMock, getAvailableSleepModes)
        .WillByDefault(::testing::Return(std::vector<device::SleepMode>({ mode })));
    ON_CALL(sleepModeMock, toString)
        .WillByDefault(::testing::ReturnRef(sleepModeString));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getAvailableStandbyModes"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"supportedStandbyModes\":[\"DEEP_SLEEP\"],\"success\":true}"));

    ON_CALL(hostImplMock, getAvailableSleepModes)
        .WillByDefault(::testing::Invoke(
            []() -> device::List<device::SleepMode> {
                throw device::Exception("test");
            }));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getAvailableStandbyModes"), _T("{}"), response));
}

/*******************************************************************************************************************
 * Test function for :getWakeupReason
 * getWakeupReason :
 *                The API which Collects reason for the device coming out of deep sleep.
 *
 *                @return Returns the reason for the device coming out of deep sleep.
 * Use case coverage:
 *                @Success : 17
 *                @Failure : 1
 ********************************************************************************************************************/

/**
 * @brief : getWakeupReason without passing any reason.
 *        Check if BUS call to retrieve the reason for the device is failed,
 *        then getWakeupReason shall be failed.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_UNKNOWN","success":false}
 */
TEST_F(SystemServicesTest, getWakeupReasonFailure)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                return IARM_RESULT_INVALID_PARAM;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_IR.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_IR as a reason,
 *        then the response returned is "WAKEUP_REASON_IR" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_IR","success":true}
 */

TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_IR)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_IR;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_IR\",\"success\":true}"));
}
/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_RCU_BT.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_RCU_BT as a reason,
 *        then the response returned is "WAKEUP_REASON_RCU_BT" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_RCU_BT","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_RCU_BT)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_RCU_BT;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_RCU_BT\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_RCU_RF4CE.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_RCU_RF4CE as a reason,
 *        then the response returned is "WAKEUP_REASON_RCU_RF4CE" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_RCU_RF4CE","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_RCU_RF4CE)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_RCU_RF4CE;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_RCU_RF4CE\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_GPIO.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_GPIO as a reason,
 *        then the response returned is "WAKEUP_REASON_GPIO" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_GPIO","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_GPIO)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_GPIO;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_GPIO\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_LAN.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_LAN as a reason,
 *        then the response returned is "WAKEUP_REASON_LAN" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_LAN","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_LAN)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_LAN;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_LAN\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_WLAN.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_WLAN as a reason,
 *        then the response returned is "WAKEUP_REASON_WLAN" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_WLAN","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_WLAN)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_WLAN;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_WLAN\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_TIMER.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_TIMER as a reason,
 *        then the response returned is "WAKEUP_REASON_TIMER" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_TIMER","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_TIMER)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_TIMER;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_TIMER\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_FRONT_PANEL.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_FRONT_PANEL as a reason,
 *        then the response returned is "WAKEUP_REASON_FRONT_PANEL" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_FRONT_PANEL","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_FRONT_PANEL)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_FRONT_PANEL;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_FRONT_PANEL\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_WATCHDOG.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_WATCHDOG as a reason,
 *        then the response returned is "WAKEUP_REASON_WATCHDOG" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_WATCHDOG","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_WATCHDOG)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_WATCHDOG;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_WATCHDOG\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET as a reason,
 *        then the response returned is "WAKEUP_REASON_SOFTWARE_RESET" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_SOFTWARE_RESET","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_SOFTWARE_RESET)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_SOFTWARE_RESET;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_SOFTWARE_RESET\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_THERMAL_RESET.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_THERMAL_RESET as a reason,
 *        then the response returned is "WAKEUP_REASON_THERMAL_RESET" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_THERMAL_RESET","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_THERMAL_RESET)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_THERMAL_RESET;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_THERMAL_RESET\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_WARM_RESET.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_WARM_RESET as a reason,
 *        then the response returned is "WAKEUP_REASON_WARM_RESET" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_WARM_RESET","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_WARM_RESET)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_WARM_RESET;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_WARM_RESET\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_COLDBOOT.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_COLDBOOT as a reason,
 *        then the response returned is "WAKEUP_REASON_COLDBOOT" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_COLDBOOT","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_COLDBOOT)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_COLDBOOT;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_COLDBOOT\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE as a reason,
 *        then the response returned is "WAKEUP_REASON_STR_AUTH_FAILURE" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_STR_AUTH_FAILURE","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_STR_AUTH_FAILURE)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_STR_AUTH_FAILURE;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_STR_AUTH_FAILURE\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_CEC.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_CEC as a reason,
 *        then the response returned is "WAKEUP_REASON_CEC" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_CEC","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_CEC)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_CEC;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_CEC\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_PRESENCE.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_PRESENCE as a reason,
 *        then the response returned is "WAKEUP_REASON_PRESENCE" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_PRESENCE","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_PRESENCE)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_PRESENCE;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_PRESENCE\",\"success\":true}"));
}

/**
 * @brief : getWakeupReason when the reason is DEEPSLEEP_WAKEUPREASON_VOICE.
 *        When BUS call retrieves DEEPSLEEP_WAKEUPREASON_VOICE as a reason,
 *        then the response returned is "WAKEUP_REASON_VOICE" with Success status as true.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"wakeupReason":"WAKEUP_REASON_VOICE","success":true}
 */
TEST_F(SystemServicesTest, getWakeupReasonSuccess_When_WAKEUPREASON_VOICE)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupReason)));
                auto param = static_cast<DeepSleep_WakeupReason_t*>(arg);
                *param = DEEPSLEEP_WAKEUPREASON_VOICE;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupReason\":\"WAKEUP_REASON_VOICE\",\"success\":true}"));
}
/*Test cases for getWakeupReason ends here*/


TEST_F(SystemServicesTest, getLastWakeupKeyCode)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_DEEPSLEEPMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_DEEPSLEEPMGR_API_GetLastWakeupKeyCode)));
                auto param = static_cast<IARM_Bus_DeepSleepMgr_WakeupKeyCode_Param_t*>(arg);
                param->keyCode = 5;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLastWakeupKeyCode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"wakeupKeyCode\":5,\"success\":true}"));
}

TEST_F(SystemServicesEventIarmTest, onFirmwareUpdateStateChange)
{
    Core::Event onFirmwareUpdateStateChange(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                // TODO: BUG. enum should be used
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onFirmwareUpdateStateChange\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"firmwareUpdateStateChange\":4"
                                                             "\\}"
                                                             "\\}")));

                onFirmwareUpdateStateChange.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onFirmwareUpdateStateChange"), _T("org.rdk.System"), message);

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_UPDATE_STATE;
    sysEventData.data.systemStates.state = IARM_BUS_SYSMGR_FIRMWARE_UPDATE_STATE_DOWNLOAD_COMPLETE;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateStateChange.Lock());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateState"), _T("{}"), response));
    // TODO: BUG. enum should be used
    EXPECT_EQ(response, string("{\"firmwareUpdateState\":4,\"success\":true}"));

    handler.Unsubscribe(0, _T("onFirmwareUpdateStateChange"), _T("org.rdk.System"), message);
}

TEST_F(SystemServicesEventIarmTest, onSystemClockSet)
{
    Core::Event onSystemClockSet(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemClockSet\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\\}"
                                                             "\\}")));

                onSystemClockSet.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemClockSet"), _T("org.rdk.System"), message);

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE;
    sysEventData.data.systemStates.state = 666; // I have no idea what it should be but SystemServices needs non-zero
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onSystemClockSet.Lock());

    handler.Unsubscribe(0, _T("onSystemClockSet"), _T("org.rdk.System"), message);
}

/*************************************************************************************************************
 * Test function for :onTemperatureThresholdChanged
 * onTemperatureThresholdChanged :
 *                Triggered when the device temperature changes beyond the WARN or MAX limits.
 *
 *                @return Whether the mode change is succeeded.
 * Use case coverage:
 *                @Success :4
 *                @Failure :5
 ************************************************************************************************************/

/**
 * @brief : onTemperatureThresholdChanged when instance is invalid.
 *        Check if SystemServices instance is null,
 *        then onTemperatureThresholdChanged shall be failed and mode change will not be successful.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedFailed_withoutValidInstance)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    WPEFramework::Plugin::SystemServices::_instance = nullptr;

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onTemperatureThresholdChanged.Lock(100));

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when newLevel is empty.
 *        Check if newLevel is empty,
 *        then onTemperatureThresholdChanged shall be failed and mode change will not be successful.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedFailed_whenNewLevelEmpty)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onTemperatureThresholdChanged.Lock(100));

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is NORMAL and current level is empty.
 *        Check if new level is NORMAL and current level is empty,
 *        then onTemperatureThresholdChanged shall be failed and mode change will not be successful.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedFailed_currLevelEmpty_newLevelNORMAL)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_NORMAL;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onTemperatureThresholdChanged.Lock(100));

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is HIGH and current level is empty.
 *        Check if new level is HIGH and current level is empty,
 *        then onTemperatureThresholdChanged shall be failed and mode change will not be successful.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedFailed_currLevelEmpty_newLevelHIGH)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onTemperatureThresholdChanged.Lock(100));

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is CRITICAL and current level is empty.
 *        Check if new level is CRITICAL and current level is empty,
 *        then onTemperatureThresholdChanged shall be failed and mode change will not be successful.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedFailed_currLevelEmpty_newLevelCRITICAL)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onTemperatureThresholdChanged.Lock(100));

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is HIGH and current level is NORMAL.
 *        Check if new level is HIGH and current level is NORMAL,
 *        then onTemperatureThresholdChanged shall be succeeded,
 *        And set the value :thresholdType = "WARN" , exceeded ="true" and temperature= curTemperature obtained from IarmEvent.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedSuccess_whenNormalToHigh)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                // TODO: BUG. number should not be string
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTemperatureThresholdChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"thresholdType\":\"WARN\","
                                                             "\"exceeded\":true,"
                                                             "\"temperature\":\"100.000000\""
                                                             "\\}"
                                                             "\\}")));

                onTemperatureThresholdChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_NORMAL;
    param.data.therm.curTemperature = 100;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onTemperatureThresholdChanged.Lock());

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is HIGH and current level is CRITICAL.
 *        Check if new level is HIGH and current level is CRITICAL,
 *        then onTemperatureThresholdChanged shall be succeeded;
 *        And set the value :thresholdType = "MAX" , exceeded ="false" and temperature= curTemperature obtained from IarmEvent.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedSuccess_whenCriticalToHigh)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTemperatureThresholdChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"thresholdType\":\"MAX\","
                                                             "\"exceeded\":false,"
                                                             "\"temperature\":\"48.000000\""
                                                             "\\}"
                                                             "\\}")));

                onTemperatureThresholdChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL;
    param.data.therm.curTemperature = 48;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onTemperatureThresholdChanged.Lock());

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is CRITICAL and current level is HIGH.
 *        Check if new level is CRITICAL and current level is HIGH,
 *        then onTemperatureThresholdChanged shall be succeeded;
 *        And set the value :thresholdType = "MAX" , exceeded ="true" and temperature= curTemperature obtained from IarmEvent.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChangedSuccess_whenHighToCritical)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTemperatureThresholdChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"thresholdType\":\"MAX\","
                                                             "\"exceeded\":true,"
                                                             "\"temperature\":\"100.000000\""
                                                             "\\}"
                                                             "\\}")));

                onTemperatureThresholdChanged.SetEvent();

                return Core::ERROR_NONE;
            }));
    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_CRITICAL;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curTemperature = 100;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onTemperatureThresholdChanged.Lock());

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief : onTemperatureThresholdChanged when new level is NORMAL and current level is HIGH.
 *        Check if new level is NORMAL and current level is HIGH,
 *        then onTemperatureThresholdChanged shall be succeeded;
 *        And set the value :thresholdType = "WARN" , exceeded ="false" and temperature= curTemperature obtained from IarmEvent.
 *
 * @param[in]   :  None.
 * @return      :  None.
 */
TEST_F(SystemServicesEventIarmTest,  onTemperatureThresholdChangedSuccess_whenHighToNormal)
{
    Core::Event onTemperatureThresholdChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onTemperatureThresholdChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"thresholdType\":\"WARN\","
                                                             "\"exceeded\":false,"
                                                             "\"temperature\":\"100.000000\""
                                                             "\\}"
                                                             "\\}")));

                onTemperatureThresholdChanged.SetEvent();

                return Core::ERROR_NONE;
            }));
    handler.Subscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.therm.newLevel = IARM_BUS_PWRMGR_TEMPERATURE_NORMAL;
    param.data.therm.curLevel = IARM_BUS_PWRMGR_TEMPERATURE_HIGH;
    param.data.therm.curTemperature = 100;
    thermMgrEventsHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_THERMAL_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onTemperatureThresholdChanged.Lock());

    handler.Unsubscribe(0, _T("onTemperatureThresholdChanged"), _T("org.rdk.System"), message);
}
/*Test cases for onTemperatureThresholdChanged ends here*/

extern "C" FILE* __real_popen(const char* command, const char* type);

TEST_F(SystemServicesTest, getTimeZones)
{
    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command, const char* type) -> FILE* {
                EXPECT_THAT(string(command), ::testing::MatchesRegex("zdump \\/usr\\/share\\/zoneinfo/.+"));
                return __real_popen(command, type);
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZones"), _T("{}"), response));
    EXPECT_THAT(response, ::testing::MatchesRegex("\\{\"zoneinfo\":\\{.*\"GMT\":\".+ GMT\".*\\},\"success\":true\\}"));
}

TEST_F(SystemServicesTest, getLastDeepSleepReason)
{
    ofstream file("/opt/standbyReason.txt");
    file << "thermal_deepsleep_critical_threshold";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getLastDeepSleepReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"reason\":\"thermal_deepsleep_critical_threshold\",\"success\":true}"));
}

TEST_F(SystemServicesTest, getCoreTemperature)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetThermalState)));
                auto param = static_cast<IARM_Bus_PWRMgr_GetThermalState_Param_t*>(arg);
                param->curTemperature = 100;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCoreTemperature"), _T("{}"), response));
    // TODO: BUG. number should not be string
    EXPECT_EQ(response, string("{\"temperature\":\"100.000000\",\"success\":true}"));
}

TEST_F(SystemServicesTest, getTemperatureThresholds)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                if (string(methodName) == string(_T(IARM_BUS_PWRMGR_API_GetTemperatureThresholds))) {
                    auto param = static_cast<IARM_Bus_PWRMgr_GetTempThresholds_Param_t*>(arg);
                    param->tempHigh = 100;
                    param->tempCritical = 200;
                } else if (string(methodName) == string(_T(IARM_BUS_PWRMGR_API_GetThermalState))) {
                    auto param = static_cast<IARM_Bus_PWRMgr_GetThermalState_Param_t*>(arg);
                    param->curTemperature = 62;
                }
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTemperatureThresholds"), _T("{}"), response));
    // TODO: BUG. number should not be string
    EXPECT_EQ(response, string("{\"temperatureThresholds\":{\"WARN\":\"100.000000\",\"MAX\":\"200.000000\",\"temperature\":\"62.000000\"},\"success\":true}"));
}

TEST_F(SystemServicesTest, setTemperatureThresholds)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetTemperatureThresholds)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetTempThresholds_Param_t*>(arg);
                EXPECT_EQ(param->tempHigh, 99);
                EXPECT_EQ(param->tempCritical, 199);
                return IARM_RESULT_SUCCESS;
            });

    // TODO: BUG. number should not be string
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTemperatureThresholds"), _T("{\"thresholds\":{\"WARN\":\"99.000000\",\"MAX\":\"199.000000\"}}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, getOvertempGraceInterval)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetOvertempGraceInterval)));
                auto param = static_cast<IARM_Bus_PWRMgr_GetOvertempGraceInterval_Param_t*>(arg);
                param->graceInterval = 600;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getOvertempGraceInterval"), _T("{}"), response));
    // TODO: BUG. number should not be string
    EXPECT_EQ(response, string("{\"graceInterval\":\"600\",\"success\":true}"));
}

TEST_F(SystemServicesTest, setOvertempGraceInterval)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetOvertempGraceInterval)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetOvertempGraceInterval_Param_t*>(arg);
                EXPECT_EQ(param->graceInterval, 600);
                return IARM_RESULT_SUCCESS;
            });

    // TODO: BUG. number should not be string
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOvertempGraceInterval"), _T("{\"graceInterval\":\"600\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, getRFCConfig)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getRFCConfig"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getRFCConfig"), _T("{\"rfclist\":[]}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getRFCConfig"), _T("{\"rfclist\":[\"#@!\"]}"), response));

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("SystemServices"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.SerialNumber"));
                strncpy(pstParamData->value, "test", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getRFCConfig"), _T("{\"rfcList\":[\"Device.DeviceInfo.SerialNumber\"]}"), response));
    EXPECT_EQ(response, string("{\"RFCConfig\":{\"Device.DeviceInfo.SerialNumber\":\"test\"},\"success\":true}"));
}

TEST_F(SystemServicesTest, enableXREConnectionRetention)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableXREConnectionRetention"), _T("{\"enable\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_TRUE(Core::File(string(_T("/tmp/retainConnection"))).Exists());
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("enableXREConnectionRetention"), _T("{\"enable\":false}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    EXPECT_FALSE(Core::File(string(_T("/tmp/retainConnection"))).Exists());
}

TEST_F(SystemServicesTest, getPowerState)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_GetPowerState_Param_t*>(arg);
                param->curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPowerState"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"powerState\":\"STANDBY\",\"success\":true}"));
}

/*******************************************************************************************************************
 * Test function for :setPowerState
 * setPowerState :
 *                Sets the power state of the device.
 *                valid powerState: {"STANDBY", "DEEP_SLEEP", "LIGHT_SLEEP", "ON"}
 *
 *                @return Whether the request succeeded.
 * Event : onSystemPowerStateChanged
 *                Triggers when the system power state changes.
 * Use case coverage:
 *                @Success :5
 *                @Failure :3
 ********************************************************************************************************************/

/**
 * @brief : setPowerState when param is empty.
 *        If param is empty,
 *        then setPowerState shall be failed.
 *
 * @param[in]   :  "params": {}
 * @return      :  {"success":false}
 */
TEST_F(SystemServicesTest, setPowerStateFailed_without_powerstate)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPowerState"), _T(""), response));
}

/**
 * @brief : setPowerState when param contains invalid powerstate.
 *        Check if input param contains invalid powerstate,
 *        then setPowerState shall be failed.
 *
 * @param[in]   :  "params": {"powerState": "abc"}
 * @return      :  {"success":false}
 */
TEST_F(SystemServicesTest, setPowerStateFailed_with_invalidpowerstate)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"abc\"}"), response));
}

/**
 * @brief : setPowerState when Bus call returns other than IARM_RESULT_SUCCESS,
 *        then setPowerState shall be failed.
 *
 * @return      :  {"success":false}
 */
 TEST_F(SystemServicesTest, setPowerStateFailed_when_Bus_call_fails)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                return IARM_RESULT_INVALID_PARAM;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"ON\"}"), response));
}

/**
 * * @brief : setPowerState when powerstate is ON.
 *        Check if  input param : powerstate is  ON,
 *        then setPowerState will sets with provided state and setPowerState shall be succeeded.
 *
 * @param[in]   :  "params": {"powerState": "ON"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setPowerStateSuccess_when_powerstate_ON)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_ON);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"ON\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * * @brief : setPowerState when powerstate is LIGHT_SLEEP.
 *        Check if  input param : powerstate is  LIGHTSLEEP,
 *        then setPowerState will sets with provided state and setPowerState shall be succeeded.
 *
 * @param[in]   :  "params": {"powerState": "LIGHT_SLEEP"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setPowerStateSuccess_when_powerstate_Light_sleep)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_STANDBY);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"LIGHT_SLEEP\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * * @brief : setPowerState when powerstate is STANDBY and has valid SystemServices instance
 *        Check if SystemServices instance is not null,and if sleepmode is not DEEP_SLEEP 
 *        then it calls setPowerState with state as the parameter and setPowerState shall be succeeded.
 *
 * @param[in]   :  "params": {"powerState": "STANDBY"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setPowerStateSuccess_when_powerstate_STANDBY_withValidInstance)
{
    device::SleepMode mode;
    NiceMock<SleepModeMock> sleepModeMock;
    mode.impl = &sleepModeMock;
    string sleepModeString(_T("STANDBY"));

    ON_CALL(hostImplMock, getPreferredSleepMode)
        .WillByDefault(::testing::Return(mode));
    ON_CALL(sleepModeMock, toString)
        .WillByDefault(::testing::ReturnRef(sleepModeString));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_STANDBY);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"STANDBY\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * * @brief : setPowerState when powerstate is STANDBY and  SystemServices instance is null
 *         Check if SystemServices instance is null,then it sets the PowerState with the current value of state and
 *         setPowerState shall be succeeded.
 *
 * @param[in]   :  "params": {"powerState": "STANDBY"}
 * @return      :  {"success":true}
 */

TEST_F(SystemServicesTest, setPowerStateSuccess_when_powerstate_STANDBY_withoutValidInstance)
{
    WPEFramework::Plugin::SystemServices::_instance = nullptr;
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_STANDBY);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"STANDBY\"}"), response));
}

/**
 * * @brief : setPowerState when valid preferredStandbyMode
 *          Check if getPreferredStandbyMode returns valid preferred standby mode,
 *          then it set the powerstate with prefered standbymode and setPowerState shall be succeeded.
 *
 * @param[in]   :  "params": {"powerState": "STANDBY"}
 * @return      :  {"success":true}
 */

TEST_F(SystemServicesTest, setPowerStateSuccess_with_PreferedStandbyMode)
{
    device::SleepMode mode;
    NiceMock<SleepModeMock> sleepModeMock;
    mode.impl = &sleepModeMock;
    string sleepModeString(_T("DEEP_SLEEP"));

    ON_CALL(hostImplMock, getPreferredSleepMode)
        .WillByDefault(::testing::Return(mode));
    ON_CALL(sleepModeMock, toString)
        .WillByDefault(::testing::ReturnRef(sleepModeString));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetPowerState)));
                auto param = static_cast<IARM_Bus_PWRMgr_SetPowerState_Param_t*>(arg);
                EXPECT_EQ(param->newState, IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP);
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setPowerState"), _T("{\"powerState\":\"STANDBY\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
/*Test cases for setPowerState ends here*/

TEST_F(SystemServicesTest, getPowerStateIsManagedByDevice)
{
    unsetenv("RDK_ACTION_ON_POWER_KEY");
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPowerStateIsManagedByDevice"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"powerStateManagedByDevice\":false,\"success\":true}"));
    setenv("RDK_ACTION_ON_POWER_KEY", "1", 1);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPowerStateIsManagedByDevice"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"powerStateManagedByDevice\":true,\"success\":true}"));
}

TEST_F(SystemServicesTest, getPowerStateBeforeReboot)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_GetPowerStateBeforeReboot)));
                auto param = static_cast<IARM_Bus_PWRMgr_GetPowerStateBeforeReboot_Param_t*>(arg);
                strncpy(param->powerStateBeforeReboot, "DEEP_SLEEP", sizeof(param->powerStateBeforeReboot));
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPowerStateBeforeReboot"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"state\":\"DEEP_SLEEP\",\"success\":true}"));
}

TEST_F(SystemServicesTest, setWakeupSrcConfiguration)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_SetWakeupSrcConfig)));
                auto param = static_cast<IARM_Bus_PWRMgr_WakeupSrcConfig_Param_t*>(arg);
                EXPECT_EQ(param->pwrMode, (1<<IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP));
                EXPECT_EQ(param->srcType, (1<<WAKEUPSRC_VOICE));
                EXPECT_EQ(param->config, (1<<WAKEUPSRC_VOICE));
                return IARM_RESULT_SUCCESS;
            });

    // TODO: BUG. enum should be used
    // TODO: BUG. boolean should not be number string
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setWakeupSrcConfiguration"), _T("{\"powerState\":\"DEEP_SLEEP\",\"wakeupSources\":[{\"WAKEUPSRC_VOICE\":true}]}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, getStoreDemoLink)
{
    Core::File file(string("/opt/persistent/store-mode-video/videoFile.mp4"));
    if (file.Exists()) {
        EXPECT_TRUE(file.Destroy());
    }

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getStoreDemoLink"), _T("{}"), response));

    Core::Directory(file.PathName().c_str()).CreatePath();
    file.LoadFileInfo();
    file.Create();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStoreDemoLink"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"fileURL\":\"file:\\/\\/\\/opt\\/persistent\\/store-mode-video\\/videoFile.mp4\",\"success\":true}"));
}

TEST_F(SystemServicesTest, deletePersistentPath)
{
    NiceMock<ServiceMock> service;
    NiceMock<ServiceMock> amazonService;
    string amazonPersistentPath(_T("/tmp/amazonPersistentPath"));

    EXPECT_EQ(string(""), plugin->Initialize(&service));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("deletePersistentPath"), _T("{}"), response));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("deletePersistentPath"), _T("{\"callsign\":\"\"}"), response));

    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                EXPECT_EQ(name, string(_T("Amazon")));
                return &amazonService;
            }));
    ON_CALL(amazonService, PersistentPath())
        .WillByDefault(::testing::Return(amazonPersistentPath));
    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/container_setup.sh Amazon")));
                return 0;
            }));

    Core::Directory dir(amazonPersistentPath.c_str());
    EXPECT_TRUE(dir.Destroy(false));
    ASSERT_TRUE(dir.CreatePath());
    EXPECT_TRUE(Core::File(amazonPersistentPath).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("deletePersistentPath"), _T("{\"callsign\":\"Amazon\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_FALSE(Core::File(amazonPersistentPath).Exists());

    plugin->Deinitialize(&service);
}

/*********************************************************************************************************
 * Test function for :onSystemPowerStateChanged
 * onSystemPowerStateChanged :
 *                Triggered when the power manager detects a device power state change.
 *                The power state (must be one of the following: STANDBY, DEEP_SLEEP, LIGHT_SLEEP, ON)
 *
 * Use case coverage:
 *                @Success :4
 *                @Failure :0
 ********************************************************************************************************/

/**
 * @brief :Triggered when the power state changes from DEEPSLEEP to ON.
 *         Check when powerEventHandler function called with power state change event from DEEPSLEEP to ON;
 *         then  onSystemPowerStateChanged event shall be triggered successfully and
 *         the expected JSON message is sent
 */
TEST_F(SystemServicesEventIarmTest, onSystemPowerStateChanged_From_DEEPSLEEP_to_ON)
{
    Core::Event onSystemPowerStateChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemPowerStateChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"powerState\":\"ON\","
                                                             "\"currentPowerState\":\"DEEP_SLEEP\""
                                                             "\\}"
                                                             "\\}")));

                onSystemPowerStateChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onSystemPowerStateChanged.Lock());

    handler.Unsubscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief :Triggered when the power state changes from ON to LIGHTSLEEP
 *         Check when powerEventHandler function called with power state change event from ON to LIGHTSLEEP;
 *         then  onSystemPowerStateChanged event shall be triggered successfully and
 *         the expected JSON message is sent
 */
TEST_F(SystemServicesEventIarmTest, onSystemPowerStateChanged_PowerState_ON_To_LIGHTSLEEP)
{
    Core::Event onSystemPowerStateChanged(false, true);

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemPowerStateChanged\","

                                                             "\"params\":"
                                                             "\\{"
                                                             "\"powerState\":\"LIGHT_SLEEP\","
                                                             "\"currentPowerState\":\"ON\""
                                                             "\\}"
                                                             "\\}")));

                onSystemPowerStateChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onSystemPowerStateChanged.Lock());

    handler.Unsubscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);
}

/**
 * @brief :Triggered when the power state changes from STANDBY to LIGHTSLEEP
 *         Check when powerEventHandler function called with power state change event from STANDBY to LIGHTSLEEP;
 *         then  onSystemPowerStateChanged event shall be triggered successfully and
 *         the expected JSON message is sent
 */
TEST_F(SystemServicesEventIarmTest, onSystemPowerStateChanged_PowerState_STANDBY_To_LIGHTSLEEP)
{
    Core::Event onSystemPowerStateChanged(false, true);
    EXPECT_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
    .Times(0);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemPowerStateChanged\","

                                                             "\"params\":"
                                                             "\\{"
                                                             "\"powerState\":\"LIGHT_SLEEP\","
                                                             "\"currentPowerState\":\"LIGHT_SLEEP\""
                                                             "\\}"
                                                             "\\}")));

                onSystemPowerStateChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onSystemPowerStateChanged.Lock());

    handler.Unsubscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);
}
/**
 * @brief :Triggered when the power state changes from ON to DEEPSLEEP
 *         Check when powerEventHandler function called with power state change event from ON to DEEPSLEEP;
 *         then  onSystemPowerStateChanged event shall be triggered successfully and
 *         the expected JSON message is sent
 */
TEST_F(SystemServicesEventIarmTest, onSystemPowerStateChanged_PowerState_ON_To_DEEPSLEEP)
{
    Core::Event onSystemPowerStateChanged(false, true);
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();

    EXPECT_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
    .Times(0);

    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();

    ofstream dcmPropertiesFile("/etc/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile.close();

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemPowerStateChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"powerState\":\"DEEP_SLEEP\","
                                                             "\"currentPowerState\":\"ON\""
                                                             "\\}"
                                                             "\\}")));

                onSystemPowerStateChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.state.curState = IARM_BUS_PWRMGR_POWERSTATE_ON;
    param.data.state.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_MODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onSystemPowerStateChanged.Lock());

    handler.Unsubscribe(0, _T("onSystemPowerStateChanged"), _T("org.rdk.System"), message);
}
/*Test cases for onSystemPowerStateChanged ends here*/

TEST_F(SystemServicesEventIarmTest, onNetworkStandbyModeChanged)
{
    Core::Event onNetworkStandbyModeChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onNetworkStandbyModeChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"nwStandby\":true"
                                                             "\\}"
                                                             "\\}")));

                onNetworkStandbyModeChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onNetworkStandbyModeChanged"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_EventData_t param;
    param.data.bNetworkStandbyMode = true;
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_NETWORK_STANDBYMODECHANGED, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onNetworkStandbyModeChanged.Lock());

    handler.Unsubscribe(0, _T("onNetworkStandbyModeChanged"), _T("org.rdk.System"), message);
}

TEST_F(SystemServicesEventIarmTest, onRebootRequest)
{
    Core::Event onRebootRequest(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onRebootRequest\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"requestedApp\":\"test\","
                                                             "\"rebootReason\":\"test\""
                                                             "\\}"
                                                             "\\}")));

                onRebootRequest.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onRebootRequest"), _T("org.rdk.System"), message);

    IARM_Bus_PWRMgr_RebootParam_t param;
    strncpy(param.requestor, "test", sizeof(param.requestor));
    strncpy(param.reboot_reason_other, "test", sizeof(param.reboot_reason_other));
    powerEventHandler(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_EVENT_REBOOTING, &param, 0);

    EXPECT_EQ(Core::ERROR_NONE, onRebootRequest.Lock());

    handler.Unsubscribe(0, _T("onRebootRequest"), _T("org.rdk.System"), message);
}


 /*******************************************************************************************************************
 * Test function for :getDeviceInfo
 * getDeviceInfo :
 *                The API which Collects device details such as bluetooth_mac, boxIP, build_type, estb_mac,
 *                imageVersion, rf4ce_mac, and wifi_mac.
 *
 *                @return An object containing the device details.
 * Use case coverage:
 *                @Success :9
 *                @Failure :5
 ********************************************************************************************************************/

/**
 * @brief : getDeviceInfo when QueryParam Contains Unallowable Character.
 *          Check if input query Param passed with unallowable characters,
 *          then  getDeviceInfo shall be failed and returns an error message
 *          in the response
 *
 * @param[in]   :  "params":{"params": {"abc#$"}}
 * @return      :  {"message":"Input has unallowable characters","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnQueryParamContainsUnallowableCharacter)
{
   EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":abc#$}"), response));
}

/**
 * @brief : getDeviceInfo with Invalid query Param
 *          Check if Invalid query  parameters passed ,
 *          then getDeviceInfo shall be failed without giving any response
 *
 * @param[in]   :  "params":{"params": {friendId"}}
 * @return      :  {"success":false}")
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnInvalidQueryParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":friendId}"), response));
}

/**
 * @brief : getDeviceInfo  When Query Parameter is make ,but Missing Key in property File
 *          Check if 'make' parameter contains missing key: MFG_NAME
 *          then ,getDeviceInfo shall be failed and returns an error message in the response
 *
 * @param[in]   : "params":{"params": "make"}
 * @return      : {"message":"Missing required key/value(s)","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnMissingKeyInDevicePropertyFile)
{
    ofstream file("/etc/device.properties");
    file << "MFGNAME=\"SKY\"";
    file.close();

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":make}"), response));
}

/**
 * @brief : getDeviceInfo  When Query Parameter is make ,but Missing KeyValue in property File
 *          check if   'make' parameter contains missing key value
 *          then ,getDeviceInfo shall be failed and returns an error message in the response
 *
 * @param[in]   :  "params":{"params": "make"}
 * @return      :  {"message":"Missing required key/value(s)","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnMissingKeyValueInDevicePropertyFile)
{
    ofstream file("/etc/device.properties");
    file << "MFG_NAME=";
    file.close();

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":make}"), response));
}

/**
 * @brief : getDeviceInfo When ManufactureDataReadAPI Failed For GetModelName/HardwareID
 *          Check if (i) input parametr as Model name/Hardware ID and
 *          (ii) Manufacture Data Read API[IARM_BUS_MFRLIB_API_GetSerializedData] failed
 *          then, getDeviceInfo shall be failed and returns an error message in the response
 * @param[in]   : "params":{"params": "modelName"}
 * @return      :  {"message":"Manufacturer Data Read Failed","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnManufactureDataReadAPIFailed)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
               EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
               EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
               //setting up a mock that always returns an error code.
               return IARM_RESULT_IPCCORE_FAIL;
            });
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":hardwareID}"), response));
}

/**
 * @brief : getDeviceInfo When QueryParam is Make
 *          Check if query parameter = make
 *          then,  getDeviceInfo shall succesfully retrieve the information from device property file
 * @param[in]   :  "params":{"params": "make"}
 * @return      : {"make":"SKY","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onMakeParameter)
{
    ofstream file("/etc/device.properties");
    file << "MFG_NAME=SKY";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":make}"), response));
    EXPECT_EQ(response, string("{\"make\":\"SKY\",\"success\":true}"));
}

/**
 * @brief : getDeviceInfo With Valid QueryParam other than make
 *          Check if valid query parameter is passed,
 *          then getDeviceInfo shall successfully retrieve  the corresponding value from getDeviceDetails script file
 *          and returns it in the response.
 *          Check if valid query parameter string contains  any of the following characters [\"] ,
 *          then  getDeviceInfo shall remove those characters from input param
 *          and then successfully retrieve  the corresponding value and returns it in the response
 *          Tested with following valid input params: {"bluetooth_mac","boxIP","build_type","estb_mac","eth_mac","friendly_id","imageVersion","version","software_version","model_number","wifi_mac"}
 * @param[in]   : "params":{"params": "estb_mac"} /
 *              : "params":{"params": "[estb_mac]"}
 * @return      : {"estb_mac":"20:F1:9E:EE:62:08","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onValidInput)
{

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
         .Times(::testing::AnyNumber())
         .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                  EXPECT_EQ(string(command), string("sh /lib/rdk/getDeviceDetails.sh read estb_mac"));
                  // Simulated the behavior of "getDeviceDetails.sh" script inorder to obtain the value of estb_mac key
                  const char key_estb_mac[] = "12:34:56:78:90:AB";
                  char buffer[1024];
                  memset(buffer, 0, sizeof(buffer));
                  strncpy(buffer, key_estb_mac, sizeof(buffer) - 1);
                  FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                  return pipe;
               }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":estb_mac}"), response));
    EXPECT_EQ(response, string("{\"estb_mac\":\"12:34:56:78:90:AB\",\"success\":true}"));
}

/**
 * @brief : getDeviceInfo  When QueryParam is HardwareId
 *          Check if device's HardwareId as input query param,
 *          then getDeviceInfo shall succeed and retrieves the information from  the external Bus device API
 *          and returns it in the response.
 * @param[in]   : "params":{"params": "hardwareID"}
 * @return      :  {"hardwareId":"5678","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onQueryParamHardwareId)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "5678";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, sizeof(param->buffer));
                param->type =  mfrSERIALIZED_TYPE_HWID;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":hardwareID}"), response));
    EXPECT_EQ(response, string("{\"hardwareID\":\"5678\",\"success\":true}"));
}

/**
 * @brief : getDeviceInfo  When QueryParam is FriendlyId/ModelName
 *          Check if device's FriendlyId/ModelName as input query param,
 *          then getDeviceInfo shall succeed and retrieves the information from  the external Bus device API
 *          and returns it in the response.
 * @param[in]   : "params": {"params": "friendly_id"}
 *              : "params": {"params": "model_name"}
 * @return      :  {"friendly_id":"IP061-ec","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onQueryParamFriendlyIdOrModelName)
{
   ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "IP061-ec";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, sizeof(param->buffer));
                param->type =  mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":friendly_id}"), response));
    EXPECT_EQ(response, string("{\"friendly_id\":\"IP061-ec\",\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":modelName}"), response));
    EXPECT_EQ(response, string("{\"modelName\":\"IP061-ec\",\"success\":true}"));

}

/**
 * @brief : getDeviceInfo returns Cached data When QueryParam is HardwareId
 *          Check (i) If input query param is  device's Hardware ID
 *          and (ii) if cached data of  hardware Id is available
 *          then , getDeviceInfo shall successfully retrieves the cached information and returns it in the response
 * @param[in]   : "params": {"params": "hardwareID"}
 * @return      : {"hardwareId":"5678","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_getCachedHardwareId)
{
   //Below IARM_Bus_Call function is called for saving the retrieved data
   //in member variables [cached value] & setting the corresponding flags to true
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "5678";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, sizeof(param->buffer));
                param->type =  mfrSERIALIZED_TYPE_HWID;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":hardwareID}"), response));
    EXPECT_EQ(response, string("{\"hardwareID\":\"5678\",\"success\":true}"));

    //To confirm that the retrieved data is cached Data;
    //sets an expectation that the IARM_Bus_Call function should not be called during this sequence
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":hardwareID}"), response));
    EXPECT_EQ(response, string("{\"hardwareID\":\"5678\",\"success\":true}"));

}

/**
 * @brief : getDeviceInfo returns Cached data When QueryParam is ModelName
 *          Check (i) If input query param is  device's ModelName
 *          and (ii) if cached data of  Model Name is available
 *          then , getDeviceInfo shall successfully retrieves the cached information and returns it in the response
 * @param[in]   :  "params": {"params": "modelName"}
 * @return      :  {"modelName":"IP061-ec","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_getCachedModelName)
{
   //Below IARM_Bus_Call function is called for saving the retrieved data
   //in member variables [cached value] & setting the corresponding flags to true
   ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "IP061-ec";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, sizeof(param->buffer));
                param->type =  mfrSERIALIZED_TYPE_PROVISIONED_MODELNAME;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":modelName}"), response));
    EXPECT_EQ(response, string("{\"modelName\":\"IP061-ec\",\"success\":true}"));

    //To confirm that the retrieved data is cached Data;
    //sets an expectation that the IARM_Bus_Call function should not be called during this sequence
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":modelName}"), response));
    EXPECT_EQ(response, string("{\"modelName\":\"IP061-ec\",\"success\":true}"));
}

/**
 * @brief : getDeviceInfo  When QueryParam passed without label "param"
 *          Check if QueryParams  contains no label as "params"
 *          then getDeviceInfo shall successfully retrieve the device info and  returns it in the response
 * @param[in]   :   "params" :{}
 * @return      : {"make":"SKY","bluetooth_mac":"D4:52:EE:32:A3:B2",
 *                                     "boxIP":"192.168.1.100","build_type":"VBN",
 *                                     "estb_mac":"D4:52:EE:32:A3:B0","eth_mac":"D4:52:EE:32:A3:B0",
 *                                     "friendly_id":"IP061-ec","imageVersion":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "software_version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "model_number":"SKXI11ANS","wifi_mac":"D4:52:EE:32:A3:B1","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onQueryParameterHasNoLabelParam)
{
    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
    const uint8_t deviceInfoContent[] = "echo \"bluetooth_mac=D4:52:EE:32:A3:B2\n"
                                         "boxIP=192.168.1.0\n"
                                         "build_type=VBN\n"
                                         "estb_mac=D4:52:EE:32:A3:B0\n"
                                         "eth_mac=D4:52:EE:32:A3:B0\n"
                                         "friendly_id=IP061-ec\n"
                                         "imageVersion=SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\n"
                                         "model_number=SKXI11ANS\n"
                                         "wifi_mac=D4:52:EE:32:A3:B1\"\n";

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
           [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("sh /lib/rdk/getDeviceDetails.sh read")));
                return __real_popen(command, type);
            }));

     //Create fake device property file
     ofstream propFile("/etc/device.properties");
     propFile << "MFG_NAME=SKY";
     propFile.close();

     //Create fake device info script
     Core::File file(deviceInfoScript);
     file.Create();
     file.Write(deviceInfoContent, sizeof(deviceInfoContent));

     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{}"), response));
     EXPECT_EQ(response, _T("{\"make\":\"SKY\",\"bluetooth_mac\":\"D4:52:EE:32:A3:B2\",\"boxIP\":\"192.168.1.0\",\"build_type\":\"VBN\",\"estb_mac\":\"D4:52:EE:32:A3:B0\",\"eth_mac\":\"D4:52:EE:32:A3:B0\",\"friendly_id\":\"\",\"imageVersion\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"software_version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"model_number\":\"SKXI11ANS\",\"wifi_mac\":\"D4:52:EE:32:A3:B1\",\"success\":true}"));
     file.Destroy();
}

/**
 * @brief : getDeviceInfo  When QueryParam passed without any value for "params"
 *          Check if no value is passed with input query param,
 *          then getDeviceInfo shall succesfully retrieve the information from both device property file and
 *          getDeviceDetails script file ,then returns it in the response
 * @param[in]   : "params": {"params": }
 * @return      : {"make":"SKY","bluetooth_mac":"D4:52:EE:32:A3:B2",
 *                                     "boxIP":"192.168.1.100","build_type":"VBN",
 *                                     "estb_mac":"D4:52:EE:32:A3:B0","eth_mac":"D4:52:EE:32:A3:B0",
 *                                     "friendly_id":"IP061-ec","imageVersion":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "software_version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "model_number":"SKXI11ANS","wifi_mac":"D4:52:EE:32:A3:B1","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_onNoValueForQueryParameter)
{
    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
    const uint8_t deviceInfoContent[] = "echo \"bluetooth_mac=D4:52:EE:32:A3:B2\n"
                                         "boxIP=192.168.1.0\n"
                                         "build_type=VBN\n"
                                         "estb_mac=D4:52:EE:32:A3:B0\n"
                                         "eth_mac=D4:52:EE:32:A3:B0\n"
                                         "friendly_id=IP061-ec\n"
                                         "imageVersion=SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\n"
                                         "model_number=SKXI11ANS\n"
                                         "wifi_mac=D4:52:EE:32:A3:B1\"\n";

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
           [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("sh /lib/rdk/getDeviceDetails.sh read")));
                return __real_popen(command, type);
            }));

    //Create fake device property file
    ofstream propFile("/etc/device.properties");
    propFile << "MFG_NAME=SKY";
    propFile.close();

    //Create fake device info script
    Core::File file(deviceInfoScript);
    file.Create();
    file.Write(deviceInfoContent, sizeof(deviceInfoContent));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":}"), response));
    EXPECT_EQ(response, _T("{\"make\":\"SKY\",\"bluetooth_mac\":\"D4:52:EE:32:A3:B2\",\"boxIP\":\"192.168.1.0\",\"build_type\":\"VBN\",\"estb_mac\":\"D4:52:EE:32:A3:B0\",\"eth_mac\":\"D4:52:EE:32:A3:B0\",\"friendly_id\":\"\",\"imageVersion\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"software_version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"model_number\":\"SKXI11ANS\",\"wifi_mac\":\"D4:52:EE:32:A3:B1\",\"success\":true}"));
    file.Destroy();
}

/**
 * @brief : getDeviceInfo When QueryParams Value is Empty and getDeviceDetails Script contains some specific key-value pairs [ImageVersion,CableCardVersion/ModelNumber]
 *          Check if No value in input parameter and getDeviceDetails Script Contain key value = ImageVersion/ key value = CableCardVersion/ key value = ModelNumber
 *          then getDeviceInfo shall successfully retrieve the device info and returns it in the response where
 *          ImageVersion stored in keys, "version" and "software_version" ,"cable_card_firmware_version","model_number" respectively
 * @param[in]   :  "params": {"params" : {}}
 * @return      : {"make":"SKY","bluetooth_mac":"D4:52:EE:32:A3:B2",
 *                                     "boxIP":"192.168.1.100","build_type":"VBN",
 *                                     "estb_mac":"D4:52:EE:32:A3:B0","eth_mac":"D4:52:EE:32:A3:B0",
 *                                     "friendly_id":"IP061-ec","imageVersion":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "software_version":"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI",
 *                                     "model_number":"SKXI11ANS","wifi_mac":"D4:52:EE:32:A3:B1","success":true}
 */
TEST_F(SystemServicesTest, getDeviceInfoSuccess_OnSpecificKeyValueParsing)
{
    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
    const uint8_t deviceInfoContent[] = "echo \"bluetooth_mac=D4:52:EE:32:A3:B2\n"
                                         "boxIP=192.168.1.0\n"
                                         "build_type=VBN\n"
                                         "estb_mac=D4:52:EE:32:A3:B0\n"
                                         "eth_mac=D4:52:EE:32:A3:B0\n"
                                         "friendly_id=IP061-ec\n"
                                         "imageVersion=SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\n"
                                         "model_number=SKXI11ANS\n"
                                         "wifi_mac=D4:52:EE:32:A3:B1\"\n";
    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
           [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("sh /lib/rdk/getDeviceDetails.sh read")));
                return __real_popen(command, type);
            }));

    //Create fake device property file
    ofstream propFile("/etc/device.properties");
    propFile << "MFG_NAME=SKY";
    propFile.close();

    //Create fake device info script
    Core::File file(deviceInfoScript);
    file.Create();
    file.Write(deviceInfoContent, sizeof(deviceInfoContent));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{\"params\":}"), response));
    EXPECT_EQ(response, _T("{\"make\":\"SKY\",\"bluetooth_mac\":\"D4:52:EE:32:A3:B2\",\"boxIP\":\"192.168.1.0\",\"build_type\":\"VBN\",\"estb_mac\":\"D4:52:EE:32:A3:B0\",\"eth_mac\":\"D4:52:EE:32:A3:B0\",\"friendly_id\":\"\",\"imageVersion\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"software_version\":\"SKXI11ANS_VBN_23Q1_sprint_20230129224229sdy_SYNA_CI\",\"model_number\":\"SKXI11ANS\",\"wifi_mac\":\"D4:52:EE:32:A3:B1\",\"success\":true}"));
    file.Destroy();
}

/*Test cases for getDeviceInfo ends here*/

/*******************************************************************************************************************
 * Test function for :requestSystemReboot
 * requestSystemReboot :
 *                Requests that the system performs a reboot of the set-top box.
 *                Triggering onRebootRequest event.
 *
 *                @return IARM BUS status and Whether the request succeeded.
 * Use case coverage:
 *                @Success :6
 *                @Failure :0
 ********************************************************************************************************************/

/**
 * @brief :requestSystemReboot when "nrdPluginApp" process is NOT running.
 *        Check if "nrdPluginApp" process is not running ensure that the system reboot is initiated
 *        without any issues and returns the BUS call status in the response.
 *
 * @param[in]   :  "params": {}
 * @return      :  {"IARM_Bus_Call_STATUS":0,"success":true}
 */
TEST_F(SystemServicesTest, requestSystemRebootSuccess_NrdPluginAppNotRunning)
{
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(1)
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                return IARM_RESULT_SUCCESS;
            });
     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{}"), response));
     EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":0,\"success\":true}"));
}

/**
 * @brief :requestSystemReboot when "nrdPluginApp" process is running & It can't be terminated successfully
 *        Check if NrdPlugin App is Running & if unable to shutdown the app,
 *        then ensure that the system reboot is initiated
 *        without any issues and returns the BUS call status in the response.
 *
 * @param[in]   :  "params": {}
 * @return      :  {"IARM_Bus_Call_STATUS":0,"success":true}
 */
TEST_F(SystemServicesTest, requestSystemRebootSuccess_NrdPluginAppShutdownFailed)
{
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(0));
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pkill nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(1)
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                return IARM_RESULT_SUCCESS;
            });
     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{}"), response));
     EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":0,\"success\":true}"));
}
/**
 * @brief :requestSystemReboot when "nrdPluginApp" process is running & It can be terminated successfully
 *        Check if NrdPlugin App is Running ,then ensure that it is terminated
 *        properly before initiating the system reboot.
 *        and returns the BUS call status in the response.
 * @param[in]   :  "params": {}
 * @return      :  {"IARM_Bus_Call_STATUS":0,"success":true}
 */
TEST_F(SystemServicesTest, requestSystemRebootSuccess_NrdPluginAppShutdownSuccess)
{
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(0));
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pkill nrdPluginApp")))
      .WillByDefault(::testing::Return(0));
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(1)
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                return IARM_RESULT_SUCCESS;
            });
     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{}"), response));
     EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":0,\"success\":true}"));
}

/**
 * @brief :requestSystemReboot when reason is not passed
 *        Check if (i) input parameter is empty then requestSystemReboot
 *        shall be succeeded and returns the BUS call status in the response.
 *
 * @param[in]   :  "params": {}
 * @return      :  {"IARM_Bus_Call_STATUS":0,"success":true}
 */
TEST_F(SystemServicesTest, requestSystemRebootSuccess_withoutReason)
{
     // Ignore the application shutdown process here because it would add extra time
     // to the test execution and is not relevant to this particular test case.
     ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));

     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(1)
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                return IARM_RESULT_SUCCESS;
            });
   EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{}"), response));
   EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":0,\"success\":true}"));
}
/**
 * @brief :requestSystemReboot when reason is passed
 *        Check if (i)reboot reason is passed as input parameter and
 *        (ii) if Bus call status returns as SUCCESS
 *        then requestSystemReboot shall be succeeded and returns the BUS call status in the response.
 *
 * @param[in]   :  "params": {"rebootReason": "FIRMWARE_FAILURE"}
 * @return      :  {"IARM_Bus_Call_STATUS":0,"success":true}
 */
TEST_F(SystemServicesTest, requestSystemRebootSuccess_withReason)
{
      // Ignore the application shutdown process here because it would add extra time
      // to the test execution and is not relevant to this particular test case.
      ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));

      ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                auto rebootParam = static_cast<IARM_Bus_PWRMgr_RebootParam_t*>(arg);
                EXPECT_THAT(string(rebootParam->requestor), "SystemServices");
                EXPECT_THAT(string(rebootParam->reboot_reason_custom), "FIRMWARE_FAILURE");
                EXPECT_THAT(string(rebootParam->reboot_reason_other), "FIRMWARE_FAILURE");
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{\"rebootReason\":\"FIRMWARE_FAILURE\"}"), response));
    EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":0,\"success\":true}"));
}

/**
 * @brief :requestSystemReboot when reason is passed and Bus API failed
 *        Check if (i)reboot reason is passed as input parameter and
 *        (ii) if Bus call status returns some error_codes
 *        then requestSystemReboot shall be succeeded and returns the respective Bus call status
 *        [IARM_RESULT_INVALID_PARAM/IARM_RESULT_INVALID_STATE/IARM_RESULT_IPCCORE_FAIL/IARM_RESULT_OOM]
 *
 * @param[in]   :  "params": {"rebootReason": "FIRMWARE_FAILURE"}
 * @return      :  {"IARM_Bus_Call_STATUS":4,"success":true}
 *
 */
TEST_F(SystemServicesTest,  requestSystemRebootSuccess_onRebootBusAPIFailed)
{
      // Ignore the application shutdown process here because it would add extra time
      // to the test execution and is not relevant to this particular test case.
      ON_CALL(wrapsImplMock, system(::testing::StrEq("pgrep nrdPluginApp")))
      .WillByDefault(::testing::Return(-1));
      ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_PWRMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_PWRMGR_API_Reboot)));
                return IARM_RESULT_INVALID_STATE;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reboot"), _T("{\"rebootReason\":\"FIRMWARE_FAILURE\"}"), response));
    EXPECT_EQ(response, string("{\"IARM_Bus_Call_STATUS\":2,\"success\":true}"));
}

/*Test cases for requestSystemReboot ends here*/

/*******************************************************************************************************************
 * Test function for :getStateInfo
 * getStateInfo :
 *                Queries device state information of various properties.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :27
 *                @Failure :2
 ********************************************************************************************************************/

/**
 * @brief : getStateInfo When QueryParam is Empty
 *       Check if QueryParam is not passed
 *       then,getStateInfo shall be failed and returns the error code:SysSrv_MissingKeyValues
 *       in the response
 *
 * @param[in]   : "params": {}
 * @return      : {"SysSrv_Status":2,"errorMessage":"Missing required key\/value(s)","success":false}}
 */
TEST_F(SystemServicesTest, getStateInfoFailed_onEmptyParamList)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getStateInfo"), _T("{}"), response));
}

/**
 * @brief : getStateInfo when Invalid query Param is passed
 *        Check if Invalid query parameters passed,
 *        then getStateInfo shall be failed and returns the error code: SysSrv_Unexpected  in the response
 *
 * @param[in]   :  "params": {"card.disconnected"}
 * @return      :  {"SysSrv_Status":7,"errorMessage":"Unexpected error","success":false}
 */
TEST_F(SystemServicesTest, getStateInfoFailed_OnInvalidQueryParam)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getStateInfo"), _T("{}"), response));
}

/**
 * @brief : getStateInfo When QueryParam is channel_map
 *        Check if valid query parameter com.comcast.channel_map is passed,
 *        then the function makes a bus call to retrieve the channel map state,
 *        getStateInfo shall be succeed and returns the channel map state in response.
 *
 * @param[in]   :  "params": {"com.comcast.channel_map"}
 * @return      :  {"com.comcast.channel_map":2,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamChannelMap)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(1)
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_SYSMGR_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_SYSMGR_API_GetSystemStates)));
                auto* paramGetSysState = static_cast<IARM_Bus_SYSMgr_GetSystemStates_Param_t*>(arg);
                paramGetSysState->channel_map.state = 2;
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"), _T("{\"param\":com.comcast.channel_map}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.channel_map\":2,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is card.disconnected
 *        Check if valid query parameter card.disconnected passed,
 *        then getStateInfo shall be succeed and returns card.disconnected state in response.
 *
 * @param[in]   :  "params": {"com.comcast.card.disconnected"}
 * @return      :  {"com.comcast.card.disconnected":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCardDisconnected)
{
        /*sets the expectation that IARM_Bus_Call should not be called*/
        EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
         EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.card.disconnected}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.card.disconnected\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is tune_ready
 *        Check if valid query parameter tune_ready is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.tune_ready"}
 * @return      :  {"com.comcast.tune_ready":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamTuneReady)
{
        /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.tune_ready}"), response));
     EXPECT_EQ(response, string("{\"com.comcast.tune_ready\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is cmac
 *        Check if valid query parameter cmac is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.cmac"}
 * @return      :  {"com.comcast.cmac":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCmac)
{
      /*sets the expectation that IARM_Bus_Call should not be called*/
      EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
      EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.cmac}"), response));
      EXPECT_EQ(response, string("{\"com.comcast.cmac\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is card.moto.entitlements
 *        Check if valid query parameter card.moto.entitlements is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.card.moto.entitlements"}
 * @return      :  {"com.comcast.card.moto.entitlements":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCardMotoEntitlements)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.card.moto.entitlements}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.card.moto.entitlements\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is card.moto.hrv_rx
 *        Check if valid query parameter card.moto.hrv_rx is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.card.moto.hrv_rx"}
 * @return      :  {"com.comcast.card.moto.hrv_rx":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCardMotoHrvRx)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.card.moto.hrv_rx}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.card.moto.hrv_rx\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is card.cisco.status
 *        Check if valid query parameter card.cisco.status is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.card.cisco.status"}
 * @return      :  {"com.comcast.card.cisco.status":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCardCiscoStatus)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.card.cisco.status}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.card.cisco.status\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is video_presenting
 *        Check if valid query parameter video_presenting is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.video_presenting"}
 * @return      :  {"com.comcast.video_presenting":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamVideoPresenting)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.video_presenting}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.video_presenting\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is hdmi_out
 *        Check if valid query parameters hdmi_out is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.hdmi_out"}
 * @return      :  {"com.comcast.hdmi_out":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamHdmiOut)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.hdmi_out}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.hdmi_out\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is hdcp_enabled
 *        Check if valid query parameters hdcp_enabled is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.hdcp_enabled"}
 * @return      :  {"com.comcast.hdcp_enabled":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamHdcpEnabled)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.hdcp_enabled}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.hdcp_enabled\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is hdmi_edid_read
 *        Check if valid query parameter hdmi_edid_read is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.hdmi_edid_read"}
 * @return      :  {"com.comcast.hdmi_edid_read":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamHdmiEdidRead)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.hdmi_edid_read}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.hdmi_edid_read\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is firmware_download
 *        Check if valid query parameters firmware_download is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.firmware_download"}
 * @return      :  {"com.comcast.firmware_download":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamFirmwareDownload)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.firmware_download}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.firmware_download\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is time_source
 *        Check if valid query parameter time_source is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.time_source"}
 * @return      :  {"com.comcast.time_source":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamTimeSource)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.time_source}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.time_source\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is time_zone_available
 *        Check if valid query parameter time_zone_available is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.time_zone_available"}
 * @return      :  {"com.comcast.time_zone_available":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamTimeZoneAvailable)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.time_zone_available}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.time_zone_available\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is ca_system
 *        Check if valid query parameter ca_system is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.ca_system"}
 * @return      :  {"com.comcast.ca_system":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCaSystem)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.ca_system}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.ca_system\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is estb_ip
 *        Check if valid query parameters estb_ip is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.estb_ip"}
 * @return      :  {"com.comcast.estb_ip":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamEstbIp)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.estb_ip}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.estb_ip\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is ecm_ip
 *        Check if valid query parameter ecm_ip is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.ecm_ip"}
 * @return      :  {"com.comcast.ecm_ip":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamEcmIp)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.ecm_ip}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.ecm_ip\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is lan_ip
 *        Check if valid query parameters lan_ip is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.lan_ip"}
 * @return      :  {"com.comcast.lan_ip":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamLanIp)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.lan_ip}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.lan_ip\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is moca
 *        Check if valid query parameter moca is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.moca"}
 * @return      :  {"com.comcast.moca":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamMoca)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.moca}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.moca\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is docsis
 *        Check if valid query parameter docsis is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.docsis"}
 * @return      :  {"com.comcast.docsis":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamDocsis)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.docsis}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.docsis\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is dsg_broadcast_tunnel
 *        Check if valid query parameter dsg_broadcast_tunnel is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.dsg_broadcast_tunnel"}
 * @return      :  {"com.comcast.dsg_broadcast_tunnel":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamDsgBroadcastTunnel)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.dsg_broadcast_tunnel}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.dsg_broadcast_tunnel\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is dsg_ca_tunnel
 *        Check if valid query parameter dsg_ca_tunnel is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.dsg_ca_tunnel"}
 * @return      :  {"com.comcast.dsg_ca_tunnel":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamDsgCaTunnel)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.dsg_ca_tunnel}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.dsg_ca_tunnel\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is cable_card
 *        Check if valid query parameter cable_card is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.cable_card"}
 * @return      :  {"com.comcast.cable_card":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCableCard)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.cable_card}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.cable_card\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is cable_card_download
 *        Check if valid query parameter cable_card_download is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.cable_card_download"}
 * @return      :  {"com.comcast.cable_card_download":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCableCardDownload)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.cable_card_download}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.cable_card_download\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is cvr_subsystem
 *        Check if valid query parameters cvr_subsystem is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.cvr_subsystem"}
 * @return      :  {"com.comcast.cvr_subsystem":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamCvrSubsystem)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.cvr_subsystem}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.cvr_subsystem\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is download
 *        Check if valid query parameter download is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.download"}
 * @return      :  {"com.comcast.download":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamDownload)
{
    /*sets the expectation that IARM_Bus_Call should not be called*/
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.download}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.download\":0,\"success\":true}"));
}

/**
 * @brief : getStateInfo When QueryParam is vod_ad
 *        Check if valid query parameter vod_ad is passed,
 *        then getStateInfo shall be succeed and returns an success message in the response.
 *
 * @param[in]   :  "params": {"com.comcast.vod_ad"}
 * @return      :  {"com.comcast.vod_ad":0,"success":true}
 */
TEST_F(SystemServicesTest, getStateInfoSuccess_onQueryParamVodAd)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getStateInfo"),  _T("{\"param\":com.comcast.vod_ad}"), response));
    EXPECT_EQ(response, string("{\"com.comcast.vod_ad\":0,\"success\":true}"));
}

/*Test cases for getStateInfo ends here*/

/*******************************************************************************************************************
 * Test function for :setBootLoaderPattern
 * setBootLoaderPattern :
 *                Sets the boot loader pattern mode in MFR.
 *                valid patterns: {"NORMAL","SILENT","SILENT_LED_ON"}
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :3
 *                @Failure :2
 ********************************************************************************************************************/

/**
 * @brief : setBootLoaderPattern when pattern is not passed
 *        Check if pattern is not passed,
 *        then setBootLoaderPattern will Fail.
 *
 * @param[in]   :  "params": {}
 * @return      :  {"success":false}
 */
TEST_F(SystemServicesTest, setBootLoaderPatternFailed_OnEmptyParamList)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBootLoaderPattern"), _T("{}"), response));
}

/**
 * @brief : setBootLoaderPattern when pattern is passed
 *       Check if invalid pattern is passed,
 *       then setBootLoaderPattern will Fail.
 *
 * @param[in]   :  "params": {"pattern": "SILENT_LED_OFF"}
 * @return      :  {"success":false}
 */
TEST_F(SystemServicesTest, setBootLoaderPatternFailed_Oninvalidpattern)
{
     /*sets the expectation that IARM_Bus_Call should not be called*/
     EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(0);
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setBootLoaderPattern"), _T("{\"pattern\":SILENT_LED_OFF}"), response));
}

/**
 * @brief : setBootLoaderPattern when pattern is NORMAL
 *        Check if (i)pattern is NORMAL and
 *        (ii) if Bus call status returns as SUCCESS
 *        then setBootLoaderPattern will Succeed.
 *
 *
 * @param[in]   :  "params": {"pattern": "NORMAL"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setBootLoaderPatternSuccess_onPatterntypeNORMAL)
{
   EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBootLoaderPattern)));
                auto param = static_cast<IARM_Bus_MFRLib_SetBLPattern_Param_t*>(arg);
                EXPECT_EQ(param->pattern, mfrBL_PATTERN_NORMAL);
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBootLoaderPattern"), _T("{\"pattern\":NORMAL}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * @brief : setBootLoaderPattern when pattern is SILENT
 *        Check if (i)pattern is SILENT and
 *        (ii) if Bus call status returns as SUCCESS
 *        then setBootLoaderPattern will Succeed.
 *
 *
 * @param[in]   :  "params": {"pattern": "SILENT"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setBootLoaderPatternSuccess_onPatterntypeSILENT)
{
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBootLoaderPattern)));
                auto param = static_cast<IARM_Bus_MFRLib_SetBLPattern_Param_t*>(arg);
                EXPECT_EQ(param->pattern, mfrBL_PATTERN_SILENT);
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBootLoaderPattern"), _T("{\"pattern\":SILENT}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/**
 * @brief : setBootLoaderPattern when pattern is SILENT_LED_ON
 *        Check if (i)pattern is SILENT_LED_ON and
 *        (ii) if Bus call status returns as SUCCESS
 *        then setBootLoaderPattern will Succeed.
 *
 *
 * @param[in]   :  "params": {"pattern": "SILENT_LED_ON"}
 * @return      :  {"success":true}
 */
TEST_F(SystemServicesTest, setBootLoaderPatternSuccess_onPatterntypeSILENTLEDON)
{

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
        .Times(::testing::AnyNumber())
        .WillRepeatedly(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_SetBootLoaderPattern)));
                auto param = static_cast<IARM_Bus_MFRLib_SetBLPattern_Param_t*>(arg);
                EXPECT_EQ(param->pattern, mfrBL_PATTERN_SILENT_LED_ON);
                return IARM_RESULT_SUCCESS;
            });
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setBootLoaderPattern"), _T("{\"pattern\":SILENT_LED_ON}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}
/*Test cases for setBootLoaderPattern ends here*/

/*************************************************************************************************************
* Test function for :getMacAddresses
* getMacAddresses :
*                Gets the MAC address of the device.
*                @input  GUID[Optional] /Can run without input params
*                @return Whether the request succeeded.
* Event : onMacAddressesRetreived
*                Triggers when the MAC addresses are requested
* Use case coverage:
*                @Failure :1
*                @Success :1
************************************************************************************************************/

/**
 * @brief :getMacAddresses when getDeviceDetails.sh not exist.
 *            Check if the file getDeviceDetails.sh not present,
 *            then getMacAddresses shall be failed.
 *
 * @param[in]   : "params": "{}" 
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesTest,getMacAddressesFailed_WhenFileNotExist)
{
    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
	Core::File file(deviceInfoScript);
	remove("/lib/rdk/getDeviceDetails.sh");
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getMacAddresses"), _T("{}"), response));
	file.Destroy();
}

/**
 * @brief :   onMacAddressesRetrieved event triggered when getMacAddresses is successful
 *            Check if the onMacAddressesRetrieved event is correctly triggered with expected Json params
 *            when MAC addresses are requested successfully.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onMacAddressesRetrieved)
{
    Core::Event onMacAddressesRetreived(false, true);
    const string deviceInfoScript = _T("/lib/rdk/getDeviceDetails.sh");
	Core::File file(deviceInfoScript);
	file.Create();

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
     .WillByDefault(::testing::Invoke(
        [&](const char* command, const char* type) -> FILE* {
            const char* valueToReturn = NULL;
            if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read ecm_mac") == 0) {
                valueToReturn = "A8:11:XX:FD:0C:XX";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read estb_mac") == 0) {
                valueToReturn = "A8:11:XX:FD:0C:XX";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read moca_mac") == 0) {
                valueToReturn = "00:15:5F:XX:20:5E:57:XX";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read eth_mac") == 0) {
                valueToReturn = "A8:11:XX:FD:0C:XX";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read wifi_mac") == 0) {
                valueToReturn = "A8:11:XX:FD:0C:XX";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read bluetooth_mac") == 0) {
                valueToReturn = "AA:AA:AA:AA:AA:AA";
            } else if (strcmp(command, "/lib/rdk/getDeviceDetails.sh read rf4ce_mac") == 0) {
                valueToReturn = "00:00:00:00:00:00";
            }
            if (valueToReturn != NULL) {
                  char buffer[1024];
                  memset(buffer, 0, sizeof(buffer));
                  strncpy(buffer, valueToReturn, sizeof(buffer) - 1);
                  FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                  return pipe;
            } else {
                 return __real_popen(command, type);
            }
        }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onMacAddressesRetreived\",\"params\":\\{\"ecm_mac\":\"A8:11:XX:FD:0C:XX\",\"estb_mac\":\"A8:11:XX:FD:0C:XX\",\"moca_mac\":\"00:15:5F:XX:20:5E:57:XX\",\"eth_mac\":\"A8:11:XX:FD:0C:XX\",\"wifi_mac\":\"A8:11:XX:FD:0C:XX\",\"bluetooth_mac\":\"AA:AA:AA:AA:AA:AA\",\"rf4ce_mac\":\"00:00:00:00:00:00\",\"success\":true\\}}")));
          onMacAddressesRetreived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onMacAddressesRetreived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMacAddresses"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onMacAddressesRetreived.Lock());
    handler.Unsubscribe(0, _T("onMacAddressesRetreived"), _T("org.rdk.System"), message);
	file.Destroy();
}
/*Test cases for getMacAddresses ends here*/

/********************************************************************************************************
* Test function for :getFirmwareUpdateInfo
* getFirmwareUpdateInfo :
*                Checks the firmware update information
*                @input  GUID[Optional] /Can run without input params
*                @return Whether the request succeeded.
* Event : onFirmwareUpdateInfoReceived
*                Triggers when the firmware update information is requested
* Use case coverage:
*                @Failure :0
*                @Success :14
************************************************************************************************************/

/**
 * @brief :  Test that onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful with HTTP status code 460.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WithHttpStatusCode460)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   Test that onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful with HTTP status code 403.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WithHttpStatusCode403)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "403";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":403,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"firmwareUpdateVersion\":\"1234\",\"updateAvailable\":true,\"updateAvailableEnum\":0,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :  onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful with an HTTP status code other than 460 or 403.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WithHttpStatusCodeOther)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "400";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":400,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"firmwareUpdateVersion\":\"1234\",\"updateAvailable\":true,\"updateAvailableEnum\":0,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
    // Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful and the environment is PROD.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenEnvPROD)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_PROD_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful and the environment is DEV.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenEnvDev)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful and the environment is VBN.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenEnvVBN)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful and the environment is CQA.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenEnvCqa)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful with the environment is other than PROD[eg:VBN] and /opt/swupdate.conf is NOT present
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenEnvNotProdWithoutConfFile)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful with the environment is other than PROD[eg:VBN] and /opt/swupdate.conf is present and contains empty URL
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, OnFirmwareUpdateInfoReceived_WhenEnvNotProdWithConfFileEmptyUrl)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    const string  swUpdateFile = _T("/opt/swupdate.conf");
    Core::File file2(swUpdateFile);
    file2.Destroy();
    file2.Create();

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
      EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"\",\"rebootImmediately\":false,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :   onFirmwareUpdateInfoReceived event is triggered correctly when getFirmwareUpdateInfo is successful and the environment is other than PROD[eg:VBN] with /opt/swupdate.conf is present and contains URL to override
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, OnFirmwareUpdateInfoReceived_WhenEnvNotProdWithConfFileOverrideUrl)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    const string  swUpdateFile = _T("/opt/swupdate.conf");
    const uint8_t swUpdateUrl[] = "test_url";
    Core::File file2(swUpdateFile);
    file2.Destroy();
    file2.Create();
    file2.Write(swUpdateUrl, sizeof(swUpdateUrl));

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{\"firmwareVersion\":\"1234\"}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
      EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"{\\\"firmwareVersion\\\":\\\"1234\\\"}\",\"rebootImmediately\":null,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :    onFirmwareUpdateInfoReceived event triggered when getFirmwareUpdateInfo is successful with missing HTTP status code
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WithoutHttpStatusCode)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    fileVer.close();

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":\\{\"status\":0,\"responseString\":\"\",\"rebootImmediately\":false,\"firmwareUpdateVersion\":\"\",\"updateAvailable\":false,\"updateAvailableEnum\":2,\"success\":false\\}}")));
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 * @brief :    onFirmwareUpdateInfoReceived event triggered when getFirmwareUpdateInfo is successful with Repsonse string is empty
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenResponseEmpty)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_PROD_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"\",\"rebootImmediately\":false,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/*
 * @brief: onFirmwareUpdateInfoReceived event triggered when there is an error parsing the response from getFirmwareUpdateInfo.
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenErrorInParsingResponse)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_PROD_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "firmwareVersion:1234";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"firmwareVersion:1234\",\"rebootImmediately\":false,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/**
 *  @brief: onFirmwareUpdateInfoReceived event triggered when the ixconf response is not valid json and/or doesn't contain firmwareVersion
 *  @param[in]   :  This method takes no parameters.
 *  @return      :  {\"asyncResponse\":true,\"success\":true}
 */
TEST_F(SystemServicesEventTest, onFirmwareUpdateInfoReceived_WhenInvalidResponse)
{
    Core::Event onFirmwareUpdateInfoReceived(false, true);
    std::ofstream fileVer("/version.txt");
    fileVer << "imagename:PX051AEI_PROD_2203_sprint_20220331225312sdy_NG";
    fileVer.close();
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string("cat /tmp/xconf_httpcode_thunder.txt")) {
                      const char http_code_str[] = "460";
                      strncpy(buffer, http_code_str, sizeof(buffer) - 1);
                  } else if (string(command) == string("cat /tmp/xconf_response_thunder.txt")) {
                      const char response_str[] = "{}";
                      strncpy(buffer, response_str, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.System.onFirmwareUpdateInfoReceived\",\"params\":{\"status\":0,\"responseString\":\"\",\"rebootImmediately\":false,\"updateAvailable\":false,\"updateAvailableEnum\":3,\"success\":true}}");
          onFirmwareUpdateInfoReceived.SetEvent();
          return Core::ERROR_NONE;
          }));
    handler.Subscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateInfo"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"asyncResponse\":true,\"success\":true}"));
    EXPECT_EQ(Core::ERROR_NONE, onFirmwareUpdateInfoReceived.Lock());
    handler.Unsubscribe(0, _T("onFirmwareUpdateInfoReceived"), _T("org.rdk.System"), message);
	
	// Clear file contents
    fileVer.open("/version.txt", std::ofstream::out | std::ofstream::trunc);
    fileVer.close();
}

/****************************************************************************************************
 * Test functions for :clearLastDeepSleepReason
 * clearLastDeepSleepReason :
 *                clear the last deep sleep reason by removing the file that stores it.
 *                This method takes no parameters.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :1
 *                @Failure :2
 ***************************************************************************************************/

/**
 * @brief : clearLastDeepSleepReason when file doesn't exists/failed to remove.
 *          Check if  If the file for the last deep sleep reason cannot be found or failed to remove,
 *          then  clearLastDeepSleepReason shall be failed and an error message is returned in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":7,"errorMessage":"Unexpected error","success":false}
 */
TEST_F(SystemServicesTest, clearLastDeepSleepReasonFailed_WhenFileFailedToRemove)
{
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("rm -f /opt/standbyReason.txt")));
                return nullptr;
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLastDeepSleepReason"), _T("{}"), response));
}

/**
 * @brief : clearLastDeepSleepReason when pclose fails to close the opened pipe.
 *          Check if the pclose failed to close the opened pipe,
 *          then clearLastDeepSleepReason shall be failed and
 *          an error message is returned in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":7,"errorMessage":"Unexpected error","success":false}
 */
TEST_F(SystemServicesTest, clearLastDeepSleepReasonFailed_WhenPcloseFailed)
{
    //popen mock returns a fake file pointer value.
    FILE* fakePipe = reinterpret_cast<FILE*>(0x1234);
    EXPECT_CALL(wrapsImplMock, popen(testing::_, testing::_))
        .WillOnce(testing::Return(fakePipe));
    //pclose mock returns -1 to simulate a failure to close the file
    EXPECT_CALL(wrapsImplMock, pclose(fakePipe))
        .WillOnce(testing::Return(-1));

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLastDeepSleepReason"), _T("{}"), response));
}

/**
 * @brief : clearLastDeepSleepReason when file successfully removed.
 *          Check if the file for the last deep sleep reason is successfully removed using popen,
 *          then clearLastDeepSleepReason shall be succeeded.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"success": true}
 */
TEST_F(SystemServicesTest, clearLastDeepSleepReasonSuccess_whenFileSuccessfullyRemoved)
{
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command, const char* type) {
                EXPECT_EQ(string(command), string(_T("rm -f /opt/standbyReason.txt")));
                return __real_popen(command, type);
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearLastDeepSleepReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

/*Test cases for clearLastDeepSleepReason ends here*/

/*********************************************************************************************************
 * Test function for :getXconfParams
 * getXconfParams :
 *                Returns XCONF configuration parameters for the device.
 *
 *                @return XconfParams, whether the request succeeded.
 * Use case coverage:
 *                @Success :7
 ******************************************************************************************************/
/**
 * @brief : getXconfParams when stb version not found.
 *        Check if stb version is not found in VERSION_FILE
 *        then getXconfParams shall be succeeded and returns XconfParams with value env = "dev"
 *        and value of firmwareVersion= "unknown" if STB_VERSION_STRING not defined,
 *        and value of firmwareVersion = value defined in STB_VERSION_STRING if STB_VERSION_STRING is defined.
 *
 * @param[in]   :  This method takes no parameters.
 * @return[if STB_VERSION_STRING defined] : {\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"\",\"model\":\"\",\"firmwareVersion\":\"string[STB_VERSION_STRING]\"},\"success\":true}
 * @return[if STB_VERSION_STRING not defined] : {\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"\",\"model\":\"\",\"firmwareVersion\":\"unknown\"},\"success\":true}
 */
#ifdef STB_VERSION_STRING
TEST_F(SystemServicesTest, getXconfParamsSuccess_whenStbVersionNotFound_withVersionDefined)
{
    //stb version information
    //Simulated as version not provided in the vesion.txt file
    ofstream file("/version.txt");
    file << "";
    file.close();
    
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();
    
    string firmwareVersion = STB_VERSION_STRING;
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\""+firmwareVersion+"\"},\"success\":true}"));
}

#else
TEST_F(SystemServicesTest, getXconfParamsSuccess_whenStbVersionNotFoundwith_VersionNotDefined)
{
    //stb version information
    //Simulated as version not provided in the vesion.txt file
    ofstream file("/version.txt");
    file << "";
    file.close();
    
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"unknown\"},\"success\":true}"));
}
#endif

/**
 * @brief : getXconfParams when STB version string does not contain environment information like VBN/PROD/QA.
 *        Check if stb version string does not contain env information,
 *        then getXconfParams shall be succeeded and returns xconfParams with env="dev" and firmwareVersion=stb version string .
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_2203_sprint_20220331225312sdy_NG\"},\"success\":true}
 */
TEST_F(SystemServicesTest, getXconfParamsSuccess_whenVersionStringWithoutEnvInfo)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_2203_sprint_20220331225312sdy_NG";
    file.close();

    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"dev\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"));
}

/**
 * @brief : getXconfParams getting all  XCONF configuration parameters
 *        check if all the configiration parameters are provided in respective files,
 *        then getXconfParams shall be Succeeded and all the values for xconfParams shall be populated.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  "{\"xconfParams\":{\"env\":\"vbn\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"AX061AEI\",\"firmwareVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"
 */
TEST_F(SystemServicesTest, getXconfParamsSuccess_withAllXConfparams)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    file.close();
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();

    //model information
    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
           [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("PATH=${PATH}:/sbin:/usr/sbin /lib/rdk/getDeviceDetails.sh read")));
                std::string filename = "getDeviceDetail.sh";
                std::ofstream file_stream(filename);
                file_stream << "model=AX061AEI\n";
                file_stream.close();
                FILE *fp = __real_popen(("cat " + filename).c_str(), "r");
                return fp;
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"vbn\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"AX061AEI\",\"firmwareVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"));
}
/**
 * @brief : getXconfParams when firm value is "VBN".
 *        Check if the firm value is "VBN"
 *        then getXconfParams shall be succeeded and returns xconfParam with env="vbn".
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"xconfParams\":{\"env\":\"vbn\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\"},\"success\":true}
 */
TEST_F(SystemServicesTest, getXconfParamsSuccess_onFirmvalueVBN)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG";
    file.close();
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"vbn\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"));
}

/**
 * @brief : getXconfParams when firm value is "PROD".
 *        Check if the firm value is "PROD"
 *        then getXconfParams shall be succeeded and returns xconfParam with env="prod".
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"xconfParams\":{\"env\":\"prod\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_PROD_2203_sprint_20220331225312sdy_NG\"},\"success\":true}
 */
TEST_F(SystemServicesTest, getXconfParamsSuccess_onFirmvaluePROD)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_PROD_2203_sprint_20220331225312sdy_NG";
    file.close();
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"prod\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_PROD_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"));
}

/**
 * @brief : getXconfParams when firm value is "QA".
 *        Check if the firm value is "QA"
 *        then getXconfParams shall be succeeded and returns xconfParam with env="qa".
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {\"xconfParams\":{\"env\":\"qa\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_QA_2203_sprint_20220331225312sdy_NG\"},\"success\":true}
 */
TEST_F(SystemServicesTest, getXconfParamsSuccess_onFirmvalueQA)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_QA_2203_sprint_20220331225312sdy_NG";
    file.close();
    //estb_mac information
    file.open("/tmp/.estb_mac");
    file << "D4:52:EE:32:A3:B0";
    file.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"xconfParams\":{\"env\":\"qa\",\"eStbMac\":\"D4:52:EE:32:A3:B0\",\"model\":\"ERROR\",\"firmwareVersion\":\"PX051AEI_QA_2203_sprint_20220331225312sdy_NG\"},\"success\":true}"));
}

/*Test cases for getXconfParams ends here*/

/*****************************************************************************************************
 * Test function for :getMfgSerialNumber
 * getMfgSerialNumber :
 *                Gets the Manufacturing Serial Number.
 *
 *                @return Manufacturing Serial Number, whether the request succeeded.
 * Use case coverage:
 *                @Success :2
 *                @Failure :1
 ******************************************************************************************************/
/**
 * @brief : getMfgSerialNumber when Bus call for retrieving the serial number failed.
 *        Check if BUS call to retrieve the manufacturing serial number is failed,
 *        then getMfgSerialNumber shall be failed and returns an error message in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"success":false}")
 */
TEST_F(SystemServicesTest, getMfgSerialNumberFailed_whenBusCallFailed)
{

    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                //setting up a mock that always returns an error code.
                return IARM_RESULT_IPCCORE_FAIL;
            });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getMfgSerialNumber"), _T("{}"), response));
}

/**
 * @brief : getMfgSerialNumber when Bus call for retrieving the serial number succeeded.
 *        Check if BUS call to retrieve the manufacturing serial number succeeded,
 *        then getMfgSerialNumber shall be succeeded and
 *        returns retrieved manufacture serial number in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"mfgSerialNumber": "F00020CE000003","success": true}
 */
TEST_F(SystemServicesTest, getMfgSerialNumberSuccess_whenBusCallSuccess)
{
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "F00020CE000003";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, param->bufLen);
                param->type =  mfrSERIALIZED_TYPE_MANUFACTURING_SERIALNUMBER;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMfgSerialNumber"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mfgSerialNumber\":\"F00020CE000003\",\"success\":true}"));
}

/**
 * @brief : getMfgSerialNumber when cached value available.
 *        Check if cached data of  mfg serial number is available
 *        then , getMfgSerialNumber shall successfully retrieves the cached information and returns it.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  "result": {"mfgSerialNumber": "F00020CE000003","success": true}
 */
TEST_F(SystemServicesTest, getMfgSerialNumberSuccess_getCachedMfgSerialNumber)
{
    //Below IARM_Bus_Call function is called for saving the retrieved data
    //in member variables [cached value]
    ON_CALL(iarmBusImplMock, IARM_Bus_Call)
        .WillByDefault(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_EQ(string(ownerName), string(_T(IARM_BUS_MFRLIB_NAME)));
                EXPECT_EQ(string(methodName), string(_T(IARM_BUS_MFRLIB_API_GetSerializedData)));
                auto* param = static_cast<IARM_Bus_MFRLib_GetSerializedData_Param_t*>(arg);
                const char* str = "F00020CE000003";
                param->bufLen = strlen(str);
                strncpy(param->buffer, str, param->bufLen);
                param->type =  mfrSERIALIZED_TYPE_MANUFACTURING_SERIALNUMBER;
                return IARM_RESULT_SUCCESS;
            });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMfgSerialNumber"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mfgSerialNumber\":\"F00020CE000003\",\"success\":true}"));
    //To confirm that the retrieved data is cached Data;
    //sets an expectation that the IARM_Bus_Call function should not be called during this sequence
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Call)
       .Times(0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMfgSerialNumber"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mfgSerialNumber\":\"F00020CE000003\",\"success\":true}"));
}
/*Test cases for getMfgSerialNumber ends here*/

/*************************************************************************************************************
 * Test function for :getSerialNumber
 * getSerialNumber :
 *                Returns the device serial number.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :2
 *                @Failure :3
 ************************************************************************************************************/

#ifdef USE_TR_69
/**
 * @brief : getSerialNumber when GetRFCParameter return failure
 *        Check if GetRFCParameter returns other than Success,
 *        then getSerialNumber shall be failed and return error message in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":7,"errorMessage":"Unexpected error","success":false}
 */
TEST_F(SystemServicesTest, getSerialNumberTR069Failed_OnGetRFCParameterFailed)
{
    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                return WDMP_FAILURE;
            }));
     EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
}

/**
 * @brief : getSerialNumber when  GetRFCParameter return Success
 *        Check if GetRFCParameter returns Success,
 *        then  getSerialNumber shall be Succeeded and
 *        returns the retrieved serial number in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"serialNumber":"32E10400103240447","success":true}
 */
TEST_F(SystemServicesTest, getSerialNumberTR069Success_OnGetRFCParameterSuccess)
{
    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                 // Define the expected serial number
                 const char* expectedSerialNumber = "32E10400103240447";
                 strncpy(pstParamData->value, expectedSerialNumber, sizeof(pstParamData->value));
                 return WDMP_SUCCESS;
            }));
     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
     EXPECT_THAT(response, string("{\"serialNumber\":\"32E10400103240447\",\"success\":true}"));
}

#else
/**
 * @brief : getSerialNumber when getStateDetails.sh does not exist
 *        Check if /lib/rdk/getStateDetails.sh file doesn't exist,
 *        then getSerialNumber shall be failed and return the error message in response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":4,"errorMessage":"Unexpected Error","success":false}
 */
TEST_F(SystemServicesTest, getSerialNumberSnmpFailed_WhenScriptFileNotExist)
{
    const string deviceStateInfoScript = _T("/lib/rdk/getStateDetails.sh");
    Core::File file(deviceStateInfoScript);
    file.Create();
    // Remove the file to simulate it is  not-existing
    file.Destroy();
    EXPECT_FALSE(Core::File(string(_T("/lib/rdk/getStateDetails.sh"))).Exists());
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
}

/**
 * @brief : getSerialNumber when TMP_SERIAL_NUMBER_FILE does not exist.
 *        Check if TMP_SERIAL_NUMBER_FILE doesn't exist,then getSerialNumber shall be failed and
 *        return the error message in response
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":4,"errorMessage":"Expected file not found","success":false}
 */
TEST_F(SystemServicesTest, getSerialNumberSnmpFailed_WhenTmpSerialNumberFileNotExist)
{
    const string deviceStateInfoScript = _T("/lib/rdk/getStateDetails.sh");
    Core::File file(deviceStateInfoScript);
    file.Create();
    
    ofstream file2("/tmp/.STB_SER_NO");
    file2 << "32E10400103240447";
    file2.close();
    // Remove the file to simulate it is  not-existing
    std::remove("/tmp/.STB_SER_NO");

    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/getStateDetails.sh"))).Exists());
    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/getStateDetails.sh STB_SER_NO")));
                return 0;
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
}


/**
 * @brief : getSerialNumber when serial number successfully read from  /tmp/.STB_SER_NO .
 *        Check if file /lib/rdk/getStateDetails.sh and TMP_SERIAL_NUMBER_FILE file[/tmp/.STB_SER_NO] exist,
 *        then getSerialNumber shall be succeeded and returns serial number in response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"serialNumber":"32E10400103240447","success":true}
 */
TEST_F(SystemServicesTest, getSerialNumberSnmpSuccess_whenSerialNumberIsInTmpFile)
{
    const string deviceStateInfoScript = _T("/lib/rdk/getStateDetails.sh");
    Core::File file(deviceStateInfoScript);
    file.Create();
    ofstream file2("/tmp/.STB_SER_NO");
    file2 << "32E10400103240447";
    file2.close();

    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/getStateDetails.sh"))).Exists());
    EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/getStateDetails.sh STB_SER_NO")));
                return 0;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
    EXPECT_THAT(response, string("{\"serialNumber\":\"32E10400103240447\",\"success\":true}"));
}
#endif

/*Test cases for getSerialNumber ends here*/

/*************************************************************************************************************
 * Test function for :onSystemModeChanged
 * onSystemModeChanged :
 *                Triggered when the device operating mode changes.
 *
 *                @return Whether the mode change is succeeded.
 * Use case coverage:
 *                @Success :1
 *                @Failure :0
 ************************************************************************************************************/

/**
 * @brief : Check when the system mode is changed from IARM,
 *        the onSystemModeChanged event is triggered with the expected JSON string containing the new mode.
 *
 * @param[in]   :  This method takes mode as parameter.
 */
TEST_F(SystemServicesEventIarmTest, onSystemModeChanged)
{
    Core::Event onSystemModeChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onSystemModeChanged\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"mode\":\"NORMAL\""
                                                             "\\}"
                                                             "\\}")));

                onSystemModeChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onSystemModeChanged"), _T("org.rdk.System"), message);

    IARM_Bus_CommonAPI_SysModeChange_Param_t param;
    param.newMode = IARM_BUS_SYS_MODE_NORMAL;
    SysModeChange(&param);

    EXPECT_EQ(Core::ERROR_NONE, onSystemModeChanged.Lock());

    handler.Unsubscribe(0, _T("onSystemModeChanged"), _T("org.rdk.System"), message);
}
/*Test cases for onSystemModeChanged ends here*/

/********************************************************************************************************
 * Test function for :getPlatformConfiguration
 * getPlatformConfiguration :
 *                Returns the supported features and device/account info.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :8
 *                @Failure :1
*********************************************************************************************************/

/**
 * @brief : getPlatformConfiguration when called with Invalid Query Parameter
 *         Check if getPlatformConfiguration api called with Invalid query then getPlatformConfiguration
 *          shall be failed and the PlatformCaps object shall NOT be populated with information
 *
 * @param[in]   :  This method takes query parameters other than "accountInfo"/"deviceInfo" which considered as Invalid
 * @return      :  Returns the response string as success:false.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationFailed_withBadQuery)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"InvalidQueryParam\"}"), response));
   EXPECT_EQ(response, string("{\"success\":false}"));
}
/**
 * @brief : getPlatformConfiguration when called with empty Query Parameter
 *         Check if getPlatformConfiguration api called with empty query then getPlatformConfiguration
 *          shall be succeeded  and the PlatformCaps object shall be populated with information
 *          about both the account and device.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  Returns a PlatformCaps object containing the retrieved configuration information.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_withEmptyQuery)
{
    NiceMock<ServiceMock> service;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    DispatcherMock* dispatcher = new DispatcherMock();
    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));

    bool firmwareUpdateDisabled = false;
    if (Core::File(string("/opt/swupdate.conf")).Exists()) {
        firmwareUpdateDisabled = true;
    }

    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();
    Core::JSONRPC::Message resp;

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    if (message.Designator == "org.rdk.AuthService.1.getAlternateIds") {
                    resp.Result = Core::JSON::String("{\"alternateIds\":{\"_xbo_account_id\":\"1234567890\"}}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getXDeviceId") {
                    resp.Result = Core::JSON::String("{\"xDeviceId\":\"1000000000000000000\"}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getExperience") {
                    resp.Result = Core::JSON::String("{\"experience\":\"test_experience_string\"}");
                      }
                    if (message.Designator == "org.rdk.Network.1.getPublicIP") {
                    resp.Result = Core::JSON::String("{\"public_ip\":\"test_publicIp_string\"}");
                      }
                    mockResponse->Result = resp.Result;
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
    .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\"}"), response));
    EXPECT_EQ(response, string( "{\"AccountInfo\":{\"accountId\":\"1234567890\",\"x1DeviceId\":\"1000000000000000000\",\"XCALSessionTokenAvailable\":true,\"experience\":\"test_experience_string\",\"deviceMACAddress\":\"null\",\"firmwareUpdateDisabled\":" + std::string(firmwareUpdateDisabled ? "true" : "false") + "},\"DeviceInfo\":{\"quirks\":[\"XRE-4621\",\"XRE-4826\",\"XRE-4896\",\"XRE-5553\",\"XRE-5743\",\"XRE-6350\",\"XRE-6827\",\"XRE-7267\",\"XRE-7366\",\"XRE-7415\",\"XRE-7389\",\"DELIA-6142\",\"RDK-2222\",\"XRE-7924\",\"DELIA-8978\",\"XRE-7711\",\"RDK-2849\",\"DELIA-9338\",\"ZYN-172\",\"XRE-8970\",\"XRE-9001\",\"DELIA-17939\",\"DELIA-17204\",\"CPC-1594\",\"DELIA-21775\",\"XRE-11602\",\"CPC-1767\",\"CPC-1824\",\"XRE-10057\",\"RDK-21197\",\"CPC-2004\",\"DELIA-27583\",\"XRE-12919\",\"DELIA-28101\",\"XRE-13590\",\"XRE-13692\",\"XRE-13722\",\"DELIA-30269\",\"RDK-22801\",\"CPC-2404\",\"XRE-14664\",\"XRE-14921\",\"XRE-14963\",\"RDK-26425\",\"RDK-28990\",\"RDK-32261\"],\"mimeTypeExclusions\":{\"CDVR\":[\"application\\/dash+xml\"],\"DVR\":[\"application\\/dash+xml\"],\"EAS\":[\"application\\/dash+xml\"],\"IPDVR\":[\"application\\/dash+xml\"],\"IVOD\":[\"application\\/dash+xml\"],\"LINEAR_TV\":[\"application\\/dash+xml\"],\"VOD\":[\"application\\/dash+xml\"]},\"features\":{\"allowSelfSignedWithIPAddress\":1,\"connection.supportsSecure\":1,\"htmlview.callJavaScriptWithResult\":1,\"htmlview.cookies\":1,\"htmlview.disableCSSAnimations\":1,\"htmlview.evaluateJavaScript\":1,\"htmlview.headers\":1,\"htmlview.httpCookies\":1,\"htmlview.postMessage\":1,\"htmlview.urlpatterns\":1,\"keySource\":1,\"uhd_4k_decode\":0},\"model\":\"null\",\"deviceType\":\"\",\"supportsTrueSD\":true,\"webBrowser\":{\"browserType\":\"WPE\",\"version\":\"1.0.0.0\",\"userAgent\":\"Mozilla\\/5.0 (Linux; x86_64 GNU\\/Linux) AppleWebKit\\/601.1 (KHTML, like Gecko) Version\\/8.0 Safari\\/601.1 WPE\"},\"HdrCapability\":\"\",\"canMixPCMWithSurround\":false,\"publicIP\":\"test_publicIp_string\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);
}

/**
 * @brief : getPlatformConfiguration when called with  Query Parameter as AccountInfo
 *         Check if getPlatformConfiguration api called with query :AccountInfo then getPlatformConfiguration
 *          shall be succeeded  and the PlatformCaps object shall be populated with Account information
 *
 * @param[in]   :  query:"AccountInfo"
 * @return      :  Returns a PlatformCaps object containing the retrieved configuration information.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_withQueryAccountInfo)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    DispatcherMock* dispatcher = new DispatcherMock();
    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));

    bool firmwareUpdateDisabled = false;
    if (Core::File(string("/opt/swupdate.conf")).Exists()) {
        firmwareUpdateDisabled = true;
    }
    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();
    Core::JSONRPC::Message resp;

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    if (message.Designator == "org.rdk.AuthService.1.getAlternateIds") {
                    resp.Result = Core::JSON::String("{\"alternateIds\":{\"_xbo_account_id\":\"1234567890\"}}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getXDeviceId") {
                    resp.Result = Core::JSON::String("{\"xDeviceId\":\"1000000000000000000\"}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getExperience") {
                    resp.Result = Core::JSON::String("{\"experience\":\"test_experience_string\"}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getSessionToken") {
                    resp.Result = Core::JSON::String("{\"token\":\"12345\"}");
                      }
                    if (message.Designator == "org.rdk.System.1.getDeviceInfo") {
                    resp.Result = Core::JSON::String("{\"estb_mac\":\"test_estb_mac_string\"}");
                      }
                    mockResponse->Result = resp.Result;
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
     .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"AccountInfo\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"accountId\":\"1234567890\",\"x1DeviceId\":\"1000000000000000000\",\"XCALSessionTokenAvailable\":true,\"experience\":\"test_experience_string\",\"deviceMACAddress\":\"test_estb_mac_string\",\"firmwareUpdateDisabled\":" + std::string(firmwareUpdateDisabled ? "true" : "false") + "},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);
}

/**
 * @brief : getPlatformConfiguration when called with  Query Parameter as DeviceInfo
 *         Check if getPlatformConfiguration api called with query :DeviceInfo then getPlatformConfiguration
 *          shall be succeeded  and the PlatformCaps object shall be populated with Device information
 *
 * @param[in]   :  query:"DeviceInfo"
 * @return      :  Returns a PlatformCaps object containing the retrieved configuration information.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_withQueryDeviceInfo)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    DispatcherMock* dispatcher = new DispatcherMock();
    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));


    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();
    Core::JSONRPC::Message resp;

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    if (message.Designator == "org.rdk.System.1.getDeviceInfo") {
                    resp.Result = Core::JSON::String("{\"model_number\":\"PX051AEI\"}");
                      }
                    if (message.Designator == "org.rdk.AuthService.1.getDeviceInfo") {
                    //Hex value for Json Resp -> "deviceInfo": "deviceType=IpStb",
                    resp.Result = Core::JSON::String("{\"deviceInfo\":\"646576696365547970653d49705374622c20646576696365547970653d4969505374622c20766f69636549643d312c206d616e7566616374757265723d496e74656c\"}");
                      }
                    if (message.Designator == "org.rdk.Network.1.getPublicIP") {
                    resp.Result = Core::JSON::String("{\"public_ip\":\"test_publicIp_string\"}");
                      }
                    mockResponse->Result = resp.Result;
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
     .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"DeviceInfo\"}"), response));
    EXPECT_EQ(response, string("{\"DeviceInfo\":{\"quirks\":[\"XRE-4621\",\"XRE-4826\",\"XRE-4896\",\"XRE-5553\",\"XRE-5743\",\"XRE-6350\",\"XRE-6827\",\"XRE-7267\",\"XRE-7366\",\"XRE-7415\",\"XRE-7389\",\"DELIA-6142\",\"RDK-2222\",\"XRE-7924\",\"DELIA-8978\",\"XRE-7711\",\"RDK-2849\",\"DELIA-9338\",\"ZYN-172\",\"XRE-8970\",\"XRE-9001\",\"DELIA-17939\",\"DELIA-17204\",\"CPC-1594\",\"DELIA-21775\",\"XRE-11602\",\"CPC-1767\",\"CPC-1824\",\"XRE-10057\",\"RDK-21197\",\"CPC-2004\",\"DELIA-27583\",\"XRE-12919\",\"DELIA-28101\",\"XRE-13590\",\"XRE-13692\",\"XRE-13722\",\"DELIA-30269\",\"RDK-22801\",\"CPC-2404\",\"XRE-14664\",\"XRE-14921\",\"XRE-14963\",\"RDK-26425\",\"RDK-28990\",\"RDK-32261\"],\"mimeTypeExclusions\":{\"CDVR\":[\"application\\/dash+xml\"],\"DVR\":[\"application\\/dash+xml\"],\"EAS\":[\"application\\/dash+xml\"],\"IPDVR\":[\"application\\/dash+xml\"],\"IVOD\":[\"application\\/dash+xml\"],\"LINEAR_TV\":[\"application\\/dash+xml\"],\"VOD\":[\"application\\/dash+xml\"]},\"features\":{\"allowSelfSignedWithIPAddress\":1,\"connection.supportsSecure\":1,\"htmlview.callJavaScriptWithResult\":1,\"htmlview.cookies\":1,\"htmlview.disableCSSAnimations\":1,\"htmlview.evaluateJavaScript\":1,\"htmlview.headers\":1,\"htmlview.httpCookies\":1,\"htmlview.postMessage\":1,\"htmlview.urlpatterns\":1,\"keySource\":1,\"uhd_4k_decode\":0},\"model\":\"PX051AEI\",\"deviceType\":\"IpStb\",\"supportsTrueSD\":true,\"webBrowser\":{\"browserType\":\"WPE\",\"version\":\"1.0.0.0\",\"userAgent\":\"Mozilla\\/5.0 (Linux; x86_64 GNU\\/Linux) AppleWebKit\\/601.1 (KHTML, like Gecko) Version\\/8.0 Safari\\/601.1 WPE\"},\"HdrCapability\":\"\",\"canMixPCMWithSurround\":false,\"publicIP\":\"test_publicIp_string\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);
}

/**
 * @brief : getPlatformConfiguration when called with a specific DeviceInfo /AccountInfo Query Parameter
 *         Check if getPlatformConfiguration api called with passing any specified configuration parameter
 *         of DeviceInfo /AccountInfo; then getPlatformConfiguration shall be succeeded
 *         and the PlatformCaps object shall be populated with the value for that
 *         specfic configuration parameter
 *
 * @param[in]   :  This method takes  any specific configuration parameter of AccountInfo/DeviceInfo
 * @return      :  Returns a PlatformCaps object containing the requested configuration information.
 */

TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_withQueryParameterValue)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    DispatcherMock* dispatcher = new DispatcherMock();
    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));


    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();
    Core::JSONRPC::Message resp;

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    if (message.Designator == "org.rdk.AuthService.1.getXDeviceId") {
                    resp.Result = Core::JSON::String("{\"xDeviceId\":\"1000000000000000000\"}");
                      }
                    mockResponse->Result = resp.Result;
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
     .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"AccountInfo.x1DeviceId\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"x1DeviceId\":\"1000000000000000000\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);

}


 /* @brief : getPlatformConfiguration when the dispatcher object is null.
 *         Check if getPlatformConfiguration api called and if dispatcher object is null;
 *         then getPlatformConfiguration shall be succeeded .
 *         But PlatformCaps object shall be populated with Null/empty
 *
 * @param[in]   :  This method takes parameter :AccountInfo/DeviceInfo/Empty param
 * @return      :  Returns PlatformCaps object with empty/Null value.
 */

TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_whenDispatcherNull)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    DispatcherMock* dispatcher = new DispatcherMock();

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
      .Times(0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"AccountInfo.x1DeviceId\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"x1DeviceId\":\"null\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
}

 /* @brief : getPlatformConfiguration when pass invalid callsign.
 *         Check if getPlatformConfiguration api called with invalid callsign;
 *         then getPlatformConfiguration shall be succeeded .
 *         But PlatformCaps object shall be populated with Null/empty
 *
 * @param[in]   :  This method takes Invalid callsign
 * @return      :  Returns PlatformCaps object with empty/Null value.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_whenInvalidCallsign)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    DispatcherMock* dispatcher = new DispatcherMock();

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
      .Times(0);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"auth\",\"query\":\"AccountInfo.x1DeviceId\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"x1DeviceId\":\"null\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);

}
 /* @brief : getPlatformConfiguration when the response JSON RPC message is unable to parse.
 *         Check if getPlatformConfiguration api called and if the JSON RPC response message is failed to parse *         then getPlatformConfiguration shall be succeeded .
 *         But PlatformCaps object shall be populated with Null/empty
 *
 * @param[in]   :  This method takes parameter :AccountInfo/DeviceInfo/Empty param
 * @return      :  Returns PlatformCaps object with empty/Null value.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_whenParseError)
{
    NiceMock<ServiceMock> service;
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    EXPECT_EQ(string(""), plugin->Initialize(&service));
    DispatcherMock* dispatcher = new DispatcherMock();

    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));


    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();
    Core::JSONRPC::Message resp;

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    if (message.Designator == "org.rdk.AuthService.1.getXDeviceId") {
                    resp.Result = Core::JSON::String("1000000000000000000");
                      }
                    mockResponse->Result = resp.Result;
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
     .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"AccountInfo.x1DeviceId\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"x1DeviceId\":\"null\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);
}

 /* @brief : getPlatformConfiguration when the dispatcher Invoke method returns an error in Response object.
 *         Check if getPlatformConfiguration api called and
 *         if dispatcher Invoke method returns an error in Response object;
 *         then getPlatformConfiguration shall be succeeded .
 *         But PlatformCaps object shall be populated with Null/empty
 *
 * @param[in]   :  This method takes parameter :AccountInfo/DeviceInfo/Empty param
 * @return      :  Returns PlatformCaps object with empty/Null value.
 */
TEST_F(SystemServicesTest, getPlatformConfigurationSuccess_withDispatcherInvokeError)
{
    NiceMock<ServiceMock> service;
    EXPECT_EQ(string(""), plugin->Initialize(&service));
    NiceMock<FactoriesImplementation> factoriesImplementation;
    PluginHost::IFactories::Assign(&factoriesImplementation);

    DispatcherMock* dispatcher = new DispatcherMock();
    EXPECT_CALL(service, QueryInterfaceByCallsign(::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const uint32_t, const string& name) -> void* {
                return (reinterpret_cast<void*>(dispatcher));
            }));


    Core::ProxyType<Core::JSONRPC::Message> mockResponse = Core::ProxyType<Core::JSONRPC::Message>::Create();

    EXPECT_CALL(*dispatcher, Invoke(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(::testing::Invoke(
            [&](const std::string&,
                uint32_t,
                const Core::JSONRPC::Message& message) ->Core::ProxyType<Core::JSONRPC::Message> {
                    mockResponse->Error.SetError(3);
                    return mockResponse;
                }));
    EXPECT_CALL(*dispatcher, Release())
     .Times(::testing::AnyNumber());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPlatformConfiguration"), _T("{\"callsign\":\"authService\",\"query\":\"AccountInfo.x1DeviceId\"}"), response));
    EXPECT_EQ(response, string("{\"AccountInfo\":{\"x1DeviceId\":\"null\"},\"success\":true}"));

    delete dispatcher;
    plugin->Deinitialize(&service);
    PluginHost::IFactories::Assign(nullptr);
}
/*Test cases for getPlatformConfiguration ends here*/

/********************************************************************************************************
 * Test function for :uploadLogs
 * uploadLogs:
 *                Uploads logs to a URL returned by SSR.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :2
 *                @Failure :3
*********************************************************************************************************/

/**
 * @brief : uploadLogs when  GetFilename Failed
 *         Check if an error retrieving MAC address while generating Filename then uploadLogs
 *          shall be failed and returns FilenameFail error
 *
 * @param[in]   :  None
 * @return      :  Returns the error message: "can't generate logs filename" and response string as success:false.
 */

TEST_F(SystemServicesTest, uploadLogFailed_whenGetFilenameFailed)
{
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string(". /lib/rdk/utils.sh && getMacAddressOnly")) {
                   // Simulate Utils::cRunScript failure by not setting the buffer value
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("uploadLogs"), _T("{}"), response));
}

/**
 * @brief : uploadLogs when  GetFilename Failed with invalid input URL
 *         Check if the input request contains an invalid URL; then uploadLogs
 *         shall be failed and returns BadUrl error
 *
 * @param[in]   :  URL NOT starting with "https"
 * @return      :  Returns the error message: "invalid or insecure input url" and response string as success:false.
 */
TEST_F(SystemServicesTest, uploadLogFailed_withBadUrl)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("uploadLogs"), _T("{\"url\": \"http://ssr.ccp.xcal.tv/cgi-bin/rdkb_snmp.cgi\"}"), response));
}

/**
 * @brief : uploadLogs when  GetFilename Failed with invalid input URL
 *         Check if there is a failure in archiving the logs; then uploadLogs
 *         shall be failed and returns TarFail error
 *
 * @param[in]   :  None
 * @return      :  Returns the error message: "tar fail" and response string as success:false.
 */
TEST_F(SystemServicesTest, uploadLogFailed_whenArchieveLogsFailed)
{
    const string logArchievedPath = _T("/tmp/test_mac_Logs_" + currentDateTimeUtc("%m-%d-%y-%I-%M%p") + ".tgz");
    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string(". /lib/rdk/utils.sh && getMacAddressOnly")) {
                      const char mac_Addr[] = "test_mac";
                      strncpy(buffer, mac_Addr, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
    EXPECT_FALSE(Core::File(string(_T(logArchievedPath))).Exists());

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("uploadLogs"), _T("{}"), response));
}

/**
 * @brief : uploadLogs with valid input URL
 *         Check if a valid URL is provided in the input parameters JsonObject;
 *         then uploadLogs shall be suceeded with the correct URL.
 *
 * @param[in]   :  Valid input URL
 * @return      :  Returns response string as success:true.
 */
TEST_F(SystemServicesTest, uploadLogSuccess_withValidURL)
{
    const string logArchievedPath = _T("/tmp/test_mac_Logs_" + currentDateTimeUtc("%m-%d-%y-%I-%M%p") + ".tgz");
    Core::File file(logArchievedPath);
    file.Create();

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string(". /lib/rdk/utils.sh && getMacAddressOnly")) {
                      const char mac_Addr[] = "test_mac";
                      strncpy(buffer, mac_Addr, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
    EXPECT_TRUE(Core::File(string(_T(logArchievedPath))).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogs"), _T("{\"url\": \"https://ssr.ccp.xcal.tv/cgi-bin/rdkb_snmp.cgi\"}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

/**
 * @brief : uploadLogs without Any input URL
 *         Check if a No URL is provided in the input parameters JsonObject;
 *         then uploadLogs shall be suceeded with the default URL.
 *
 * @param[in]   :  None
 * @return      :  Returns response string as success:true.
 */
TEST_F(SystemServicesTest, uploadLogSuccess_WithDefaultURL)
{
    const string logArchievedPath = _T("/tmp/test_mac_Logs_" + currentDateTimeUtc("%m-%d-%y-%I-%M%p") + ".tgz");
    Core::File file(logArchievedPath);
    file.Create();

    EXPECT_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
          .Times(::testing::AnyNumber())
          .WillRepeatedly(::testing::Invoke(
              [&](const char* command, const char* type) {
                      char buffer[1024];
                      memset(buffer, 0, sizeof(buffer));
                  if (string(command) == string(". /lib/rdk/utils.sh && getMacAddressOnly")) {
                      const char mac_Addr[] = "test_mac";
                      strncpy(buffer, mac_Addr, sizeof(buffer) - 1);
                  }
                 FILE* pipe = fmemopen(buffer, strlen(buffer), "r");
                 return pipe;
              }));
    EXPECT_TRUE(Core::File(string(_T(logArchievedPath))).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogs"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}
/*Test cases for uploadLogs ends here*/

 /***********************************************************************************************************
 * Test function for :uploadLogsAsync
 * uploadLogsAsync :
 *                  Starts background process to upload logs.
 *                  @returns Whether the request succeeded
 * Event onLogUpload: Triggered when logs upload process is done
 * Use case coverage:
 *                @Success :2
 *                @Failure :3
 ********************************************************************************************************/

/**
 * @brief : uploadLogsAsync  when uploadSTBLogs.sh not present.
 *          Checks if uploadSTBLogs.sh is not present in /lib/rdk, then uploadLogsAsync
 *          should return a response status as true but fail to start the log upload process.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, uploadLogsAsyncFailed_WhenUploadLogFileNotExist)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

/**
 * @brief : uploadLogsAsync  without BUILD_TYPE information.
 *          Check if device.properties does not contain BUILD_TYPE information then,
 *          then uploadLogsAsync should return a response status as true but fail to start the log upload process.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, uploadLogsAsyncFailed_withoutBuildType)
{
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();

    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties.close();

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

/**
 * @brief : uploadLogsAsync  when dcm property file not exist.
 *          Checks if the DCM property file is not present in the /opt or /etc folder,
 *          then uploadLogsAsync should return a response status as true but fail
 *          to start the log upload process since it could not get the LOG information.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, uploadLogsAsyncFailed_WhenDcmFileNotExist)
{
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/uploadSTBLogs.sh"))).Exists());

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));


    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();
    EXPECT_TRUE(Core::File(string(_T("/etc/device.properties"))).Exists());


    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_TRUE(Core::File(string(_T("/tmp/DCMSettings.conf"))).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

/**
 * @brief : uploadLogsAsync  when Build type is PROD.
 *          Checks if the device.properties file contains BUILD_TYPE="prod",
 *          then uploadLogsAsync should retrieve the LOG SERVER information from etc/dcm.properties
 *          and the log upload process should succeed.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, uploadLogsAsyncSuccess_WithBuildTypeProd)
{
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/uploadSTBLogs.sh"))).Exists());

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));


    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=prod\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();
    EXPECT_TRUE(Core::File(string(_T("/etc/device.properties"))).Exists());

    ofstream dcmPropertiesFile("/etc/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER=stblogger.ccp.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER_URL=https://xconf.xcal.tv/loguploader/getSettings\n";
    dcmPropertiesFile << "DCM_SCP_SERVER=stbscp.ccp.xcal.tv\n";
    dcmPropertiesFile << "HTTP_UPLOAD_LINK=https://ssr.ccp.xcal.tv/cgi-bin/S3.cgi\n";
    dcmPropertiesFile << "DCA_UPLOAD_URL=https://stbrtl.r53.xcal.tv\n";
    dcmPropertiesFile.close();
    EXPECT_TRUE(Core::File(string(_T("/etc/dcm.properties"))).Exists());

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_TRUE(Core::File(string(_T("/tmp/DCMSettings.conf"))).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}

/**
 * @brief : uploadLogsAsync  when dcm property exist.
 *          Checks if the dcm.properties file is present,
 *          then uploadLogsAsync should retrieve the LOG SERVER information from dcm.properties
 *          and the log upload process should succeed.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, uploadLogsAsyncSuccess_WhenDcmFileExist)
{
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/uploadSTBLogs.sh"))).Exists());

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));


    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();
    EXPECT_TRUE(Core::File(string(_T("/etc/device.properties"))).Exists());

    ofstream dcmPropertiesFile("/opt/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER=stblogger.ccp.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER_URL=https://xconf.xcal.tv/loguploader/getSettings\n";
    dcmPropertiesFile << "DCM_SCP_SERVER=stbscp.ccp.xcal.tv\n";
    dcmPropertiesFile << "HTTP_UPLOAD_LINK=https://ssr.ccp.xcal.tv/cgi-bin/S3.cgi\n";
    dcmPropertiesFile << "DCA_UPLOAD_URL=https://stbrtl.r53.xcal.tv\n";
    dcmPropertiesFile.close();
    EXPECT_TRUE(Core::File(string(_T("/opt/dcm.properties"))).Exists());

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_TRUE(Core::File(string(_T("/tmp/DCMSettings.conf"))).Exists());

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}
/*Test cases for uploadLogsAsync ends here*/

 /***********************************************************************************************************
 * Test function for :abortLogUpload
 * uploadLogsAsync :
 *                  Stops background process to upload logs.
 *                  @returns Whether the request succeeded
 * Event onLogUpload :Triggered when logs upload process is stopped
 * Use case coverage:
 *                @Success :1
 *                @Failure :1
 ********************************************************************************************************************/

/**
 * @brief : abortLogUploadFailure when UploadLogScript is Not Running.
 *          Checks if the abortLogUpload method is called when the uploadLogScript is not running,
 *          then response status should be false indicating failure.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":false}")
 */
TEST_F(SystemServicesTest, abortLogUploadFailure_whenUploadLogScriptNotRunning)
{
    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command, const char* type) -> FILE* {
                return __real_popen(command, type);
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("abortLogUpload"), _T("{}"), response));
}

/**
 * @brief : abortLogUploadSuccess
 *          Checks if the abortLogUpload method is called successfully and
 *          returns a response status as true.
 *
 * @param[in]   :  no parameter
 * @return      :  "{\"success\":true}")
 */
TEST_F(SystemServicesTest, abortLogUploadSuccess)
{
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();
    EXPECT_TRUE(Core::File(string(_T("/lib/rdk/uploadSTBLogs.sh"))).Exists());

    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));


    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();

    EXPECT_TRUE(Core::File(string(_T("/etc/device.properties"))).Exists());

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_TRUE(Core::File(string(_T("/tmp/DCMSettings.conf"))).Exists());

	//uploadLogsAsync method is invoked first to ensure that m_uploadLogsPid is assigned a value other than -1.
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));

    ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command, const char* type) -> FILE* {
                return __real_popen(command, type);
            }));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("abortLogUpload"), _T("{}"), response));
    EXPECT_EQ(response, "{\"success\":true}");
}
/*Test cases for abortLogUpload ends here*/

/***********************************************************************************************************
 * Test function for Event API :onLogUpload
 * onLogUplaod:
 *                  Triggered when logs upload process is done or stopped.
 *                  @returns Upload status (must be one of the following: UPLOAD_SUCCESS, UPLOAD_FAILURE, UPLOAD_ABORTED)
 * Use case coverage:
 *                @Success :2
 *                @Failure :1
 ********************************************************************************************************************/
/**
 * @brief Test case for onLogUpload with uploadStatusSuccess.
 *
 * Verifies if the onLogUpload event is triggered correctly with log upload status as success.
 *
 * @param None.
 * @return None.
 */
TEST_F(SystemServicesEventIarmTest, onLogUploadSuccess_withUploadStatusSuccess)
{
    Core::Event onLogUpload(false, true);
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();


    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));

    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();

    ofstream dcmPropertiesFile("/opt/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER=stblogger.ccp.xcal.tv\n";
    dcmPropertiesFile << "DCM_LOG_SERVER_URL=https://xconf.xcal.tv/loguploader/getSettings\n";
    dcmPropertiesFile << "DCM_SCP_SERVER=stbscp.ccp.xcal.tv\n";
    dcmPropertiesFile << "HTTP_UPLOAD_LINK=https://ssr.ccp.xcal.tv/cgi-bin/S3.cgi\n";
    dcmPropertiesFile << "DCA_UPLOAD_URL=https://stbrtl.r53.xcal.tv\n";
    dcmPropertiesFile.close();
    EXPECT_TRUE(Core::File(string(_T("/opt/dcm.properties"))).Exists());

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onLogUpload\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"logUploadStatus\":\"UPLOAD_SUCCESS\""
                                                             "\\}"
                                                             "\\}")));

                onLogUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD;
    sysEventData.data.systemStates.state =   IARM_BUS_SYSMGR_LOG_UPLOAD_SUCCESS;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onLogUpload.Lock());

    handler.Unsubscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);
}

/**
 * @brief Test case for onLogUploadSuccess with abortStatusSuccess.
 *
 * Verifies if the onLogUpload event is triggered correctly with log abort status as success.
 *
 * @param None.
 * @return None.
 */
TEST_F(SystemServicesEventIarmTest, onLogUploadSuccess_withAbortStatusSuccess)
{
    Core::Event onLogUpload(false, true);
    const string uploadStbLogFile = _T("/lib/rdk/uploadSTBLogs.sh");
    Core::File file(uploadStbLogFile);
    file.Create();


    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                pstParamData->type = WDMP_BOOLEAN;
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));

    std::ofstream deviceProperties("/etc/device.properties");
    deviceProperties << "BUILD_TYPE=dev\n";
    deviceProperties << "FORCE_MTLS=true\n";
    deviceProperties.close();

    ofstream dcmPropertiesFile("/etc/dcm.properties");
    dcmPropertiesFile << "LOG_SERVER=logs.xcal.tv\n";
    dcmPropertiesFile.close();

    std::ofstream tmpDcmSettings("/tmp/DCMSettings.conf");
    tmpDcmSettings << "LogUploadSettings:UploadRepository:uploadProtocol=https\n";
    tmpDcmSettings << "LogUploadSettings:UploadRepository:URL=https://example.com/upload\n";
    tmpDcmSettings << "LogUploadSettings:UploadOnReboot=true\n";
    tmpDcmSettings.close();
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("uploadLogsAsync"), _T("{}"), response));

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_THAT(text, ::testing::MatchesRegex(_T("\\{"
                                                             "\"jsonrpc\":\"2.0\","
                                                             "\"method\":\"org.rdk.System.onLogUpload\","
                                                             "\"params\":"
                                                             "\\{"
                                                             "\"logUploadStatus\":\"UPLOAD_SUCCESS\""
                                                             "\\}"
                                                             "\\}")));

                onLogUpload.SetEvent();

                return Core::ERROR_NONE;
            }));

    handler.Subscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD;
    sysEventData.data.systemStates.state =   IARM_BUS_SYSMGR_LOG_UPLOAD_SUCCESS;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    EXPECT_EQ(Core::ERROR_NONE, onLogUpload.Lock());

    handler.Unsubscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);
}

/**
 * @brief Test case for onLogUploadFailed when uploadLogScriptNotRunning.
 *
 * Verifies onLogUpload event will NOT be triggered correctly when the upload log script is not running.
 *
 * @param None.
 * @return None.
 */
TEST_F(SystemServicesEventIarmTest, onLogUploadFailed_whenUploadLogScriptNotRunning)
{
    Core::Event onLogUpload(false, true);
    EXPECT_CALL(service, Submit(::testing::_, ::testing::_)).Times(0);
    handler.Subscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);

    IARM_Bus_SYSMgr_EventData_t sysEventData;
    sysEventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_LOG_UPLOAD;
    sysEventData.data.systemStates.state =   IARM_BUS_SYSMGR_LOG_UPLOAD_SUCCESS;
    systemStateChanged(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, &sysEventData, 0);

    EXPECT_EQ(Core::ERROR_TIMEDOUT, onLogUpload.Lock(100));

    handler.Unsubscribe(0, _T("onLogUpload"), _T("org.rdk.System"), message);
}
/*Test cases for onLogUpload ends here*/

