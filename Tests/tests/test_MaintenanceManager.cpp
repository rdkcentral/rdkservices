
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
#include <fstream>
#include <iostream>
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

extern "C" FILE* __real_popen(const char* command, const char* type);
extern "C" int __real_pclose(FILE* pipe);

class MaintenanceManagerTest : public Test {
protected:
    Core::ProxyType<Plugin::MaintenanceManager> plugin_;
    Core::JSONRPC::Handler&                 handler_;
    Core::JSONRPC::Connection               connection_;
    string                                  response_;
    NiceMock<IarmBusImplMock>               iarmBusImplMock_;
    NiceMock<RfcApiImplMock>                rfcApiImplMock;
    NiceMock<WrapsImplMock>                 wrapsImplMock;
	
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

        EXPECT_EQ(string(""), plugin_->Initialize(&service_));
        PluginHost::IFactories::Assign(&factoriesImplementation_);
        dispatcher_ = static_cast<PluginHost::IDispatcher*>(plugin_->QueryInterface(PluginHost::IDispatcher::ID));
        dispatcher_->Activate(&service_);
    }

    virtual ~MaintenanceManagerInitializedEventTest() override
    {
        plugin_->Deinitialize(&service_);
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

TEST_F(MaintenanceManagerTest, setMaintenanceMode)
{
    ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
				pstParamData->type = WDMP_BOOLEAN;
				strncpy(pstParamData->value, "true", MAX_PARAM_LEN);
                return WDMP_SUCCESS;
            }));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"IGNORE_UPDATE\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"IGNORE_UPDATE\",\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"ENFORCE_OPTOUT\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"ENFORCE_OPTOUT\",\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"BYPASS_OPTOUT\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"FOREGROUND\",\"optOut\":\"BYPASS_OPTOUT\",\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"IGNORE_UPDATE\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"IGNORE_UPDATE\",\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"ENFORCE_OPTOUT\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"ENFORCE_OPTOUT\",\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.setMaintenanceMode"), _T("{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"BYPASS_OPTOUT\"}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("org.rdk.MaintenanceManager.1.getMaintenanceMode"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceMode\":\"BACKGROUND\",\"optOut\":\"BYPASS_OPTOUT\",\"success\":true}");
}

TEST_F(MaintenanceManagerInitializedEventTest, getMaintenanceActivityStatus)
{
    IARM_Bus_MaintMGR_EventData_t	eventData;
	
	EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(::testing::AnyNumber())
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                return 0;
            }));
		
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	eventData.data.maintenance_module_status.status = MAINT_DCM_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));

    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getMaintenanceActivityStatus"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceStatus\":\"MAINTENANCE_STARTED\",\"LastSuccessfulCompletionTime\":0,\"isCriticalMaintenance\":false,\"isRebootPending\":false,\"success\":true}");
}

TEST_F(MaintenanceManagerInitializedEventTest, startMaintenanceOnReboot)
{	
    IARM_Bus_MaintMGR_EventData_t	eventData;
	struct tm result = {};
	time_t start_time = time(NULL);
   
	localtime_r(&start_time, &result);
	snprintf(eventData.data.startTimeUpdate.start_time, MAX_TIME_LEN-1, "%04d-%02d-%02d %02d:%02d:%02d", 
		result.tm_year+1900,
		result.tm_mon +1,
		result.tm_mday,
		result.tm_hour,
		result.tm_min,
		result.tm_sec);
	eventData.data.startTimeUpdate.start_time[MAX_TIME_LEN-1] = '\0';
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_DCM_NEW_START_TIME_EVENT, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	 EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(4)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/StartDCM_maintaince.sh &")));
                return 0;
            }))
		.WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/RFCbase.sh &")));
                return 0;
            }))
		.WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/swupdate_utility.sh >> /opt/logs/swupdate.log &")));
                return 0;
            }))
		.WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/Start_uploadSTBLogs.sh &")));
                return 0;
            }));
	std::this_thread::sleep_for(std::chrono::seconds(3));
			
	eventData.data.maintenance_module_status.status = MAINT_DCM_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_RFC_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_RFC_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
    
	std::this_thread::sleep_for(std::chrono::seconds(5));
	
}

TEST_F(MaintenanceManagerInitializedEventTest, startMaintenance)
{	
    IARM_Bus_MaintMGR_EventData_t	eventData;
	
	EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [&](const char* command) {
                return 0;
            }));
    std::this_thread::sleep_for(std::chrono::seconds(3));
	
	eventData.data.maintenance_module_status.status = MAINT_DCM_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_RFC_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_RFC_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("startMaintenance"), _T("{org.rdk.startMaintenance}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_RFC_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_RFC_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_FWDOWNLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(3));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_LOGUPLOAD_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	std::this_thread::sleep_for(std::chrono::seconds(5));
}

TEST_F(MaintenanceManagerInitializedEventTest, stopMaintenanceRFCEnable)
{
    IARM_Bus_MaintMGR_EventData_t	eventData;
	
	EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(::testing::AnyNumber())
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                return 0;
            }));
		
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	eventData.data.maintenance_module_status.status = MAINT_DCM_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	ON_CALL(rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
				pstParamData->type = WDMP_BOOLEAN;
				strncpy(pstParamData->value, "true", MAX_PARAM_LEN);
                return WDMP_SUCCESS;
            }));
	
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("stopMaintenance"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"success\":true}");
}



TEST_F(MaintenanceManagerInitializedEventTest, getMaintenanceStartTime)
{
	IARM_Bus_MaintMGR_EventData_t	eventData;
	const char *deviceInfoScript = "/lib/rdk/getMaintenanceStartTime.sh";
	
                 
	EXPECT_CALL(wrapsImplMock, system(::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/StartDCM_maintaince.sh &")));
				eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	            controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
                return 0;
            }))
		.WillOnce(::testing::Invoke(
            [&](const char* command) {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/RFCbase.sh &")));
                return 0;
            }));
	ON_CALL(wrapsImplMock, popen(::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](const char* command, const char* type) -> FILE* {
                EXPECT_EQ(string(command), string(_T("/lib/rdk/getMaintenanceStartTime.sh &")));
                return __real_popen(deviceInfoScript, type);
            }));
    ON_CALL(wrapsImplMock, pclose(::testing::_))
        .WillByDefault(::testing::Invoke(
            [&](FILE* pipe){
                return __real_pclose(pipe);
            }));
	std::this_thread::sleep_for(std::chrono::seconds(2));
	
	eventData.data.maintenance_module_status.status = MAINT_DCM_INPROGRESS;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	eventData.data.maintenance_module_status.status = MAINT_DCM_COMPLETE;
	controlEventHandler_(IARM_BUS_MAINTENANCE_MGR_NAME, IARM_BUS_MAINTENANCEMGR_EVENT_UPDATE, &eventData, sizeof(IARM_Bus_MaintMGR_EventData_t));
	
	//Create fake device info script & Invoke getDeviceInfo
    ofstream file(deviceInfoScript);
    file << "echo \"123456789\"\n";
    file.close();
		
    EXPECT_EQ(Core::ERROR_NONE, handler_.Invoke(connection_, _T("getMaintenanceStartTime"), _T("{}"), response_));
    EXPECT_EQ(response_, "{\"maintenanceStartTime\":123456789,\"success\":true}");
	
}