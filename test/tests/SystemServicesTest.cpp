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
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "source/SystemInfo.h"
#include "FactoriesImplementation.h"
#include "SystemMock.h"
#include "WrapsMock.h"
#include "RfcApiMock.h"

using namespace WPEFramework;

namespace {
const string iarmName = _T("Thunder_Plugins");
}

class SystemServicesTest : public::testing::Test
{
    protected:
    Core::ProxyType<Plugin::SystemServices> systemplugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Handler& handlerV2;
    Core::JSONRPC::Connection connection;
    string response;
    IarmBusImplMock iarmBusImplMock;
    IARM_EventHandler_t handlerOnTerritoryChanged;
    IARM_EventHandler_t handlerOnDSTTimeChanged;
    FactoriesImplementation factoriesImplementation;
    WrapsImplMock wrapsImplMock;
    RfcApiImplMock rfcApiImplMock;
    //Mode
    string sysMode;
    int sysDuration;
    //GzEnabled
    bool gzEnabled;
    //devPower state
    string devPowerState;
    string devStandbyReason;
    bool networkStandby;
    
private:
    /* data */
public:
    SystemServicesTest()
    :systemplugin(Core::ProxyType<Plugin::SystemServices>::Create())
    ,handler(*systemplugin)
    ,handlerV2(*(systemplugin->GetHandler(2)))
    ,connection(1,0)
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }
    virtual void SetUp()
    {
        IarmBus::getInstance().impl = &iarmBusImplMock;
        Wraps::getInstance().impl = &wrapsImplMock;
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }

    virtual void TearDown()
    {
        IarmBus::getInstance().impl = nullptr;
        Wraps::getInstance().impl = nullptr;
        PluginHost::IFactories::Assign(nullptr);
    }

    void InitService()
    {

         // called by SystemServices::InitializeIARM, SystemServices::DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    // called by SystemServices::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Init(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](const char* name) {
                if (iarmName == string(name)) {
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }));

    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect)
            .WillOnce(return(IARM_RESULT_SUCCESS));

    EXPECT_EQ(string(""), systemplugin->Initialize(nullptr));

    }

    ~SystemServicesTest()
    {
        PluginHost::IFactories::Assign(nullptr);
    }


};

//Register all systemservices methods
TEST_F(SystemServicesTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getWakeupReason")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getPowerStateBeforeReboot")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareRebootDelay")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getXconfParams")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hasRebootBeenRequested")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isGzEnabled")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isOptOutTelemetry")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("queryMocaStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestSystemUptime")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setCachedValue")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeCacheKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDeepSleepTimer")));
   
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setNetworkStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setBootLoaderPattern")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("enableXREConnectionRetention")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getAvailableStandbyModes")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCachedValue")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getCoreTemperature")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDownloadedFirmwareInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareDownloadPercent")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastDeepSleepReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getLastDeepSleepReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMfgSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMfgSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getNetworkStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getOvertempGraceInterval")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPowerState")));
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPowerStateIsManagedByDevice")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreferredStandbyMode")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo2")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootReason")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getRFCConfig")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSerialNumber")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getStateInfo")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemVersions")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTemperatureThresholds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimeZoneDST")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("sampleSystemServiceAPI")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOptOutTelemetry")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOvertempGraceInterval")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPowerState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setPreferredStandbyMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTemperatureThresholds")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTimeZoneDST")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setWakeupSrcConfiguration")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("updateFirmware")));

}

//Mode
TEST_F(SystemServicesTest, mode){
        EXPECT_CALL(SystemMock, setMode(::testing::_))
                .Times(1)
                .WillOnce(::testing::Invoke(
                    [&](const char* mode, const int duration) {
                        sysMode = mode;
                        sysDuration = duration;
                        return 0;
                    }));

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":\"mode\":normal,\"duration\":1}"), response));
        EXPECT_EQ(response, string("{\"success\":false}"));

        EXPECT_CALL(SystemMock, getMode(::testing::_))
                .Times(1)
                .WillOnce(::testing::Invoke(
                    [&](char* mode, int* duration) {
                        ::memcpy(mode, sysMode.c_str(), sysMode.length());
                        duration = sysDuration;
                        return 0;
                    }));
        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMode"),_T("{}"), response));
        EXPECT_EQ(response, string("{\"success\":false}"));
}

//NetworkStandby
TEST_F(SystemServicesTest, networkStandby){
// check if intiIARM required.
    EXPECT_CALL(IarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, IARM_BUS_PWRMGR_API_SetNetworkStandbyMode) == 0);
                auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
                networkStandby = param->networkStnadby;
                param->result = 0;
                return IARM_RESULT_SUCCESS;
            })
        .WillOnce(
        [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
            EXPECT_TRUE(strcmp(methodName, IARM_BUS_PWRMGR_API_GetNetworkStandbyMode) == 0);
            auto* param = static_cast<iarmbus_acm_arg_t*>(arg);
            param->networkStnadby = networkStandby;
            param->result = 0;
            return IARM_RESULT_SUCCESS;
        });

EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setNetworkStandbyMode"), _T("{\"nwStandby\":standaby}"), response));
EXPECT_EQ(response, string("{\"success\":true}"));

EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getNetworkStandbyMode"),_T("{}"), response));
EXPECT_EQ(response, string("{\"success\":false}"));

}

//Call the methods... 
TEST_F(SystemServicesTest, gzEnabled){
    EXPECT_CALL(SystemMock, setGzEnabled(::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke(
                [&](bool enabled) {
                    gzEnabled = enabled;
                    return true;
                }));

    EXPECT_CALL(SystemMock, isGzEnabledHelper(::testing::_))
            .Times(1)
            .WillOnce(::testing::Invoke(
                [&](bool* enabled) {
                    enabled = &gzEnabled;
                    return true;
                }));
}

TEST_F(SystemServicesTest, powerState){
EXPECT_CALL(SystemMock, setDevicePowerState(::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
        [&](const char* powerState, const char* standbyReason) {
            //::memcpy(devPowerState, sysMode.c_str(), sysMode.length());
            devPowerState = powerState;
            devStandbyReason = standbyReason;
        }));

EXPECT_CALL(SystemMock, getDevicePowerState(::testing::_))
    .Times(1)
    .WillOnce(::testing::Invoke(
        [&](const char* powerState) {
            ::memcpy(powerState, devPowerState.c_str(), devPowerState.length());
        }));
}

TEST_F(SystemServicesTest, prefferedStandby)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setPreferredStandbyMode"),_T("{\"standbyMode\":LIGHT_SLEEP}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPreferredStandbyMode"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"preferredStandbyMode\":LIGHT_SLEEP\"\",\"success\":true}"));

}

TEST_F(SystemServicesTest, deviceInfo )
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getDeviceInfo"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
}

TEST_F(SystemServicesTest, rebootInfo)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPreviousRebootInfo"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPreviousRebootInfo2"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
}

//VIsit back ----
TEST_F(SystemServicesTest, firmware)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getDownloadedFirmwareInfo"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getFirmwareDownloadPercent"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getFirmwareUpdateInfo"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getFirmwareUpdateState"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("updateFirmware"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
    
}

//setPowerState / setDevicePowerState
TEST_F(SystemServicesTest, power)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setPowerState"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPowerState"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPowerStateBeforeReboot"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPowerStateIsManagedByDevice"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

}


// Cahce - set,get,remove, checkKey/contains
TEST_F(SystemServicesTest, cache)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCachedValue"), _T("{\"key\":test}",\"value\":test1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getCachedValue"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"test\":\"test1\",\"deprecated\":\"true\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cacheContains"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"deprecated\":\"true\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("removeCacheKey"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"deprecated\":\"true\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("cacheContains"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"SysSrv_Status\":12,\"errorMessage\":\"Key not found\",\"deprecated\":\"true\",\"success\":true}"));
}

TEST_F(SystemServicesTest, RebootDelay)
{
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "SystemServices"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                EXPECT_EQ(strcmp(pcParameterValue, "5"), 0);
                EXPECT_EQ(eDataType, WDMP_INT);

                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T(""), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":5}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, AutoReboot)
{
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                return WDMP_FAILURE;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "SystemServices"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"), 0);
                EXPECT_EQ(strcmp(pcParameterValue, "false"), 0);
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);

                return WDMP_SUCCESS;
            }))
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "SystemServices"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"), 0);
                EXPECT_EQ(strcmp(pcParameterValue, "true"), 0);
                EXPECT_EQ(eDataType, WDMP_BOOLEAN);

                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareAutoReboot"), _T(""), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{\"enable\":false}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{\"enable\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}


TEST_F(SystemServicesTest, PendingReboot)
{
    EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "SystemServices"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                EXPECT_EQ(strcmp(pcParameterValue, "0"), 0);
                EXPECT_EQ(eDataType, WDMP_INT);
                // trigger event saying we are in Maintenance Window
                return WDMP_SUCCESS;
            }));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("fireFirmwarePendingReboot"), _T(""), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}



TEST_F(SystemServicesTest, Territory )
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"territory\":\"\",\"region\":\"\",\"success\":true}"));
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":USA}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));   
}

