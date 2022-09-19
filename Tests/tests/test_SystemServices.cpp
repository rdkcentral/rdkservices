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

#include "RfcApiMock.h"

#include <fstream>

using namespace WPEFramework;

class SystemInitializeTest: public::testing::Test
{
	protected:
	    	RfcApiImplMock rfcApiImplMock;

	SystemInitializeTest()
	{
        	RfcApi::getInstance().impl = &rfcApiImplMock;
	}

	virtual ~SystemInitializeTest()
	{
        	RfcApi::getInstance().impl = nullptr;
	}
};

class SystemServicesTest : public::SystemInitializeTest
{
    protected:
	    Core::ProxyType<Plugin::SystemServices> systemplugin;
	    Core::JSONRPC::Handler& handler;
	    Core::JSONRPC::Handler& handlerV2;
	    Core::JSONRPC::Connection connection;
	    string response;
 
	    SystemServicesTest()
		    :SystemInitializeTest()
    		    ,systemplugin(Core::ProxyType<Plugin::SystemServices>::Create())
    		    ,handler(*systemplugin)
    		    ,handlerV2(*(systemplugin->GetHandler(2)))
    		    ,connection(1,0)
    {
    }

    virtual ~SystemServicesTest() override = default;
};

TEST_F(SystemServicesTest, RegisterMethods)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("requestSystemUptime")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("fireFirmwarePendingReboot")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareAutoReboot")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("setFirmwareRebootDelay")));
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Exists(_T("getLastFirmwareFailureReason")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDownloadedFirmwareInfo")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareDownloadPercent")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getFirmwareUpdateState")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTimeZoneDST")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTimeZoneDST")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setTerritory")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getTerritory")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("hasRebootBeenRequested")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootInfo2")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPreviousRebootReason")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMilestones")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("isOptOutTelemetry")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setOptOutTelemetry")));
	EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSystemVersions")));
}

TEST_F(SystemServicesTest, SystemUptime)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("requestSystemUptime"), _T("{}"), response));

    EXPECT_THAT(response, ::testing::MatchesRegex(_T("\\{"
				    "\"systemUptime\":\"[0-9]+.[0-9]+\","
				    "\"success\":true"
				    "\\}")));
}

TEST_F(SystemServicesTest, PendingReboot)
{
    	EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
		return WDMP_SUCCESS;
	}));
	
	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("fireFirmwarePendingReboot"), _T("{}"),response));
	EXPECT_EQ(response,string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, AutoReboot)
{
    	EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.Enable"), 0);
                return WDMP_SUCCESS;
        }));

    	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFirmwareAutoReboot"), _T("{\"enable\":true}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, RebootDelay)
{
    	EXPECT_CALL(rfcApiImplMock, setRFCParameter(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char *pcCallerID, const char* pcParameterName, const char* pcParameterValue, DATA_TYPE eDataType) {
                EXPECT_EQ(strcmp(pcCallerID, "thunderapi"), 0);
                EXPECT_EQ(strcmp(pcParameterName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AutoReboot.fwDelayReboot"), 0);
                return WDMP_SUCCESS;
		}));
    	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("setFirmwareRebootDelay"), _T("{\"delaySeconds\":10}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));
}

TEST_F(SystemServicesTest, Firmware)
{
	ofstream file("/version.txt");
    	file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\nVERSION=000.36.0.0\nBUILD_TIME=\"2022-08-05 16:14:54\"\n";
    	file.close();

	file.open("/opt/fwdnldstatus.txt");
	file << "Method|xconf \nProto|\nStatus|Failure\nReboot|\nFailureReason|Invalid Request\nDnldVersn|AX013AN_5.6p4s1_VBN_sey\nDnldFile|\nDnlURL|https://dac15cdlserver.ae.ccp.xcal.tv:443/Images/AX013AN_5.6p4s1_VBN_sey-signed.bin\n FwupdateState|Failed\n";

	file.close();

	EXPECT_EQ(Core::ERROR_NONE, handlerV2.Invoke(connection, _T("getLastFirmwareFailureReason"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"failReason\":\"None\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDownloadedFirmwareInfo"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"currentFWVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"downloadedFWVersion\":\"\",\"downloadedFWLocation\":\"\",\"isRebootDeferred\":false,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareDownloadPercent"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"downloadPercent\":-1,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getFirmwareUpdateState"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"firmwareUpdateState\":0,\"success\":true}"));
}

TEST_F(SystemServicesTest, Timezone)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/New_York\"}"),response));
	EXPECT_EQ(response,string("{\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"timeZone\":\"America\\/New_York\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTimeZoneDST"), _T("{\"timeZone\":\"America/Costa_Rica\"}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));

    	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTimeZoneDST"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"timeZone\":\"America\\/Costa_Rica\",\"success\":true}"));
}

TEST_F(SystemServicesTest, InvalidTerritory)
{
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"U-NYC\"}"),response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US@\",\"region\":\"US-NYC\"}"),response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-N$C\"}"),response));
	EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"US12\",\"region\":\"US-NYC\"}"),response));
}

TEST_F(SystemServicesTest, ValidTerritory)
{
    	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"USA\",\"region\":\"US-NYC\"}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));

    	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"territory\":\"USA\",\"region\":\"US-NYC\",\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setTerritory"), _T("{\"territory\":\"GBR\",\"region\":\"GB-EGL\"}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));

    	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getTerritory"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"territory\":\"GBR\",\"region\":\"GB-EGL\",\"success\":true}"));
}

TEST_F(SystemServicesTest, reboot)
{
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("hasRebootBeenRequested"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"rebootRequested\":false,\"success\":true}"));
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

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"timeStamp\":\"18.08.2022_09:51.38\",\"reason\":\"Triggered from SystemServices! MAINTENANCE_REBOOT\",\"source\":\"HAL_CDL_notify_mgr_event\",\"customReason\":\"MAINTENANCE_REBOOT\",\"otherReason\":\"MAINTENANCE_REBOOT\",\"success\":true}"));
	
	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootInfo2"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"rebootInfo\":{\"timestamp\":\"Thu Aug 18 13:51:39 UTC 2022\",\"source\":\"HAL_CDL_notify_mgr_event\",\"reason\":\"OPS_TRIGGERED\",\"customReason\":\"Unknown\",\"lastHardPowerReset\":\"Thu Aug 18 13:51:39 UTC 2022\"},\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPreviousRebootReason"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"reason\":\"OPS_TRIGGERED\",\"success\":true}"));
}

TEST_F(SystemServicesTest, MileStones)
{
	ofstream file("/opt/logs/rdk_milestones.log");
	file << "RDK_STARTED:35755\n";
	file.close();


    	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMilestones"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"milestones\":[\"RDK_STARTED:35755\"],\"deprecated\":true,\"success\":true}"));
}

TEST_F(SystemServicesTest, Telemetry)
{
	ofstream file("/opt/tmtryoptout");
	file << "false" ;
	file.close();

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"),response));
	EXPECT_EQ(response,string("{\"Opt-Out\":false,\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setOptOutTelemetry"), _T("{\"Opt-Out\":true}"),response));
    	EXPECT_EQ(response,string("{\"success\":true}"));

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("isOptOutTelemetry"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"Opt-Out\":true,\"success\":true}"));
}

TEST_F(SystemServicesTest, SystemVersions)
{
    ofstream file("/version.txt");
    file << "imagename:PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\nSDK_VERSION=17.3\nMEDIARITE=8.3.53\nYOCTO_VERSION=dunfell\nVERSION=000.36.0.0\nBUILD_TIME=\"2022-08-05 16:14:54\"\n";
    file.close();

	EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSystemVersions"), _T("{}"),response));
    	EXPECT_EQ(response,string("{\"stbVersion\":\"PX051AEI_VBN_2203_sprint_20220331225312sdy_NG\",\"receiverVersion\":\"000.36.0.0\",\"stbTimestamp\":\"Fri 05 Aug 2022 16:14:54 AP UTC\",\"success\":true}"));
}
