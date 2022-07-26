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
    //WrapsImplMock wrapsImplMock;
    //RfcApiImplMock rfcApiImplMock;
    //Mode
    //string sysMode;
    //int sysDuration;
    //GzEnabled
    //bool gzEnabled;
    //devPower state
    //string devPowerState;
    //string devStandbyReason;
    //bool networkStandby;
    
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
         EXPECT_CALL(IarmBusImplMock, IARM_Bus_Call)
        .WillOnce(
            [](const char* ownerName, const char* methodName, void* arg, size_t argLen) {
                EXPECT_TRUE(strcmp(methodName, "DaemonSysModeChange") == 0);
                param->result = 0;
                return IARM_RESULT_SUCCESS;
            });

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":\"mode\":normal,\"duration\":1}"), response));
        EXPECT_EQ(response, string("{\"success\":false}"));

        EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMode"),_T("{}"), response));
        EXPECT_EQ(response, string("{\"success\":false}"));
}
