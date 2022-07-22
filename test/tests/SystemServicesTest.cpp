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

using namespace WPEFramework;

namespace WPEFramework
{
    namespace plugin
    {
        class SystemServicesTest : public::testing::Test
        {
            protected:
	        Core::ProxyType<Plugin::SystemServices> systemplugin;
	        Core::JSONRPC::Handler& handler;
    	    Core::JSONRPC::Handler& handlerV2;
            IarmBusImplMock iarmBusImplMock;
            IARM_EventHandler_t handlerOnTerritoryChanged;
            IARM_EventHandler_t handlerOnDSTTimeChanged;
        private:
            /* data */
        public:
            SystemServicesTest()
            :systemplugin(Core::ProxyType<Plugin::SystemServices>::Create())
            ,handler(*systemplugin)
            ,connection(1,0)
            ,handlerV2(*(systemplugin->GetHandler(2)))
        {
        }
        virtual void SetUp()
        {
            IarmBus::getInstance().impl = &iarmBusImplMock_;
            PluginHost::IFactories::Assign(&factoriesImplementation_);
        }

        virtual void TearDown()
        {
            IarmBus::getInstance().impl = nullptr;
            PluginHost::IFactories::Assign(nullptr);
        }

        ~SystemServicesTest()
        {
            PluginHost::IFactories::Assign(nullptr);
        }
    };
    } // namespace plugin
} // namespace WPEFramework



TEST_F(SystemServicesTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getWakeupReason")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getPowerStateBeforeReboot")));
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareRebootDelay");

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getXconfParams")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTerritory")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hasRebootBeenRequested");
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isGzEnabled");

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isOptOutTelemetry")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("queryMocaStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestSystemUptime");
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setCachedValue");

    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("removeCacheKey")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setDeepSleepTimer")));
   
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMode");
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

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":5}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
}

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

TEST_F(SystemServicesTest, mode)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMode"), _T("{\"modeInfo\":\"mode\":normal,\"duration\":1"}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMode"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

}

TEST_F(SystemServicesTest, cache)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setCachedValue"), _T("{\"key\":test}",\"value\":1"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("removeCacheKey"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"deprecated\":\"true\",\"success\":true}"));
}


TEST_F(SystemServicesTest, Territory )
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"territory\":\"\",\"region\":"\",\"success\":true}"));
    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":USA}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
    
}


TEST_F(SystemServicesTest, Plugin)
{
       // called by SystemServices::InitializeIARM, SystemServices::DeinitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_IsConnected(::testing::_, ::testing::_))
        .Times(3)
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 0;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
        .WillOnce(::testing::Invoke(
            [](const char* memberName, int* isRegistered) {
                if (iarmName == string(memberName)) {
                    *isRegistered = 1;
                    return IARM_RESULT_SUCCESS;
                }
                return IARM_RESULT_INVALID_PARAM;
            }))
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

    // called by SystemServices::InitializeIARM
    EXPECT_CALL(iarmBusImplMock, IARM_Bus_Connect())
        .Times(1)
        .WillOnce(::testing::Return(IARM_RESULT_SUCCESS));

    

    
    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getWakeupReason"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getXconfParams"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));
    
    

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getRFCConfig"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getSerialNumber"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getStateInfo"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getSystemVersions"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getTemperatureThresholds"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getTimeZoneDST"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("sampleSystemServiceAPI"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setOptOutTelemetry"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setOvertempGraceInterval"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setPreferredStandbyMode"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setTemperatureThresholds"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setTimeZoneDST"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("setWakeupSrcConfiguration"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    

    // check of deprecated api should be checked???
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isGzEnabled"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"enabled\":\"false\",\"deprecated\":\"true\",\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"Opt-Out\":\"false\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("queryMocaStatus"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"mocaEnabled\":\"false\",\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestSystemUptime"), _T("{\"key\":test}"), response));
    EXPECT_EQ(response, string("{\"systemUptime\":\"\",\"success\":true}"));

    

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setDeepSleepTimer"), _T("{\"seconds\":5}",\"value\":1}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    

   

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setNetworkStandbyMode"), _T("{\"nwStandby\":standaby}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("enableXREConnectionRetention"),_T("{\"enable\":true}"),response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getAvailableStandbyModes"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getCachedValue"),_T("{\"key\"}"), response));
    //EXPECT_EQ(response, string(""))
    // check exact return and update based on the result

    

    


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getLastDeepSleepReason"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMacAddresses"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMfgSerialNumber"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getMilestones"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));


    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getNetworkStandbyMode"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getOvertempGraceInterval"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

    
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection,_T("getPreferredStandbyMode"),_T("{}"), response));
    EXPECT_EQ(response, string("{\"success\":false}"));

  ////////******/
  

    // JSON-RPC events

    auto dispatcher = static_cast<PluginHost::IDispatcher*>(
        plugin->QueryInterface(PluginHost::IDispatcher::ID));
    EXPECT_TRUE(dispatcher != nullptr);

    dispatcher->Activate(&service);

    EXPECT_TRUE(handlerOnTerritoryChanged != nullptr);
    EXPECT_TRUE(handlerOnDSTTimeChanged != nullptr);

    handler.Subscribe(0, _T("OnTerritoryChanged"), _T("org.rdk.System"), message);
    handler.Subscribe(0, _T("OnDSTTimeChanged"), _T("org.rdk.System"), message);

    handlerOnTerritoryChanged(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_PRECHANGE, nullptr, 0);

    handlerOnDSTTimeChanged(
        IARM_BUS_DSMGR_NAME, IARM_BUS_DSMGR_EVENT_DISPLAY_FRAMRATE_POSTCHANGE, nullptr, 0);

    handler.Unsubscribe(0, _T("OnTerritoryChanged"), _T("org.rdk.System"), message);
    handler.Unsubscribe(0, _T("OnDSTTimeChanged"), _T("org.rdk.System"), message);

    dispatcher->Deactivate();

    dispatcher->Release();

    // Deinitialize

    plugin->Deinitialize(nullptr);
}
