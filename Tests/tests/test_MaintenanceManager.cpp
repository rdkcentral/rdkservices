
/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 **/

#include "gtest/gtest.h"
#include "FactoriesImplementation.h"
#include "MaintenanceManager.h"
#include "RfcApiMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "WrapsMock.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Test;
using ::testing::StrEq;
using ::testing::Gt;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

class MaintenanceManagerTest : public Test {
protected:
    Core::ProxyType<Plugin::MaintenanceManager> plugin_;
    Core::JSONRPC::Handler&                 handler_;
    Core::JSONRPC::Connection               connection_;
    string                                  response_;
    NiceMock<IarmBusImplMock>               iarmBusImplMock_;
    NiceMock<RfcApiImplMock>                rfcApiImplMock;
    NiceMock<WrapsImplMock> wrapsImplMock;
	
    MaintenanceManagerTest()
        : plugin_(Core::ProxyType<Plugin::MaintenanceManager>::Create())
        , handler_(*plugin_)
        , connection_(1, 0)
    {
        IarmBus::getInstance().impl = &iarmBusImplMock_;
        RfcApi::getInstance().impl = &rfcApiImplMock;
		Wraps::getInstance().impl = &wrapsImplMock;
    }

    virtual ~MaintenanceManagerTest() override
    {
        IarmBus::getInstance().impl = nullptr;
    }
};


static AssertionResult isValidCtrlmRcuIarmEvent(IARM_EventId_t ctrlmRcuIarmEventId)
{
    switch (ctrlmRcuIarmEventId) {
        case IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE:
        case IARM_BUS_DCM_NEW_START_TIME_EVENT:
            return AssertionSuccess();
        default:
            return AssertionFailure();
    }
}

class MaintenanceManagerInitializedEventTest : public MaintenanceManagerTest {
protected:
    IARM_EventHandler_t               controlEventHandler_;
    NiceMock<ServiceMock>             service_;
    NiceMock<FactoriesImplementation> factoriesImplementation_;
    Core::JSONRPC::Message            message_;
    PluginHost::IDispatcher*          dispatcher_;

    MaintenanceManagerInitializedEventTest() :
        MaintenanceManagerTest()
    {
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(StrEq(IARM_BUS_MAINTENANCE_MGR_NAME),IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, _))
            .WillOnce(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    controlEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));
        EXPECT_CALL(iarmBusImplMock_, IARM_Bus_RegisterEventHandler(StrEq(IARM_BUS_MAINTENANCE_MGR_NAME), IARM_BUS_DCM_NEW_START_TIME_EVENT, _))
            .WillRepeatedly(Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                    EXPECT_TRUE(isValidCtrlmRcuIarmEvent(eventId));
                    controlEventHandler_ = handler;
                    return IARM_RESULT_SUCCESS;
                }));

        EXPECT_EQ(string(""), plugin_->Initialize(nullptr));
        PluginHost::IFactories::Assign(&factoriesImplementation_);
        dispatcher_ = static_cast<PluginHost::IDispatcher*>(plugin_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher_->Activate(&service_);
    }

    virtual ~MaintenanceManagerInitializedEventTest() override
    {
        plugin_->Deinitialize(nullptr);
        dispatcher_->Deactivate();
        dispatcher_->Release();
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(MaintenanceManagerTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getMaintenanceActivityStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getMaintenanceStartTime")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("setMaintenanceMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("startMaintenance")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("stopMaintenance")));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Exists(_T("getMaintenanceMode")));
}

#if 1
TEST_F(MaintenanceManagerTest, startMaintenance)
{
	/*EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/RFCbas/RFCbase.sh &")));

                return 0;
            }));*/
			
	ON_CALL(wrapsImplMock, system(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/RFCbas/RFCbase.sh &")));
                return 0;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("startMaintenance"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}

TEST_F(MaintenanceManagerTest, getMaintenanceActivityStatus)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getMaintenanceActivityStatus"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceStatus\":\"MAINTENANCE_ERROR\",\"LastSuccessfulCompletionTime\":0,\"isCriticalMaintenance\":false,\"isRebootPending\":false,\"success\":true}");
}

TEST_F(MaintenanceManagerTest, getMaintenanceStartTime)
{
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getMaintenanceStartTime"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceStartTime\":12345,\"success\":true}");
}



TEST_F(MaintenanceManagerTest, stopMaintenance)
{
    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
				pstParamData->type = WDMP_BOOLEAN;
				strncpy(pstParamData->value, "true", MAX_PARAM_LEN);
                return WDMP_SUCCESS;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("stopMaintenance"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"success\":false}");
}

#endif
/*
#include <gtest/gtest.h>

#include "MaintenanceManager.h"

using namespace WPEFramework;

//using ::testing::NiceMock;

class MaintenanceManagerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::MaintenanceManager> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Connection connection;
    string response;

    MaintenanceManagerTest()
        : plugin(Core::ProxyType<Plugin::MaintenanceManager>::Create())
        , handler(*(plugin))
        , connection(1, 0)
    {
    }
    virtual ~MaintenanceManagerTest() = default;
};

TEST_F(MaintenanceManagerTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMaintenanceActivityStatus")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMaintenanceStartTime")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMaintenanceMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startMaintenance")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopMaintenance")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMaintenanceMode")));
}
*/