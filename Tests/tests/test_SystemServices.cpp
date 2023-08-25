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
#include "readprocMock.h"

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

/****************************************************************************************************
 * Test functions for :clearLastDeepSleepReason
 * clearLastDeepSleepReason :
 *                clear the last deep sleep reason by removing the file that stores it.
 *                This method takes no parameters.
 *
 *                @return Whether the request succeeded.
 * Use case coverage:
 *                @Success :1
 *                @Failure :1
 ***************************************************************************************************/

/**
 * @brief : clearLastDeepSleepReason when file failed to remove.
 *          Check if the file for the last deep sleep reason failed to remove using unlink,
 *          then  clearLastDeepSleepReason shall be failed and an error message is returned in the response.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"SysSrv_Status":7,"errorMessage":"Unexpected error","success":false}
 */
TEST_F(SystemServicesTest, clearLastDeepSleepReasonFailed_When_unlinkFailed)
{
    const char* filepath = "/opt/standbyReason.txt";
    EXPECT_TRUE(Core::File(string(_T("/opt/standbyReason.txt"))).Exists());
    EXPECT_CALL(wrapsImplMock, unlink(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T(filepath)));
                return -1;
            }));
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("clearLastDeepSleepReason"), _T("{}"), response));
}

/**
 * @brief : clearLastDeepSleepReason when file successfully removed.
 *          Check if the file for the last deep sleep reason is successfully removed using unlink,
 *          then clearLastDeepSleepReason shall be succeeded.
 *
 * @param[in]   :  This method takes no parameters.
 * @return      :  {"success": true}
 */
TEST_F(SystemServicesTest, clearLastDeepSleepReasonSuccess_When_unlinkSucceed)
{
    const char* filepath = "/opt/standbyReason.txt";
    EXPECT_TRUE(Core::File(string(_T("/opt/standbyReason.txt"))).Exists());
    EXPECT_CALL(wrapsImplMock, unlink(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T(filepath)));
                return 0;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("clearLastDeepSleepReason"), _T("{}"), response));
}


/*Test cases for clearLastDeepSleepReason ends here*/
