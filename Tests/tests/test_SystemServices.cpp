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

        EXPECT_EQ(string(""), plugin->Initialize(&service));
    }

    virtual ~SystemServicesEventIarmTest() override
    {
        plugin->Deinitialize(&service);
    }

    virtual void SetUp()
    {
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
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("clearLastDeepSleepReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMfgSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getXconfParams")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSerialNumber")));
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
                EXPECT_EQ(string(command), string(_T("/lib/rdk/deviceInitiatedFWDnld.sh 0 4 >> /opt/logs/swupdate.log &")));
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

TEST_F(SystemServicesTest, getWakeupReason)
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

TEST_F(SystemServicesEventIarmTest, onTemperatureThresholdChanged)
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

TEST_F(SystemServicesTest, setPowerState)
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
                auto param = static_cast<IARM_Bus_PWRMgr_SetWakeupSrcConfig_Param_t*>(arg);
                EXPECT_EQ(param->srcType, WAKEUPSRC_WIFI);
                EXPECT_EQ(param->config, true);
                return IARM_RESULT_SUCCESS;
            });

    // TODO: BUG. enum should be used
    // TODO: BUG. boolean should not be number string
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setWakeupSrcConfiguration"), _T("{\"wakeupSrc\":3,\"config\":\"1\"}"), response));
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

TEST_F(SystemServicesEventIarmTest, onSystemPowerStateChanged)
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
 *                @Failure :7
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
   //ASSERT_EQ(response, "{\"message\":\"Input has unallowable characters\",\"success\":false}");
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
    //EXPECT_THAT(response, string("{\"success\":false}"));
}

/**
 * @brief : getDeviceInfo When QueryParam is Empty  and DevicePropertyFile Not Exist
 *          Check if (i)No input query param passed/ query Param = {make}
 *          & (ii) device property file doesnot exist,
 *          then,getDeviceInfo shall be failed and  returns an error message in the response
 *
 * @param[in]   : "params": "{}"
 * @return      : {"message":"Expected file not found","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnDevicePropertyFileNotExist)
{
    /* TODO : Implementation To be done:
     * Need to mock as etc/device.properties does not exist.Working on it */
    //EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{}"), response));
    //ASSERT_EQ(response, "{\"SysSrv_Status\":4,\"errorMessage\":\"Unexpected Error\",\"success\":false}");
}

/**
 * @brief : getDeviceInfo When QueryParam is Empty  and DevicePropertyFile failed to open
 *          Check if (i)No input query param passed/ query Param = {make}
 *          & (ii) Failed to open the device property file ,
 *          then,getDeviceInfo shall be failed and  returns an error message in the response
 *
 * @param[in]   : "params": "{}"
 * @return      :  {"message":"File access failed","success":false}
 */
TEST_F(SystemServicesTest, getDeviceInfoFailed_OnDevicePropertyFileFailedToOpen)
{
    /* TODO : Implementation To be done :
     * Mocking fopen with file doesnt exist is not working straight forward
     * as it impacts other APIs/plugins using fopen, so working on that */
    //EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceInfo"), _T("{}"), response));
    //ASSERT_EQ(response,"{\"SysSrv_Status\":5,\"errorMessage\":\"Unexpected Error\",\"success\":false}");
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
    //EXPECT_THAT(response, string("{\"SysSrv_Status\":2,\"errorMessage\":\"Unexpected Error\",\"success\":false}"));
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
    //EXPECT_THAT(response, string("{\"SysSrv_Status\":2,\"errorMessage\":\"Unexpected Error\",\"success\":false}"));
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
    //EXPECT_THAT(response, string("{\"SysSrv_Status\":11,\"errorMessage\":\"Unexpected Error\",\"success\":false}"));
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
                  strcpy(buffer, key_estb_mac);
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
    //EXPECT_THAT(response, string("{\"SysSrv_Status\":2,\"errorMessage\":\"Unexpected Error\",\"success\":false}"));

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
    //EXPECT_THAT(response, string("{\"SysSrv_Status\":2,\"errorMessage\":\"Unexpected Error\",\"success\":false}"));
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
    //EXPECT_EQ(response, string("{\"success\":false}"));
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
    //EXPECT_EQ(response, string("{\"success\":false}"));
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
 *                @Failure :4
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
 * @brief : getSerialNumber when TMP_SERIAL_NUMBER_FILE failed to read.
 *        Check if contents of TMP_SERIAL_NUMBER_FILE can not be open,
 *        then getSerialNumber shall be failed
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":6,"errorMessage":"Unsupported file content","success":false}
 */
TEST_F(SystemServicesTest, getSerialNumberSnmpFailed_WhenFailedToReadFromTmpFile)
{
     /*TODO : Implementation To be done :
     * Mocking fopen with file can not open and read has not been working straight forward
     * as it impacts other APIs/plugins using fopen, so working on that */

    //EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSerialNumber"), _T("{}"), response));
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
